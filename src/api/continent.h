#ifndef _continent_h
#define _continent_h

#include "endpoint.h"

struct ContinentData : public AbstractEndpoint {
    ContinentData(int tmpId = 0)
        : AbstractEndpoint("https://api.guildwars2.com/v2/continents/", 60 * 60 * 24)
        , id(tmpId)
    {
    }
    virtual void fetch() override;

    int id = 0;
    std::string name = "???";
    glm::vec2 dimensions = glm::vec2(0, 0);
    int minZoom = 0;
    int maxZoom = 0;
    std::vector<int> floors;
};
json_helper(ContinentData);

#endif 