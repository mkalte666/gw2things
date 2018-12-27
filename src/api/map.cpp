#include "map.h"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using nlohmann::json;

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