/*
    fetcher.cpp: Abstraction and further madness for curl
    Copyright (C) 2018 Malte Kießling

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "fetcher.h"

#include "env.h"
#include <SDL.h>
#include <cassert>
#include <curl/curl.h>
#include <curl/easy.h>
#include <fstream>
#include <thread>

// stat party
#include <algorithm>
#include <chrono>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

// singleton instance
Fetcher Fetcher::fetcher;

// source: https://stackoverflow.com/questions/11413860/best-string-hashing-function-for-short-filenames
// and: https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
uint32_t fnvHash(const std::string& in)
{
    uint32_t h = 2166136261;

    for (auto c : in) {
        h = (h * 16777619) ^ c;
    }

    return h;
}

std::string makeId(const std::string& in)
{
    auto hash = fnvHash(in);
    return std::to_string(hash);
}

size_t writefunc(char* ptr, size_t, size_t nmemb, void* userdata)
{
    auto data = static_cast<std::vector<char>*>(userdata);
    data->insert(data->end(), ptr, ptr + nmemb);
    return nmemb;
}

void Fetcher::fetchWorker()
{
    CURL* curl = curl_easy_init();
    while (running) {
        using namespace std::chrono;
        std::this_thread::sleep_for(10ms);
        if (paused) {
            using namespace std::chrono;
            std::this_thread::sleep_for(5000ms);
            paused = false;
            continue;
        }
        FetchInfo info;
        {
            std::unique_lock<std::mutex> lock(scheduleMutex);
            if (scheduledFetches.empty()) {
                continue;
            }
            info = std::move(scheduledFetches.front());
            scheduledFetches.pop();
        }
        singleFetch(info, curl);
        {
            std::unique_lock<std::mutex> lock(completeMutex);
            completeFetches.push(std::move(info));
        }
    }
    curl_easy_cleanup(curl);
}

int Fetcher::singleFetch(FetchInfo& fetch, void* curlPtr)
{
    // ============
    // REAAAD THE COMMENTS, YOU WHO DARES TO GO HERE
    // ============
    // this will attempt to load the date, either from the filesystem or from the cache
    // try to not scare it
    if (fetch.maxAge >= 0) {
        std::string filename = Env::makeCacheFilepath(makeId(fetch.url));
        std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);

        // are we already cached?
        if (file.good()) {
            // stat the file and calculate its age
            struct stat statRes;
            (void)stat(filename.c_str(), &statRes);
            time_t age = time(nullptr) - statRes.st_mtime;

            // can we use the data?
            if (fetch.maxAge == 0 || age < fetch.maxAge) {
                SDL_Log("Doing a cache read for %s (%s)", fetch.url.c_str(), filename.c_str());
                std::streamsize filesize = file.tellg(); // we opend with ios::ate
                file.seekg(0, std::ios::beg);
                fetch.data.resize(filesize);
                file.read(fetch.data.data(), filesize);

                // we done
                fetch.success = true;
                fetch.complete = true;
                return 0;
            } else {
                SDL_Log("Cache expired for %s", fetch.url.c_str());
            }
        } else {
            SDL_Log("No cache available for %s", fetch.url.c_str());
        }
    }

    // we need to do a fetch (for whatever reason). now follows a bit of curl
    CURL* curl;
    curl = (CURL*)curlPtr;
    // plug the api key into from the api
    std::string header = "Authorization: Bearer ";
    header += Env::getApiKey();
    curl_slist* slist = nullptr;
    slist = curl_slist_append(slist, header.c_str());
    // check if the string list is ok beause we fuck up completly if it isnt
    if (slist == nullptr) {
        SDL_Log("Something is terribly wrong with curl");
        curl_easy_cleanup(curl);

        // so that this one is clean out of the list
        fetch.complete = true;
        return -1;
    }

    // set the stuff up
    //char* escapedUrl = curl_easy_escape(curl, fetch.url.c_str(), fetch.url.size());
    //if (escapedUrl) {
    //    curl_easy_setopt(curl, CURLOPT_URL, escapedUrl);
    //    curl_free(escapedUrl);
    //} else {
        curl_easy_setopt(curl, CURLOPT_URL, fetch.url.c_str());
    //}

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fetch.data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

    // actual fetch is here. this takes an arbitrary amout of time.
    // thats why we are in a thread btw.
    CURLcode res = curl_easy_perform(curl);

    // race condition: set complete only after we are done (before return) (down down)
    if (res == CURLE_OK) {
        fetch.success = true;
    } else {
        SDL_Log("CURL ERROR: %s", curl_easy_strerror(res));
    }

    curl_slist_free_all(slist);

    //ok, so we did a fetch and data is populated.
    // do we need to write this to the cache?
    if (fetch.maxAge >= 0) {
        fillCache(fetch.url, fetch.data);
    }

    // done with writing to the file
    // and also done if we dont. NOW we can set the complete flag
    fetch.complete = true;
    return 0;
}

Fetcher::Fetcher()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    running = true;
    for (int i = 0; i < MAX_FETCHES; i++) {
        workers[i] = std::thread(std::bind(&Fetcher::fetchWorker, this));
    }
}

Fetcher::~Fetcher()
{
    running = false;
    for (int i = 0; i < MAX_FETCHES; i++) {
        workers[i].join();
    }

    curl_global_cleanup();
}

size_t Fetcher::fetch(std::string url, FetcherCallback callback, time_t maxAge)
{
    FetchInfo info;
    info.callback = callback;
    info.url = url;
    info.maxAge = maxAge;
    numFetches++;
    info.fetchId = numFetches;

    return fetch(info);
}

void Fetcher::fillCache(const std::string& url, const std::vector<char>& data)
{
    std::string filename = Env::makeCacheFilepath(makeId(url));
    SDL_Log("Updating Cache for %s (%s))", url.c_str(), filename.c_str());

    std::ofstream outfile(filename, std::ios::out | std::ios::trunc | std::ios::binary);
    if (outfile.good()) {
        outfile.write(data.data(), data.size());
        outfile.close();
    }
}

bool Fetcher::isExpired(const std::string& url, time_t maxAge)
{
    if (maxAge < 0) {
        return true;
    }

    std::string filename = Env::makeCacheFilepath(makeId(url));
    struct stat statRes;
    if (stat(filename.c_str(), &statRes) == 0) {
        time_t age = time(nullptr) - statRes.st_mtime;
        if (maxAge == 0 || age < maxAge) {
            return false;
        }
    }

    return true;
}

size_t Fetcher::fetch(FetchInfo& info)
{
    std::unique_lock<std::mutex> lock(scheduleMutex);
    scheduledFetches.push(info);

    return info.fetchId;
}

void Fetcher::drop(size_t fetchId)
{
    std::unique_lock<std::mutex> lock(completeMutex);
    if (fetchId != 0 && std::find(droppedIDs.begin(), droppedIDs.end(), fetchId) == droppedIDs.end()) {
        droppedIDs.push_back(fetchId);
    }
}

void Fetcher::tick()
{
    std::vector<FetchInfo> fetches;
    {
        std::unique_lock<std::mutex> lock(completeMutex);
        if (!completeFetches.empty()) {
            fetches.reserve(completeFetches.size());
            auto& info = completeFetches.front();
            auto dropIter = std::find(droppedIDs.begin(), droppedIDs.end(), info.fetchId);
            if (dropIter == droppedIDs.end()) {
                fetches.push_back(std::move(info));
            } else {
                droppedIDs.erase(dropIter);
            }
            completeFetches.pop();
        }
    }

    for (auto& fetch : fetches) {
        if (fetch.success && fetch.callback) {
            fetch.callback(fetch.data);
        } else {
            SDL_Log("Fetcher: Could not fetch %s", fetch.url.c_str());
        }
    }
}

void Fetcher::forceDelay()
{
    paused = true;
}
