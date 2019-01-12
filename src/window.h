/*
    window.h: Interface of the main window class
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
#ifndef _window_h
#define _window_h

#include "api/itemcache.h"
#include "api/accountoverview.h"
#include "api/bank.h"
#include "api/item.h"
#include "api/materialstorage.h"
#include "mapview.h"
#include <fetcher.h>

class Window {
public:
    void keyWindow();
    void accountWindow();
    bool tick();

private:
    bool showKeyWindow = false;
    bool showAccountWindow = false;
    AccountOverviewData overviewData;
    ItemData testItem;
    std::string tmpApiKey = "";
    MapView mapview;
    BankData bank;
    MaterialStorageData materialStorage;
    ItemCache itemCache;
};

#endif