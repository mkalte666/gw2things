#include "bank.h"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using nlohmann::json;


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