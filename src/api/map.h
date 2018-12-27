#ifndef _map_h
#define _map_h

#include "continent.h"
#include "endpoint.h"

struct MapData : public AbstractEndpoint {
    MapData(int tmpId = 0)
        : AbstractEndpoint("https://api.guildwars2.com/v2/maps/", 60 * 60 * 24)
        , id(tmpId)
    {
    }
    virtual void fetch() override;
    virtual void onFetchComplete() override;

    int id = 0;
    std::string name = "???";
    int minLevel = 0;
    int maxLevel = 0;
    int defaultFloor = 0;
    std::vector<int> floors;
    int regionId = 0;
    std::string regionName = "???";
    int continentId = 0;
    std::string continentName = "???";
    ContinentData continent;
    SDL_Rect mapRect = { 0, 0, 0, 0 };
    SDL_Rect continentRect = { 0, 0, 0, 0 };
};
json_helper(MapData);

#endif //_map_h