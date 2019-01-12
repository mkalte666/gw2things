#ifndef _overlay_h
#define _overlay_h

#include <SDL.h>


class Overlay
{
public:
    static Overlay instance;

private:
    Overlay();
    
    Overlay(const Overlay& other) = delete;
    Overlay(Overlay&& other) = delete;

    SDL_Window* window = nullptr;
};

#endif // _overlay_h