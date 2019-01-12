/*
    fetcher.h: Interface for the api fetcher
    Copyright (C) 2018 Malte Kieﬂling

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
#ifndef _fetcher_h
#define _fetcher_h

#include <functional>
#include <queue>
#include <string>
#include <vector>
#include <mutex>
#include <thread>

class Fetcher {
public:
    using FetcherCallback = std::function<void(const std::vector<char>& data)>;
    static const time_t CACHE_DEFAULT_MAXAGE = 60 * 60;
    static const int MAX_FETCHES = 5;

    size_t fetch(std::string url, FetcherCallback callback, time_t maxAge = CACHE_DEFAULT_MAXAGE);

    static void fillCache(const std::string& url, const std::vector<char>& data);
    static bool isExpired(const std::string& url, time_t maxAge);

    void drop(size_t fetchId);

    void tick();
    void forceDelay();

    static Fetcher fetcher;

private:
    Fetcher();
    ~Fetcher();

    struct FetchInfo {
        size_t fetchId = 0;
        std::vector<char> data;
        bool complete = false;
        bool success = false;
        std::string url = "";
        FetcherCallback callback = nullptr;
        // max age < 0 means no caching, maxAge == 0 means alway from cache,
        // other numbers specify the time after wich the cache expires
        time_t maxAge = CACHE_DEFAULT_MAXAGE;
    };
    size_t fetch(FetchInfo& info);
    
    std::mutex completeMutex;
    std::queue<FetchInfo> completeFetches;
    std::mutex scheduleMutex;
    std::queue<FetchInfo> scheduledFetches;
    std::vector<size_t> droppedIDs;

    size_t numFetches = 0;

    void fetchWorker();
    volatile bool running;
    volatile bool paused = false;
    std::thread workers[MAX_FETCHES];

    static int singleFetch(FetchInfo& fetch, void* curlPtr);
};

#endif
