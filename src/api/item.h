#ifndef _item_h
#define _item_h

#include "endpoint.h"
#include "icon.h"
#include "price.h"



struct ItemData : public AbstractEndpoint {
    ItemData(int tmpId = 0)
        : AbstractEndpoint("https://api.guildwars2.com/v2/items/", 0)
        , id(tmpId)
    {
    }
    virtual void fetch() override;
    virtual void onFetchComplete() override;
    virtual void show() override;

    virtual bool matchSearch(const std::string& query);

    const int INVALID_ID = 0;

    int id = INVALID_ID;
    std::string name = "???";
    std::string chatLink = "";
    std::string iconUrlString = "";
    Icon icon;
    std::string description = "No Description";
    std::string type = "";
    std::string rarity = "";
    int level = 0;
    int vendorValue = 0;
    int defaultSkinId = 0;
    std::vector<std::string> flags;
    std::vector<std::string> gameTypes;
    std::vector<std::string> restrictions;
    PriceData price;

private:
    
    
};
json_helper(ItemData);


#endif //_item_h