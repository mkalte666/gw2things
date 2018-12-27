#ifndef _bank_h
#define _bank_h

#include "endpoint.h"
#include "inventory.h"

struct BankData : public AbstractEndpoint {
    BankData()
        : AbstractEndpoint("https://api.guildwars2.com/v2/account/bank", 10)
    {
    }
    virtual void fetch() override;
    virtual void onFetchComplete() override;
    virtual void show() override;

    std::vector<std::shared_ptr<InventorySlot>> slots;
    bool disableFullFetch = false;

private:
    std::string searchFilter;
};
json_helper(BankData);

#endif // _bank_h