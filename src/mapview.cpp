#include "mapview.h"

#include "SDL_image.h"
#include "fetcher.h"
#include "mumblefile.h"
#include "env.h"

glm::ivec2 coordToPixelOffset(const glm::ivec2& coord, int currentZoom, int maxZoom)
{
    int32_t projection = static_cast<int>(pow(2, maxZoom - currentZoom));
    return glm::ivec2(coord.x / projection, coord.y / projection);
}

glm::ivec2 pixelToCoordOffset(const glm::ivec2& pixel, int currentZoom, int maxZoom)
{
    int32_t projection = static_cast<int>(pow(2, maxZoom - currentZoom));
    return glm::ivec2(pixel.x * projection, pixel.y * projection);
}

glm::ivec2 calculateRelativeOffset(const glm::ivec2& inPixel, int zoom)
{
    int x = inPixel.x % 256;
    int y = inPixel.y % 256;
    return glm::ivec2(x, y);
}

MapView::MapView()
{
}

MapView::~MapView()
{
}

void MapView::tick()
{
    auto mapId = MumbleFile::instance.getMapId();
    if (mapId != mapdata.id) {
        mapdata.id = mapId;
        mapdata.fetch();
        return;
    }
    auto& continent = mapdata.continent;
    // guard for the math
    if (continent.dimensions.x == 0 || continent.dimensions.y == 0) {
        return;
    }

    zoom = glm::clamp(continent.minZoom, continent.maxZoom, zoom);

    // how many tiles do i need to display the whole map?
    int tilesPerZoom = static_cast<int>(pow(2, zoom)); // at least 2

    // detect resize. resize might mean a full redraw
    SDL_Rect viewport;
    SDL_RenderGetViewport(Env::mainRenderer, &viewport);
    int wTiles = viewport.w / 256 + 2;
    int hTiles = viewport.h / 256 + 2;
    int tileCountsForDisplay = wTiles * hTiles;
    if (tiles.size() != tileCountsForDisplay) {
        tiles.clear();
        for (int i = 0; i < tileCountsForDisplay; i++) {
            tiles.push_back(VisibleTile());
        }
    }

    // get the player position
    glm::vec3 playerMapPosition = MumbleFile::instance.getPos();
    glm::ivec2 playerWorldCoordinate = glm::vec2(0);
    const auto& continentRect = mapdata.continentRect;
    const auto& mapRect = mapdata.mapRect;
    playerWorldCoordinate.x = static_cast<int>(round(
            continentRect.x + (continentRect.w - continentRect.x) 
            * (playerMapPosition.x - mapRect.x) 
            / (mapRect.w - mapRect.x)
    ));
    playerWorldCoordinate.y = static_cast<int>(round(
            continentRect.h - (continentRect.h - continentRect.y) 
            * (playerMapPosition.z - mapRect.y) 
            / (mapRect.h - mapRect.y)
    ));

    // move the tiles around
    auto pixelOffset
        = coordToPixelOffset(playerWorldCoordinate, zoom, continent.maxZoom);
    pixelOffset -= glm::ivec2(viewport.w / 2, viewport.h / 2);
    auto tileOffset = calculateRelativeOffset(pixelOffset, zoom);

    int apiXStart = (pixelOffset.x) / 256;
    int apiYStart = (pixelOffset.y) / 256;
    for (int y = 0, apiY = apiYStart; y < hTiles; y++, apiY++) {
        for (int x = 0, apiX = apiXStart; x < wTiles; x++, apiX++) {
            int i = y * wTiles + x;
            auto& tile = tiles[i];
            tile.pos.x = apiX * 256 - pixelOffset.x;
            tile.pos.y = apiY * 256 - pixelOffset.y;
            std::string newUrl = std::string("https://tiles.guildwars2.com/")
                + std::to_string(continent.id) + "/"
                + std::to_string(mapdata.defaultFloor) + "/"
                + std::to_string(zoom) + "/"
                + std::to_string(apiX) + "/"
                + std::to_string(apiY) + ".jpg";
            if (tile.url != newUrl) {
                tile.url = newUrl;
                tile.fetch();
            }
        }
    }
}

void MapView::show()
{
    ImGui::Begin("Map Control");
    ImGui::InputInt("Zoom", &zoom);
    ImGui::End();

    for (auto& t : tiles) {
        if (!t.tex) {
            continue;
        }

        SDL_Rect src = { 0, 0, 256, 256 };
        SDL_Rect dst = { t.pos.x, t.pos.y, 256, 256 };
        SDL_RenderCopy(Env::mainRenderer, t.tex, &src, &dst);
    }
}

MapView::VisibleTile::~VisibleTile()
{
    if (tex) {
        SDL_DestroyTexture(tex);
        Fetcher::fetcher.drop(fetchId);
    }
}

void MapView::VisibleTile::fetch()
{
    Fetcher::fetcher.drop(fetchId);
    fetchId = Fetcher::fetcher.fetch(url, [this](const std::vector<char>& data) {
        if (tex) {
            SDL_DestroyTexture(tex);
            tex = nullptr;
        }
        auto surface = IMG_LoadTyped_RW(SDL_RWFromMem((void*)data.data(), static_cast<int>(data.size())), 1, "JPG");
        if (surface) {
            tex = SDL_CreateTextureFromSurface(Env::mainRenderer, surface);
            SDL_FreeSurface(surface);
        }
    },
        (time_t)0);
}
