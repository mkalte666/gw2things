#include "accountoverview.h"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using nlohmann::json;

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