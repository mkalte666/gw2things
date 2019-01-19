#include "materialstorage.h"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using nlohmann::json;

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
        //InventorySlot::bulkFetch(slots);
        for (auto& s : slots) {
            s->fetchFullData();
        }
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
