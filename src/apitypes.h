#ifndef _apitypes_h
#define _apitypes_h

#include "env.h"
#include "icon.h"
#include <glm/glm.hpp>
#include <list>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#define json_helper(T) void from_json(const nlohmann::json& j, T& data);

json_helper(SDL_Rect);

// i am a bad boy
namespace glm {
json_helper(glm::vec1);
json_helper(glm::vec2);
json_helper(glm::vec3);
json_helper(glm::vec4);
}; // namespace glm

struct AbstractEndpoint {
    AbstractEndpoint(std::string endpoint, time_t maxCacheAge)
        : endpoint(endpoint)
        , maxCacheAge(maxCacheAge)
    {
    }

    virtual ~AbstractEndpoint();

    // completly disable copies (callbacks are a bitch)
    AbstractEndpoint(const AbstractEndpoint& other) = delete;
    AbstractEndpoint(AbstractEndpoint&& other) = delete;

    bool visible = false;
    std::string endpoint = "";
    time_t maxCacheAge = 0;
    size_t fetchId;
    virtual void fetch() = 0;
    virtual void show(){};

    virtual void onFetchComplete(){};
};

struct GuildPublicData : public AbstractEndpoint {
    GuildPublicData(std::string idForFetch = "")
        : AbstractEndpoint("https://api.guildwars2.com/v2/guild/", 60)
        , id(idForFetch)
    {
    }
    void fetch() override;
    void show() override;

    std::string id;
    std::string name = "???";
    std::string motd = "???";
};
json_helper(GuildPublicData);

struct AccountOverviewData : public AbstractEndpoint {
    AccountOverviewData()
        : AbstractEndpoint("https://api.guildwars2.com/v2/account", 60)
    {
    }
    void fetch() override;
    void show() override;
    virtual void onFetchComplete() override;

    std::string id = "";
    std::string name = "Unknown";
    int age = 0;
    int world = 0;
    std::vector<std::shared_ptr<GuildPublicData>> guilds;
    std::vector<std::string> access;
    bool commander = false;
    int fractalLevel = 0;
    int dailyAp = 0;
    int monthlyAp = 0;
    int wvwRank = 0;
};
json_helper(AccountOverviewData);

struct ContinentData : public AbstractEndpoint {
    ContinentData(int tmpId = 0)
        : AbstractEndpoint("https://api.guildwars2.com/v2/continents/", 60 * 60 * 24)
        , id(tmpId)
    {
    }
    virtual void fetch() override;

    int id = 0;
    std::string name = "???";
    glm::vec2 dimensions = glm::vec2(0, 0);
    int minZoom = 0;
    int maxZoom = 0;
    std::vector<int> floors;
};
json_helper(ContinentData);

struct MapData : public AbstractEndpoint {
    MapData(int tmpId = 0)
        : AbstractEndpoint("https://api.guildwars2.com/v2/maps/", 60 * 60 * 24)
        , id(tmpId)
    {
    }
    virtual void fetch() override;
    virtual void onFetchComplete() override;

    int id = 0;
    std::string name = "???";
    int minLevel = 0;
    int maxLevel = 0;
    int defaultFloor = 0;
    std::vector<int> floors;
    int regionId = 0;
    std::string regionName = "???";
    int continentId = 0;
    std::string continentName = "???";
    ContinentData continent;
    SDL_Rect mapRect = { 0, 0, 0, 0 };
    SDL_Rect continentRect = { 0, 0, 0, 0 };
};
json_helper(MapData);

struct PriceData : public AbstractEndpoint {
    PriceData()
        : AbstractEndpoint("https://api.guildwars2.com/v2/commerce/prices/", 60*30)
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

struct ItemData : public AbstractEndpoint {
    ItemData(int tmpId = 0)
        : AbstractEndpoint("https://api.guildwars2.com/v2/items/", 60 * 60 * 24)
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
};
json_helper(MapData);

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
};
json_helper(InventorySlot);

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

#undef json_helper
#endif // _apytypes_h