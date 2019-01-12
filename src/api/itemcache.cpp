#include "itemcache.h"
#include "env.h"
#include "fetcher.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <functional>
#include <workpool.h>

bool my_isalpha(char ch)
{
    return std::isalpha(static_cast<unsigned char>(ch));
}

char my_tolower(char c)
{
    c = tolower(c);
    if (c == '\n' || c == '\r') {
        c = ' ';
    }

    return c;
}

ItemCache::ItemCache()
{
    Workpool::instance.addTask(std::bind(&ItemCache::readIndex, this), nullptr);
}

void ItemCache::bulkFetchHelper(const std::vector<char>& data)
{
    if (data.size() > 0) {
        auto j = json::parse(data);
        if (j.is_array()) {
            for (auto item : j.get<std::vector<json>>()) {
                std::string fakeFetchUrl = "https://api.guildwars2.com/v2/items/" + std::to_string(item["id"].get<int>());
                auto strdata = item.dump();
                breakDownForIndex(item);

                if (Fetcher::isExpired(fakeFetchUrl, 0)) {
                    Fetcher::fillCache(fakeFetchUrl, std::vector<char>(strdata.begin(), strdata.end()));
                }
            }
        }
    }

    if (idsLeft.empty()) {
        saveIndex();
        return;
    }

    std::string nextFetchUrl = "https://api.guildwars2.com/v2/items?ids=";
    for (int i = 0; i < 150 && idsLeft.size() > 0; i++) {
        nextFetchUrl += std::to_string(idsLeft[idsLeft.size() - 1]) + ",";
        idsLeft.pop_back();
    }

    using namespace std::placeholders;
    auto boundFetchHelper = std::bind(&ItemCache::bulkFetchHelper, this, _1);
    Fetcher::fetcher.fetch(nextFetchUrl, boundFetchHelper, 0);
}

void ItemCache::addToIndex(const std::string& word, int itemId)
{
    index[word].insert(itemId);
}

void ItemCache::breakDownForIndex(const json& j)
{
    ItemData item;
    j.get_to(item);

    std::vector<std::string> toIndex = {
        item.chatLink,
        item.description,
        item.name
    };
    for (auto str : toIndex) {
        if (str.empty()) {
            continue;
        }
        std::transform(str.begin(), str.end(), str.begin(), my_tolower);
        addToIndex(str, item.id);
        for (
            std::string::iterator pos = std::find_if(str.begin(), str.end(), my_isalpha), end;
            pos != str.end();
            pos = std::find_if(end, str.end(), my_isalpha)) {

            end = std::find_if_not(pos, str.end(), my_isalpha);
            std::string tag(pos, end);
            if (!tag.empty() && tag.size() >= 3) {
                addToIndex(tag, item.id);
            }
        }
    }
}

void ItemCache::saveIndex()
{
    std::string filename = Env::makeCacheFilepath("itemindex");
    std::ofstream file(filename, std::ios::out);
    file << "v1"
         << "\n";
    file << index.size() << "\n";
    for (const auto& p : index) {
        file << p.first.c_str() << "\n";
        file << p.second.size() << "\n";
        for (const auto& id : p.second) {
            file << id << "\n";
        }
    }

    file.close();
}

void ItemCache::readIndex()
{
    std::string filename = Env::makeCacheFilepath("itemindex");
    std::ifstream file(filename);
    if (!file.good()) {
        return;
    }

    std::string version;
    file >> version;
    if (version != "v1") {
        SDL_Log("item cahce format not supported");
        return;
    }

    int size = 0;
    file >> size;
    SDL_Log("reading %d index entries", size);
    while (size > 0 && !file.eof() && file.good()) {
        int refCount = 0;
        std::string tag;
        std::getline(file, tag);
        if (tag.empty()) {
            continue;
        }
        file >> refCount;
        std::set<int> items;
        for (int i = 0; i < refCount; i++) {
            int id = 0;
            file >> id;
            items.insert(id);
        }
        index.insert({ tag, std::move(items) });
        size--;
    }

    SDL_Log("done reading index");
}

void ItemCache::update()
{
    int fetch = Fetcher::fetcher.fetch("https://api.guildwars2.com/v2/items", [this](const std::vector<char>& data) {
        auto j = json::parse(data);
        j.get_to(idsLeft);
        bulkFetchHelper(std::vector<char>());
    },
        0);
}

std::set<int> ItemCache::query(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), my_tolower);
    const auto& ids = index[str];
    
    return ids;
}
