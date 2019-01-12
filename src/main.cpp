/*
    main.c: entry point
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

// clang-format off
#include "imgui.h"
#include "examples/imgui_impl_opengl3.h"
#include "examples/imgui_impl_sdl.h"
// clang-format on

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include "GL/glcorearb.h"
#include "GL/gl3w.h"

#include <iostream>
#include "window.h"
#include "env.h"
#include "workpool.h"

int main(int argc, char* argv[])
{
#ifdef _DEBUG
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);
#endif
    int rc = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_VIDEO | SDL_INIT_TIMER);
    if (rc < 0) {
        std::cerr << "Cannot Init SDL! " << SDL_GetError() << std::endl;
        exit(1);
    }

    rc = TTF_Init();
    if (rc < 0) {
        std::cerr << "Cannot init TTF!" << TTF_GetError() << std::endl;
        exit(2);
    }

    rc = IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
    if (rc < 0) {
        std::cerr << "Cannot init IMG!" << IMG_GetError() << std::endl;
        exit(2);
    }

    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");

    // we are runnable. put in own scope so we run some destructors of stack objects before everything explodes
    {
        Workpool::init(3);
        // GL 3.3 + GLSL 130
        const char* glsl_version = "#version 100";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        // Create window with graphics context
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_DisplayMode current;
        SDL_GetCurrentDisplayMode(0, &current);

        Env::mainWindow = SDL_CreateWindow("GW2Things", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        if (!Env::mainWindow) {
            std::cerr << "Cannot create window" << SDL_GetError() << std::endl;
            exit(3);
        }

        Env::mainRenderer = SDL_CreateRenderer(Env::mainWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
        if (!Env::mainRenderer) {
            std::cerr << "Cannot create renderer" << SDL_GetError() << std::endl;
            exit(3);
        }
        Env::mainGlContext = SDL_GL_CreateContext(Env::mainWindow);
        SDL_GL_SetSwapInterval(1);

        // env
        Env::init();

        // gl3w
        auto gl3wres = gl3wInit();
        bool err = gl3wres != 0;
        if (err) {
            std::cerr << "Failed to init gl3w:" << gl3wres << std::endl;
            if (gl3wres == GL3W_ERROR_OPENGL_VERSION) {
                std::cerr << "trying to continue with broken gl3w cause its a version mismatch error that can happen with gles contexts" << std::endl
                          << "this might crash" << std::endl;
            } else {
                exit(3);
            }
        }
        // imgui setup
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        ImGui_ImplSDL2_InitForOpenGL(Env::mainWindow, Env::mainGlContext);
        ImGui_ImplOpenGL3_Init(glsl_version);
        (void)io;
        ImGui::StyleColorsDark();

        //SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
        // init the "window"
        Window gui;

        bool running
            = true;
        double deltaTime = 0.01;

        uint64_t now = SDL_GetPerformanceCounter();
        uint64_t last = 0;
        while (running) {
            Workpool::instance.tick();
            // event polling is here
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                ImGui_ImplSDL2_ProcessEvent(&e);
                if (e.type == SDL_QUIT) {
                    running = false;
                }
            }

            // pre-render window clean
            SDL_SetRenderDrawColor(Env::mainRenderer, 0, 0, 0, 255);
            SDL_RenderClear(Env::mainRenderer);

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame(Env::mainWindow);
            ImGui::NewFrame();
            SDL_GL_MakeCurrent(Env::mainWindow, Env::mainGlContext);
            // tick the window
            Fetcher::fetcher.tick();
            running = gui.tick();

            // flush the renderer before we do our own code
            // does its own context switch
            SDL_RenderFlush(Env::mainRenderer);

            // now render imgui
            SDL_GL_MakeCurrent(Env::mainWindow, Env::mainGlContext);
            ImGui::Render(); // thos does not actually render (as in: draw calls)
            glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // this does, though
            // and swap
            SDL_GL_SwapWindow(Env::mainWindow);

            // time keeping
            deltaTime = (double)((now - last) / (double)SDL_GetPerformanceFrequency());
            last = now;
            now = SDL_GetPerformanceCounter();
            if (deltaTime < 0.008) {
                SDL_Delay(static_cast<int>((0.008 - deltaTime) * 1000));
            }
        }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        SDL_GL_DeleteContext(Env::mainGlContext);

        SDL_DestroyRenderer(Env::mainRenderer);
        SDL_DestroyWindow(Env::mainWindow);
    }

    TTF_Quit();
    SDL_Quit();

    return 0;
}
