#include "endpoint.h"
#include <vector>

using nlohmann::json;

void from_json(const json& j, SDL_Rect& data)
{
    std::vector<std::vector<int>> arr = j;
    data.x = arr[0][0];
    data.y = arr[0][1];
    data.w = arr[1][0];
    data.h = arr[1][1];
}

// i am a bad boy
namespace glm {

void from_json(const json& j, glm::vec1& data)
{
    std::vector<float> arr = j;
    data.x = arr[0];
}

void from_json(const json& j, glm::vec2& data)
{
    std::vector<float> arr = j;
    data.x = arr[0];
    data.y = arr[1];
}

void from_json(const json& j, glm::vec3& data)
{
    std::vector<float> arr = j;
    data.x = arr[0];
    data.y = arr[1];
    data.z = arr[3];
}

void from_json(const json& j, glm::vec4& data)
{
    std::vector<float> arr = j;
    data.x = arr[0];
    data.y = arr[1];
    data.z = arr[3];
    data.w = arr[4];
}
}; // namespace glm

AbstractEndpoint::~AbstractEndpoint()
{
    Fetcher::fetcher.drop(fetchId);
}

void AbstractEndpoint::fetchOnce()
{
    if (fetchedOnce) {
        return;
    }
    fetchedOnce = false;
    fetch();
}

void toLower(std::string& input)
{
    std::transform(input.begin(), input.end(), input.begin(), ::tolower);
}