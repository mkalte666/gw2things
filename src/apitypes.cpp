#include "apitypes.h"

#include <SDL.h>
#include <algorithm>
#include <fetcher.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using nlohmann::json;

namespace nlohmann {
template <typename T>
struct adl_serializer<std::shared_ptr<T>> {
    static void from_json(const json& j, std::shared_ptr<T>& opt)
    {
        if (!opt.get()) {
            opt = std::make_shared<T>();
        }

        j.get_to(*opt);
    }

    static void to_json(json& j, const std::shared_ptr<T>& opt)
    {
        // NOP HERE
        j = nullptr;
    }
};
}

void from_json(const json& j, SDL_Rect& data)
{
    std::vector<std::vector<int>> arr = j;
    data.x = arr[0][0];
    data.y = arr[0][1];
    data.w = arr[1][0];
    data.h = arr[1][1];
}

// i am a bad boy
namespace glm {

void from_json(const json& j, glm::vec1& data)
{
    std::vector<float> arr = j;
    data.x = arr[0];
}

void from_json(const json& j, glm::vec2& data)
{
    std::vector<float> arr = j;
    data.x = arr[0];
    data.y = arr[1];
}

void from_json(const json& j, glm::vec3& data)
{
    std::vector<float> arr = j;
    data.x = arr[0];
    data.y = arr[1];
    data.z = arr[3];
}

void from_json(const json& j, glm::vec4& data)
{
    std::vector<float> arr = j;
    data.x = arr[0];
    data.y = arr[1];
    data.z = arr[3];
    data.w = arr[4];
}
}; // namespace glm

template <typename T>
void jsonProcessor(T* target, const std::vector<char>& data)
{
#ifdef _DEBUG
    //SDL_Log("Got response: %s", std::string(data.begin(), data.end()).c_str());
#endif
    json j = json::parse(data);
    // refetch if we get slapped
    if (j.is_object() && j.value("text", std::string()) == "too many requests") {
        target->maxCacheAge = -1; // enforce fetch
        Fetcher::fetcher.forceDelay();
        target->fetch();
        return;
    }
    j.get_to<T>(*target);
    target->onFetchComplete();
}

template <typename T>
Fetcher::FetcherCallback jsonProcBinder(T& target)
{
    using namespace std::placeholders;
    auto f = std::bind(jsonProcessor<T>, &target, _1);
    return f;
}

bool vectorOfStringGetter(void* data, int n, const char** out_text)
{
    const std::vector<std::string>& v = *(std::vector<std::string>*)data;
    *out_text = v[n].c_str();
    return true;
}

void toLower(std::string& input)
{
    std::transform(input.begin(), input.end(), input.begin(), ::tolower);
}

// here data begins
AbstractEndpoint::~AbstractEndpoint()
{
    Fetcher::fetcher.drop(fetchId);
}

void from_json(const json& j, AccountOverviewData& data)
{
    data.id = j.value("id", "");
    data.name = j.value("name", "error");
    data.age = j.value("age", 0);
    data.world = j.value("world", 0);
    data.guilds.clear();
    for (auto guild : j.value<std::vector<std::string>>("guilds", std::vector<std::string>())) {
        data.guilds.push_back(std::make_shared<GuildPublicData>(guild));
    }
    data.access = j.value<std::vector<std::string>>("access", std::vector<std::string>());
    data.commander = j.value("commander", false);
    data.fractalLevel = j.value("fractal_level", 0);
    data.dailyAp = j.value("daily_ap", 0);
    data.monthlyAp = j.value("monthly_ap", 0);
    data.wvwRank = j.value("wvw_rank", 0);
}

void AccountOverviewData::show()
{
    if (!visible) {
        return;
    }
    ImGui::Begin("Account Overview", &visible);
    ImGui::InputText("Account Name", &name, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputText("Account ID", &id, ImGuiInputTextFlags_ReadOnly);
    std::string ageStr = std::to_string(age);
    ImGui::InputText("Age", &ageStr, ImGuiInputTextFlags_ReadOnly);
    std::string worldStr = std::to_string(world);
    ImGui::InputText("World", &worldStr, ImGuiInputTextFlags_ReadOnly);
    bool copyCommander = commander;
    ImGui::Checkbox("Commander", &copyCommander);

    ImGui::Text("Guilds");
    ImGui::Indent(20.0f);
    for (auto& guild : guilds) {
        ImGui::Text("%s", guild->name.c_str());
        if (ImGui::BeginPopupContextItem(guild->name.c_str())) {
            if (ImGui::MenuItem("Show more...")) {
                guild->visible = true;
            }

            ImGui::EndPopup();
        }
    }
    ImGui::Unindent(20.0f);

    ImGui::Text("Expansions");
    ImGui::Indent(20.0f);
    for (auto expansion : access) {
        ImGui::Text("%s", expansion.c_str());
    }
    ImGui::Unindent(20.0f);

    ImGui::InputInt("Fractal Level", &fractalLevel, 1, 100, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputInt("Daily AP", &dailyAp, 1, 100, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputInt("Monthly AP", &monthlyAp, 1, 100, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputInt("WVW Rank", &wvwRank, 1, 100, ImGuiInputTextFlags_ReadOnly);
    ImGui::End();

    for (auto& g : guilds) {
        g->show();
    }
}

void AccountOverviewData::onFetchComplete()
{
    for (auto& g : guilds) {
        g->fetch();
    }
}

void AccountOverviewData::fetch()
{
    Fetcher::fetcher.drop(fetchId);
    fetchId = Fetcher::fetcher.fetch(endpoint, jsonProcBinder(*this), maxCacheAge);
}

void from_json(const json& j, GuildPublicData& data)
{
    data.id = j.value("id", "");
    data.name = j.value("name", "");
    data.motd = j.value("motd", "");
}

void GuildPublicData::fetch()
{
    Fetcher::fetcher.drop(fetchId);
    fetchId = Fetcher::fetcher.fetch(endpoint + id, jsonProcBinder(*this), maxCacheAge);
}

void GuildPublicData::show()
{
    if (!visible) {
        return;
    }
    ImGui::Begin(name.c_str(), &visible);
    ImGui::InputText("Name", &name, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputTextMultiline("MOTD", &motd, ImVec2(0, 0), ImGuiInputTextFlags_ReadOnly);
    ImGui::End();
}

void from_json(const json& j, MapData& data)
{
    data.id = j.value("id", 0);
    data.name = j.value("name", "");
    data.minLevel = j.value("min_level", 0);
    data.maxLevel = j.value("max_level", 0);
    data.defaultFloor = j.value("default_floor", 0);
    data.floors = j.value("floors", std::vector<int>());
    data.regionId = j.value("region_id", 0);
    data.regionName = j.value("region_name", "");
    data.continentId = j.value("continent_id", 0);
    data.continentName = j.value("continent_name", "");
    data.continent.id = data.continentId;
    data.mapRect = j.value("map_rect", SDL_Rect({ 0, 0, 0, 0 }));
    data.continentRect = j.value("continent_rect", SDL_Rect({ 0, 0, 0, 0 }));
}

void MapData::fetch()
{
    Fetcher::fetcher.drop(fetchId);
    fetchId = Fetcher::fetcher.fetch(endpoint + std::to_string(id), jsonProcBinder(*this), maxCacheAge);
}

void MapData::onFetchComplete()
{
    continent.fetch();
}

void from_json(const json& j, ContinentData& data)
{
    data.id = j.value("id", 0);
    data.name = j.value("name", "");
    data.dimensions = j.value("continent_dims", glm::vec2(0, 0));
    data.minZoom = j.value("min_zoom", 0);
    data.maxZoom = j.value("max_zoom", 0);
    data.floors = j.value("floors", std::vector<int>());
}

void ContinentData::fetch()
{
    Fetcher::fetcher.drop(fetchId);
    fetchId = Fetcher::fetcher.fetch(endpoint + std::to_string(id), jsonProcBinder(*this), maxCacheAge);
}

void from_json(const json& j, ItemData& data)
{
    data.id = j.value("id", 0);
    data.name = j.value("name", "");
    data.chatLink = j.value("chat_link", "");
    data.iconUrlString = j.value("icon", "");
    data.description = j.value("description", "");
    data.type = j.value("type", "");
    data.rarity = j.value("rarity", "");
    data.level = j.value("level", 0);
    data.vendorValue = j.value("vendor_value", 0);
    data.defaultSkinId = j.value("default_skin", 0);
    data.flags = j.value("flags", std::vector<std::string>());
    data.gameTypes = j.value("game_types", std::vector<std::string>());
    data.restrictions = j.value("restrictions", std::vector<std::string>());
    // missing: details
}

void ItemData::fetch()
{
    if (id == INVALID_ID) {
        return;
    }
    Fetcher::fetcher.drop(fetchId);
    fetchId = Fetcher::fetcher.fetch(endpoint + std::to_string(id), jsonProcBinder(*this), maxCacheAge);
}

void ItemData::onFetchComplete()
{
    icon.url = iconUrlString;
    icon.fetch();
    price.itemId = id;
    price.fetch();
}

void ItemData::show()
{
    if (!visible) {
        return;
    }
    std::string windowTitle = "Item: ";
    windowTitle += name;
    ImGui::Begin(windowTitle.c_str(), &visible);
    if (ImGui::BeginPopupContextWindow(name.c_str())) {
        if (ImGui::MenuItem("Copy Chat Link...")) {
            SDL_SetClipboardText(chatLink.c_str());
        }

        ImGui::EndPopup();
    }

    icon.imguiDraw();
    ImGui::SameLine();
    ImGui::Text("Item: %s", name.c_str());
    ImGui::Text("Chat Link: %s", chatLink.c_str());
    ImGui::End();
}

bool ItemData::matchSearch(const std::string& query)
{
    auto lowerQuery = query;
    toLower(lowerQuery);
    auto lowerName = name;
    toLower(lowerName);
    auto lowerDescription = description;
    toLower(description);

    return lowerName.find(lowerQuery) != lowerName.npos
        || lowerDescription.find(lowerQuery) != lowerDescription.npos;
}

void from_json(const json& j, InventorySlot& data)
{
    data.upgrades.clear();
    data.infusions.clear();
    data.item.reset();

    if (j.is_null()) {
        data.emptySlot = true;
        return;
    }

    data.emptySlot = false;
    data.itemId = j.value("id", 0);
    data.count = j.value("count", 0);
    data.charges = j.value("charges", 0);
    data.skinId = j.value("skin", 0);
    data.upgradeIds = j.value("upgrades", std::vector<int>());
    data.infusionIds = j.value("infusions", std::vector<int>());
    data.binding = j.value("binding", "");
    data.boundTo = j.value("bound_to", "");
}

InventorySlot::InventorySlot()
{
}

void InventorySlot::fetchFullData()
{
    item = std::make_shared<ItemData>(itemId);
    item->fetch();

    for (auto upgradeId : upgradeIds) {
        auto upgrade = std::make_shared<ItemData>(upgradeId);
        upgrade->fetch();
        upgrades.push_back(upgrade);
    }

    for (auto infusionId : infusionIds) {
        auto infusion = std::make_shared<ItemData>(infusionId);
        infusion->fetch();
        infusions.push_back(infusion);
    }
}

void InventorySlot::draw()
{
    if (!emptySlot && item) {
        // icon
        item->icon.imguiDraw();
        // context menu on the icon
        if (ImGui::BeginPopupContextItem(item->name.c_str())) {
            if (ImGui::MenuItem("Details")) {
                item->visible = true;
            }
            if (ImGui::MenuItem("Copy Chat Link")) {
                SDL_SetClipboardText(item->chatLink.c_str());
            }
            ImGui::EndPopup();
        }
        item->show();

        // the rest
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::TextWrapped("%s", item->name.c_str());
        ImGui::Text("Count: %d", count);
        ImGui::TextWrapped("%s", item->price.niceString(count).c_str());
        ImGui::EndGroup();

        ImGui::SameLine(ImGui::GetWindowWidth() / 2.0f);
        ImGui::BeginGroup();
        ImGui::Text("Description");
        ImGui::TextWrapped("%s", item->description.c_str());
        ImGui::EndGroup();
    }
}

void InventorySlot::bulkFetch(std::vector<std::shared_ptr<InventorySlot>>& slots)
{
    // upgrades are ignore for now. FIXME
    for (int i = 0; i < slots.size(); i++) {
        std::string bulkEndpoint = "https://api.guildwars2.com/v2/items?ids=";
        for (int j = 0; j < 100 && i < slots.size(); j++, i++) {
            bulkEndpoint += std::to_string(slots[i]->itemId) + ",";
        }

        Fetcher::fetcher.fetch(bulkEndpoint, [slots](const std::vector<char>& data) {
            json j = json::parse(data);
            // refetch if we get slapped
            if (j.is_object() && j.value("text", std::string()) == "too many requests") {
                // unfortunate
                return;
            }
            std::vector<json> values;
            for (auto item : j.get<std::vector<std::shared_ptr<ItemData>>>()) {
                auto it = std::find_if(slots.begin(), slots.end(), [item](auto& slot) {
                    return slot->itemId == item->id;
                });
                if (it != slots.end()) {
                    (*it)->item = item;
                }
                item->onFetchComplete();
            }
        },
            60 * 60 * 24);
    }
}

void from_json(const json& j, BankData& data)
{
    data.slots.clear();
    try {
        j.get_to(data.slots);
    } catch (json::out_of_range) {
        // welp. nothing needs to happen, json is just broken. happens i guess
        // usually we avoid this
    }
}

void BankData::fetch()
{
    Fetcher::fetcher.drop(fetchId);
    fetchId = Fetcher::fetcher.fetch(endpoint, jsonProcBinder(*this), maxCacheAge);
}

void BankData::onFetchComplete()
{
    if (!disableFullFetch) {
        for (auto& slot : slots) {
            slot->fetchFullData();
        }
    }
}

void BankData::show()
{
    if (!visible) {
        return;
    }

    ImGui::Begin("Account Bank", &visible);
    ImGui::InputText("Filter", &searchFilter);
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        searchFilter.clear();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reload")) {
        fetch();
    }

    ImGui::BeginChild("Scrolling");
    for (auto& slot : slots) {
        if (!slot->emptySlot && slot->item) {
            auto& item = slot->item;
            if (!searchFilter.empty() && !item->matchSearch(searchFilter)) {
                continue;
            }

            slot->draw();
            ImGui::Separator();
        }
    }
    ImGui::EndChild();
    ImGui::End();
}

void from_json(const json& j, PriceData& data)
{
    json buys = j.value("buys", json());
    json sells = j.value("sells", json());
    if (sells.is_null() || buys.is_null()) {
        return;
    }
    data.buyPrice = buys.value("unit_price", 0);
    data.buyCount = buys.value("quantity", 0);
    data.sellPrice = sells.value("unit_price", 0);
    data.sellCount = sells.value("quantity", 0);
    data.whitelisted = j.value("whitelisted", false);
}

void PriceData::fetch()
{
    Fetcher::fetcher.drop(fetchId);
    fetchId = Fetcher::fetcher.fetch(endpoint + std::to_string(itemId), jsonProcBinder(*this), maxCacheAge);
}

std::string PriceData::niceString(int stack)
{
    if (sellCount == 0) {
        return std::string("Not Traded");
    }

    std::string result;
    result = "Buy: ";
    result += std::to_string(buyPrice);
    result += "\nSell: ";
    result += std::to_string(sellPrice);
    if (stack != 1) {
        result += "\nBuy All";
        result += std::to_string(buyPrice * stack);
        result += "\nSell All: ";
        result += std::to_string(sellPrice * stack);
    }
    return result;
}

void from_json(const json& j, MaterialStorageData& data)
{
    data.slots.clear();
    try {
        j.get_to(data.slots);
    } catch (json::out_of_range) {
    }
}

void MaterialStorageData::fetch()
{
    Fetcher::fetcher.drop(fetchId);
    fetchId = Fetcher::fetcher.fetch(endpoint, jsonProcBinder(*this), maxCacheAge);
}

void MaterialStorageData::onFetchComplete()
{
    if (!disableFullFetch) {
        InventorySlot::bulkFetch(slots);
    }
}

void MaterialStorageData::show()
{
    if (!visible) {
        return;
    }

    ImGui::Begin("Material Storage", &visible);
    ImGui::InputText("Filter", &searchFilter);
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        searchFilter.clear();
    }

    ImGui::BeginChild("Scrolling");
    for (auto& slot : slots) {
        if (!slot->emptySlot && slot->item) {
            auto& item = slot->item;
            if (searchFilter.size() < 3 || !item->matchSearch(searchFilter)) {
                continue;
            }

            slot->draw();
            ImGui::Separator();
        }
    }
    ImGui::EndChild();
    ImGui::End();
}
