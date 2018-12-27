#ifndef _price_h
#define _price_h

#include "endpoint.h"

struct PriceData : public AbstractEndpoint {
    PriceData()
        : AbstractEndpoint("https://api.guildwars2.com/v2/commerce/prices/", 60 * 30)
    {
    }
    virtual void fetch() override;

    std::string niceString(int stack = 1);

    int itemId = 0;
    int sellCount = 0;
    int sellPrice = 0;
    int buyCount = 0;
    int buyPrice = 0;
    bool whitelisted = false;
};
json_helper(PriceData);

#endif // _price_h