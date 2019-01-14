#ifndef _item_cache_h
#define _item_cache_h

#include "item.h"

#include <set>

class ItemCache {
public:
    ItemCache();
    void update();

    std::vector<int> query(std::string str, int limit = 50);
    void show();

    bool visible = false;

private:
    using TextIndex = std::map<std::string, std::vector<int>>;
    void bulkFetchHelper(const std::vector<char>& data);
    std::vector<int> idsLeft;

    void addToIndex(const std::string& word, int itemId);
    void breakDownForIndex(const json& j);

    void saveIndex();
    void readIndex();

    static TextIndex index;
    static bool needsLoad;
    static bool isLoading;
    static int loadProgress;

    std::string itemSearchQuery;
    std::vector<std::shared_ptr<ItemData>> itemQueryResults;
};

#endif //_item_cache_h