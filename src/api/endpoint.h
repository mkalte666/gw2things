#ifndef _endpoint_h
#define _endpoint_h

#include "fetcher.h"
#include <SDL.h>
#include <glm/glm.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

using nlohmann::json;

#define json_helper(T) void from_json(const nlohmann::json& j, T& data);

json_helper(SDL_Rect);

// i am a bad boy
namespace glm {
json_helper(glm::vec1);
json_helper(glm::vec2);
json_helper(glm::vec3);
json_helper(glm::vec4);
}; // namespace glm

struct AbstractEndpoint {
    AbstractEndpoint(std::string endpoint, time_t maxCacheAge)
        : endpoint(endpoint)
        , maxCacheAge(maxCacheAge)
    {
    }

    virtual ~AbstractEndpoint();

    // completly disable copies (callbacks are a bitch)
    AbstractEndpoint(const AbstractEndpoint& other) = delete;
    AbstractEndpoint(AbstractEndpoint&& other) = delete;

    bool visible = false;
    std::string endpoint = "";
    time_t maxCacheAge = 0;
    size_t fetchId = 0;
    virtual void fetch() = 0;
    virtual void show(){};

    virtual void onFetchComplete(){};
};

namespace nlohmann {
template <typename T>
struct adl_serializer<std::shared_ptr<T>> {
    static void from_json(const json& j, std::shared_ptr<T>& opt)
    {
        if (!opt.get()) {
            opt = std::make_shared<T>();
        }

        j.get_to(*opt);
    }

    static void to_json(json& j, const std::shared_ptr<T>& opt)
    {
        // NOP HERE
        j = nullptr;
    }
};
}

template <typename T>
void jsonProcessor(T* target, const std::vector<char>& data)
{
#ifdef _DEBUG
    //SDL_Log("Got response: %s", std::string(data.begin(), data.end()).c_str());
#endif
    json j = json::parse(data);
    // refetch if we get slapped
    if (j.is_object() && j.value("text", std::string()) == "too many requests") {
        target->maxCacheAge = -1; // enforce fetch
        Fetcher::fetcher.forceDelay();
        target->fetch();
        return;
    }
    j.get_to<T>(*target);
    target->onFetchComplete();
}

template <typename T>
Fetcher::FetcherCallback jsonProcBinder(T& target)
{
    using namespace std::placeholders;
    auto f = std::bind(jsonProcessor<T>, &target, _1);
    return f;
}

void toLower(std::string& input);

#endif // _endpoint_h
