/*
    env.h: Interface (Globals) that define the programs runtime enviorment
    Copyright (C) 2018 Malte Kieﬂling

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef _env_h
#define _env_h

#include <string>
#include <SDL.h>

class Env {
public:
    static void init();

    static std::string makeCacheFilepath(const std::string& filename);
    static std::string makeSettingsFilepath(const std::string& filename);

    static void setApiKey(const std::string& key);
    static std::string getApiKey();

    static SDL_Renderer* mainRenderer;
    static SDL_Window* mainWindow;
    static SDL_GLContext mainGlContext;

private:
    static std::string writeablePath;
    static std::string apiKey;
};

#endif //_env_h