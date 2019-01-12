#ifndef _item_cache_h
#define _item_cache_h

#include "item.h"

#include <set>

class ItemCache {
public:
    ItemCache();
    void update();

    std::set<int> query(std::string str);
    
private:
    using TextIndex = std::map<std::string, std::set<int>>;
    void bulkFetchHelper(const std::vector<char>& data);
    std::vector<int> idsLeft;

    void addToIndex(const std::string& word, int itemId);
    void breakDownForIndex(const json& j);

    void saveIndex();
    void readIndex();

    TextIndex index;
};

#endif //_item_cache_h