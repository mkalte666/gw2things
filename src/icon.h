#ifndef _icon_h
#define _icon_h

#include <SDL.h>
#include <string>
#include <glm/glm.hpp>
#include "GL/glcorearb.h"
#include "GL/gl3w.h"

class Icon {
public:
    Icon();
    Icon(std::string nameOrUrl);
    Icon(const Icon& other) = delete;
    Icon(Icon&& other) = delete;
    virtual ~Icon();
    void draw(SDL_Renderer* renderer, glm::ivec2 pos);
    void imguiDraw();
    void fetch();
    std::string url;

private:
    SDL_Texture* tex = nullptr;
    GLuint glTex = GL_INVALID_VALUE;
    void makeUrlFromFileResource(std::string name);
    void _fetch();
    int fetchId = 0;
    SDL_Rect srcrect;
    glm::ivec2 size;
};

#endif //_icon_h
