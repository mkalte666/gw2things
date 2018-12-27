#include "continent.h"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using nlohmann::json;

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