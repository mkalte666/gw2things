#ifndef _mapview_h
#define _mapview_h

#include "apitypes.h"

class MapView {
public:
    MapView();
    ~MapView();

    void tick();
    void show();

    bool enabled;

private:
    MapData mapdata;
    int zoom = 5;

    struct VisibleTile {
        ~VisibleTile();
        glm::ivec2 pos;
        std::string url;
        int zoom = -1;
        SDL_Texture* tex = nullptr;

        int fetchId = 0;
        void fetch();
    };
    std::vector<VisibleTile> tiles;
};

#endif _mapview_h
