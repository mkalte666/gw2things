#ifndef _material_storage_h
#define _material_storage_h

#include "endpoint.h"
#include "inventory.h"

struct MaterialStorageData : public AbstractEndpoint {
    MaterialStorageData()
        : AbstractEndpoint("https://api.guildwars2.com/v2/account/materials", 10)
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
json_helper(MaterialStorageData);

#endif // _material_storage_h