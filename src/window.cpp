/*
    window.cpp: functions for the main window and stuff
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
#include "window.h"

#include "env.h"
#include <SDL.h>
#include <imgui.h>
#include <iostream>
#include <misc/cpp/imgui_stdlib.h>
#include <nlohmann/json.hpp>

// for convenience
using nlohmann::json;

void Window::keyWindow()
{
    ImGui::Begin("Api Key", &showKeyWindow);
    if (ImGui::InputText("Api Key", &tmpApiKey, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue)) {
        Env::setApiKey(tmpApiKey);
    }
    if (ImGui::Button("Ok")) {
        Env::setApiKey(tmpApiKey);
        showKeyWindow = false;
    }
    if (ImGui::Button("Cancel")) {
        showKeyWindow = false;
        tmpApiKey = "";
    }
    ImGui::End();
}

void Window::accountWindow()
{
    ImGui::Begin("Account Overview", &showAccountWindow);
    ImGui::InputText("Account Name", &overviewData.name, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputText("Account ID", &overviewData.id, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputInt("Age", &overviewData.age, 1, 100, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputInt("World", &overviewData.world, 1, 100, ImGuiInputTextFlags_ReadOnly);
    bool copyCommander = overviewData.commander;
    ImGui::Checkbox("Commander", &copyCommander);
    ImGui::InputInt("Fractal Level", &overviewData.fractalLevel, 1, 100, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputInt("Daily AP", &overviewData.dailyAp, 1, 100, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputInt("Monthly AP", &overviewData.monthlyAp, 1, 100, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputInt("WVW Rank", &overviewData.wvwRank, 1, 100, ImGuiInputTextFlags_ReadOnly);
    ImGui::End();
}

bool Window::tick()
{
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Set Api Key...")) {
            tmpApiKey = Env::getApiKey();
            showKeyWindow = true;
        }
        if (ImGui::MenuItem("Quit")) {
            return false;
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Account")) {
        if (ImGui::MenuItem("Overview")) {
            overviewData.fetch();
            overviewData.visible = true;
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Items")) {
        if (ImGui::MenuItem("Bank Content")) {
            bank.fetch();
            bank.visible = true;
        }
        if (ImGui::MenuItem("Material Storage")) {
            materialStorage.fetch();
            materialStorage.visible = true;
        }
        if (ImGui::MenuItem("Overview")) {
            testItem.id = 123;
            testItem.fetch();
            testItem.visible = true;
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

    if (showKeyWindow) {
        keyWindow();
    }

    overviewData.show();

    mapview.tick();
    mapview.show();
    testItem.show();
    bank.show();
    materialStorage.show();

    return true;
}
