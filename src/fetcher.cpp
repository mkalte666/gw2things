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

Fetcher::Fetcher()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

Fetcher::~Fetcher()
{
    curl_global_cleanup();
}

size_t writefunc(char* ptr, size_t, size_t nmemb, void* userdata)
{
    auto data = static_cast<std::vector<char>*>(userdata);
    data->insert(data->end(), ptr, ptr + nmemb);
    return nmemb;
}

size_t Fetcher::fetch(std::string url, FetcherCallback callback, time_t maxAge)
{
    FetchInfo info;
    info.callback = callback;
    info.url = url;
    info.maxAge = maxAge;
    numFetches++;
    info.fetchId = numFetches;

    // we have a connection limit. dont over do it
    if (fetches.size() >= MAX_FETCHES) {
        scheduledFetches.push_back(info);
        return info.fetchId;
    }

    return fetch(info);
}
size_t Fetcher::fetch(FetchInfo& info)
{
    auto infoIter = fetches.insert(fetches.end(), info);

    // ============
    // REAAAD THE COMMENTS, YOU WHO DARES TO GO HERE
    // ============
    // this thread will attempt to load the date, either from the filesystem or from the cache
    // try to not scare it
    std::thread([infoIter]() {
        // see if we have to do a fetch
        if (infoIter->maxAge >= 0) {
            std::string filename = Env::makeCacheFilepath(makeId(infoIter->url));
            std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);

            // are we already cached?
            if (file.good()) {
                // stat the file and calculate its age
                struct stat statRes;
                (void)stat(filename.c_str(), &statRes);
                time_t age = time(nullptr) - statRes.st_mtime;

                // can we use the data?
                if (infoIter->maxAge == 0 || age < infoIter->maxAge) {
                    SDL_Log("Doing a cache read for %s (%s)", infoIter->url.c_str(), filename.c_str());
                    std::streamsize filesize = file.tellg(); // we opend with ios::ate
                    file.seekg(0, std::ios::beg);
                    infoIter->data.resize(filesize);
                    file.read(infoIter->data.data(), filesize);

                    // we done
                    infoIter->success = true;
                    infoIter->complete = true;
                    return 0;
                } else {
                    SDL_Log("Cache expired for %s", infoIter->url.c_str());
                }
            } else {
                SDL_Log("No cache available for %s", infoIter->url.c_str());
            }
        }

        // we need to do a fetch (for whatever reason). now follows a bit of curl
        CURL* curl;
        curl = curl_easy_init();
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
            infoIter->complete = true;
            return -1;
        }

        // set the stuff up
        curl_easy_setopt(curl, CURLOPT_URL, infoIter->url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &infoIter->data);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

        // actual fetch is here. this takes an arbitrary amout of time.
        // thats why we are in a thread btw.
        CURLcode res = curl_easy_perform(curl);

        // race condition: set complete only after we are done (before return) (down down)
        if (res == CURLE_OK) {
            infoIter->success = true;
        } else {
            SDL_Log("CURL ERROR: %s", curl_easy_strerror(res));
        }

        curl_slist_free_all(slist);
        curl_easy_cleanup(curl);

        //ok, so we did a fetch and data is populated.
        // do we need to write this to the cache?
        if (infoIter->maxAge >= 0) {
            std::string filename = Env::makeCacheFilepath(makeId(infoIter->url));
            SDL_Log("Updating Cache for %s (%s))", infoIter->url.c_str(), filename.c_str());

            std::ofstream outfile(filename, std::ios::out | std::ios::trunc | std::ios::binary);
            if (outfile.good()) {
                outfile.write(infoIter->data.data(), infoIter->data.size());
                outfile.close();
            }
        }

        // done with writing to the file
        // and also done if we dont. NOW we can set the complete flag
        infoIter->complete = true;
        return 0;
    })
        .detach();

    return info.fetchId;
}

void Fetcher::drop(size_t fetchId)
{
    for (auto& f : scheduledFetches) {
        if (f.fetchId != fetchId) {
            continue;
        }
        f.callback = nullptr;
    }
    for (auto& f : fetches) {
        if (f.fetchId != fetchId) {
            continue;
        }
        f.callback = nullptr;
    }
}

void Fetcher::tick()
{
    auto i = fetches.begin();
    while (i != fetches.end()) {
        auto last = i++;
        if (last->complete) {
            if (last->success && last->callback) {
                last->callback(last->data);
            } else {
                SDL_Log("Could not fetch %s", last->url.c_str());
            }
            fetches.erase(last);
        }
    }

    // check if we have pending fetches
    if (fetches.size() < MAX_FETCHES && scheduledFetches.size() > 0) {
        while (fetches.size() < MAX_FETCHES && scheduledFetches.size() > 0) {
            auto infoIter = scheduledFetches.begin();
            fetch(*infoIter);
            scheduledFetches.erase(infoIter);
        }
    }
}
