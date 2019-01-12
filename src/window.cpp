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
        if (ImGui::MenuItem("Search Item")) {
            showItemSearch = true;
        }
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
        if (ImGui::MenuItem("Update Item Cache")) {
            itemCache.update();
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

    if (showKeyWindow) {
        keyWindow();
    }

    if (showItemSearch) {
        ImGui::Begin("item search", &showItemSearch);
        if (ImGui::InputText("item query", &itemSearchQuery)) {
            itemQueryResults.clear();
            for (auto itemId : itemCache.query(itemSearchQuery)) {
                auto item = std::make_shared<ItemData>(itemId);
                item->fetch();
                itemQueryResults.push_back(item);
            }
        }

        ImGui::BeginChild("scroll");
        for (auto item : itemQueryResults) {
            item->icon.imguiDraw();
            // context menu on the icon
            if (ImGui::BeginPopupContextItem(item->name.c_str())) {
                if (ImGui::MenuItem("Details")) {
                    item->visible = true;
                }
                if (ImGui::MenuItem("Copy Chat Link")) {
                    SDL_SetClipboardText(item->chatLink.c_str());
                }
                ImGui::EndPopup();
            }
            item->show();

            // the rest
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::TextWrapped("%s", item->name.c_str());
            ImGui::TextWrapped("%s", item->price.niceString().c_str());
            ImGui::EndGroup();

            ImGui::SameLine(ImGui::GetWindowWidth() / 2.0f);
            ImGui::BeginGroup();
            ImGui::Text("Description");
            ImGui::TextWrapped("%s", item->description.c_str());
            ImGui::EndGroup();
            ImGui::Separator();
        }
        ImGui::EndChild();
        ImGui::End();
    }

    overviewData.show();

    mapview.tick();
    mapview.show();
    testItem.show();
    bank.show();
    materialStorage.show();

    return true;
}
