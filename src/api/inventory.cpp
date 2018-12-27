#include "inventory.h"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using nlohmann::json;

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