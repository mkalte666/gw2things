/*
    env.cpp: Functions that interact with the runtime enviorment
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

#include "env.h"
#include <SDL.h>
#include <fstream>

std::string Env::writeablePath = "./"; 
std::string Env::apiKey = "";

SDL_Renderer* Env::mainRenderer = nullptr;
SDL_Window* Env::mainWindow = nullptr;
SDL_GLContext Env::mainGlContext = nullptr;

void Env::init()
{
    auto pPath = SDL_GetPrefPath("mkalte666", "gw2things");
    writeablePath = std::string(pPath);
    SDL_Log("Writeable Path:%s", writeablePath.c_str());
    SDL_free(pPath);

    std::string keyFilename = makeSettingsFilepath("auth.conf");
    std::ifstream keyFile(keyFilename);
    if (keyFile.is_open()) {
        keyFile >> apiKey;
        keyFile.close();
    }
}

std::string Env::makeCacheFilepath(const std::string& filename)
{
    return writeablePath+"_cache"+filename;
}

std::string Env::makeSettingsFilepath(const std::string& filename)
{
    return writeablePath+filename;
}

void Env::setApiKey(const std::string& key)
{
    std::string filename = makeSettingsFilepath("auth.conf");
    std::ofstream file(filename, std::ios::out | std::ios::trunc);
    file << key;
    file.close();
    apiKey = key;
}

std::string Env::getApiKey()
{
    return apiKey;
}


