#ifndef _inventory_h
#define _inventory_h

#include "endpoint.h"
#include "item.h"

struct InventorySlot {
    InventorySlot();
    InventorySlot(const InventorySlot& other) = delete;
    InventorySlot(InventorySlot&& other) = delete;

    bool emptySlot = true;
    int itemId = 0;
    int count = 0;
    int charges = 0;
    int skinId = 0;
    std::vector<int> upgradeIds;
    std::vector<int> infusionIds;
    std::string binding = "";
    std::string boundTo = "";

    std::shared_ptr<ItemData> item;
    std::vector<std::shared_ptr<ItemData>> upgrades;
    std::vector<std::shared_ptr<ItemData>> infusions;
    void fetchFullData();
    void draw();

    static void bulkFetch(std::vector<std::shared_ptr<InventorySlot>>& slots);
};
json_helper(InventorySlot);

#endif //_inventory_h