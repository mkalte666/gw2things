#include "guildpublic.h"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using nlohmann::json;

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