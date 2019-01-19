#include "character.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

void from_json(const json& j, CharacterData& data)
{
    data.name = j.value("name", "");
    data.race = j.value("race", "");
    data.gender = j.value("gender", "");
    data.profession = j.value("profession", "");
    data.age = j.value<time_t>("age", 0);
    data.createdStr = j.value("created", "");
    data.deaths = j.value("deaths", 0);
}

void CharacterData::show()
{
}

void CharacterData::fetch()
{
    Fetcher::fetcher.drop(fetchId);
    fetchId = Fetcher::fetcher.fetch(endpoint+name, jsonProcBinder(*this), maxCacheAge);
}

void CharacterData::drawOverviewInfo()
{
    ImGui::BeginGroup();
    ImGui::Text("Name: %s", name.c_str());
    ImGui::Text("%s %s, %s", gender.c_str(), race.c_str(), profession.c_str());
    ImGui::Text("Died %d times since %s (age: %d hours)", deaths, createdStr.c_str(), age/(60*60));
    ImGui::EndGroup();
}
