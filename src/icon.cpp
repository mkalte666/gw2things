#include "icon.h"

#include "env.h"
#include "fetcher.h"
#include <SDL_image.h>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include "workpool.h"

using nlohmann::json;

GLuint SurfaceToTexture(SDL_Surface* s)
{

    GLenum textureFormat = GL_BGR;
    if (s->format->BytesPerPixel == 4) {
        if (s->format->Rmask == 0x000000ff) {
            textureFormat = GL_RGBA;
        } else {
            textureFormat = GL_BGRA;
        }
    } else {
        if (s->format->Rmask == 0x0000000ff) {
            textureFormat = GL_RGB;
        } else {
            textureFormat = GL_BGR;
        }
    }

    //Throw at ogl
    SDL_GL_MakeCurrent(Env::mainWindow, Env::mainGlContext);
    GLuint TextureId = GL_INVALID_VALUE;
    glGenTextures(1, &TextureId);
    if (TextureId == GL_INVALID_VALUE) {
        return GL_INVALID_VALUE;
    }
    glBindTexture(GL_TEXTURE_2D, TextureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s->w, s->h, 0, textureFormat, GL_UNSIGNED_BYTE, s->pixels);
    return TextureId;
}

Icon::Icon()
{
}

Icon::Icon(std::string nameOrUrl)
{
    url = nameOrUrl;
    fetch();
}

Icon::~Icon()
{
    if (tex) {
        SDL_DestroyTexture(tex);
        tex = nullptr;
    }
    if (glTex != GL_INVALID_VALUE) {
        SDL_GL_MakeCurrent(Env::mainWindow, Env::mainGlContext);
        glDeleteTextures(1, &glTex);
    }

    Fetcher::fetcher.drop(fetchId);
}

void Icon::draw(SDL_Renderer* renderer, glm::ivec2 pos)
{
    if (!renderer || !tex) {
        return;
    }

    SDL_Rect dstRect = { pos.x - srcrect.w / 2, pos.y - srcrect.h / 2, srcrect.w, srcrect.h };
    SDL_RenderCopy(renderer, tex, &srcrect, &dstRect);
}

void Icon::imguiDraw()
{
    if (glTex == GL_INVALID_VALUE) {
        return;
    }
    // converted id gets rid of the warning that i try to plug glTex into void*
    ImVec2 size(static_cast<float>(srcrect.w), static_cast<float>(srcrect.h));
    ImGui::Image((void*)(intptr_t)glTex, size, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0));
}

void Icon::fetch()
{
    if (url.find("render.guildwars2.com") != -1) {
        _fetch();
        return;
    }

    makeUrlFromFileResource(url);
}

void Icon::makeUrlFromFileResource(std::string name)
{
    std::string fileEndpointUrl = "https://api.guildwars2.com/v2/files/" + name;

    fetchId = Fetcher::fetcher.fetch(fileEndpointUrl, [this](const std::vector<char>& data) {
        auto j = json::parse(data);
        auto iconUrl = j.value("icon", "");
        if (!iconUrl.empty()) {
            url = iconUrl;
            _fetch();
        }
    },
        static_cast<time_t>(0));
}

void Icon::_fetch()
{
    fetchId = Fetcher::fetcher.fetch(url, [this](const std::vector<char>& data) {
        if (tex) {
            SDL_DestroyTexture(tex);
            tex = nullptr;
        }
        if (glTex != GL_INVALID_VALUE) {
            SDL_GL_MakeCurrent(Env::mainWindow, Env::mainGlContext);
            glDeleteTextures(1, &glTex);
            glTex = GL_INVALID_VALUE;
        }
        surface = nullptr;
        Workpool::instance.addTask([this,data]() { 
            surface = IMG_LoadTyped_RW(SDL_RWFromMem((void*)data.data(), static_cast<int>(data.size())), 1, "PNG");
            },
        [this]{
            if (surface) {
                // common
                srcrect = { 0, 0, surface->w, surface->h };

                // texture for sdl and imgui
                tex = SDL_CreateTextureFromSurface(Env::mainRenderer, surface);
                glTex = SurfaceToTexture(surface);
                SDL_FreeSurface(surface);
                surface = nullptr;
            }
            });
        
    },
        static_cast<time_t>(0));
}
