#include "accountcharacters.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

void from_json(const json& j, AccountCharacterData& data)
{
    try {
        j.get_to(data.characterNames);
    } catch (json::type_error e) {
        SDL_Log("Error json parsing: %s", e.what());
    }
}

void AccountCharacterData::fetch()
{
    Fetcher::fetcher.drop(fetchId);
    fetchId = Fetcher::fetcher.fetch(endpoint, jsonProcBinder(*this), maxCacheAge);
}

void AccountCharacterData::show()
{
    if (!visible) {
        return;
    }

    ImGui::Begin("Account Characters", &visible);
    for (auto character : characters) {
        character->drawOverviewInfo();
        ImGui::Separator();
    }
    ImGui::End();
}

void AccountCharacterData::onFetchComplete()
{
    for (auto name : characterNames) {
        auto c = std::make_shared<CharacterData>(name);
        c->fetch();
        characters.push_back(c);
    }
}
