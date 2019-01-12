#include "item.h"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using nlohmann::json;



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


