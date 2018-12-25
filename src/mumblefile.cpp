#include "mumblefile.h"
#include <SDL.h>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#else
#include <fcntl.h> /* For O_* constants */
#include <sys/mman.h>
#endif // _WIN32

#include <nlohmann/json.hpp>
using nlohmann::json;

MumbleFile MumbleFile::instance;

MumbleFile::MumbleFile()
    : valid(false)
    , lm(nullptr)

{
#ifdef _WIN32
    HANDLE hMapObject = OpenFileMapping(FILE_MAP_READ, FALSE, "MumbleLink");
    if (!hMapObject) {
        SDL_Log("Cannot open mumble link, creating...");
        hMapObject = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(LinkedMem), "MumbleLink");
    }
    if (hMapObject) {
        lm = static_cast<LinkedMem*>(MapViewOfFile(hMapObject, FILE_MAP_READ, 0, 0, sizeof(LinkedMem)));
        if (lm) {
            valid = true;
            Sleep(10);
            SDL_Log("Mumble file is open: %s", lm->name);
        } else {
            SDL_Log("Cannot open mumble link :(");
            CloseHandle(hMapObject);
            hMapObject = nullptr;
            abort();
        }
    } else {
        SDL_Log("Cannot open or create Mumble link file :(");
        abort();
    }
#else
    char memname[256];
    snprintf(memname, 256, "/MumbleLink.%d", getuid());
    int shmfd = shm_open(memname, O_RDWR, S_IRUSR);
    if (shmfd >= 0) {
        lm = (LinkedMem*)(mmap(NULL, sizeof(struct LinkedMem), PROT_READ, MAP_SHARED, shmfd, 0));
        if (lm != (void*)(-1)) {
            valid = true;
        }
    }
#endif

    std::thread([this]() {
        using namespace std; // for the chrono suffix
        for(;;) {
            updateData();
            this_thread::sleep_for(10s);
        }
    })
        .detach();
}

MumbleFile::~MumbleFile()
{
}

bool MumbleFile::isValid() const
{
    return valid;
}

bool MumbleFile::isRunning() const
{
    return isValid() && std::wstring(lm->name) == L"Guild Wars 2";
}

glm::vec3 MumbleFile::getPos() const
{
    return glm::vec3(lm->fAvatarPosition[0] * 39.3701,
        lm->fAvatarPosition[1] * 39.3701,
        lm->fAvatarPosition[2] * 39.3701);
}

std::string MumbleFile::getName() const
{
    return name;
}

int MumbleFile::getProfession() const
{
    return profession;
}

int MumbleFile::getRace() const
{
    return race;
}

int MumbleFile::getMapId() const
{
    return mapId;
}

int MumbleFile::getWorldId() const
{
    return worldId;
}

int MumbleFile::getTeamColorId() const
{
    return teamColorId;
}

bool MumbleFile::getCommander() const
{
    return commander;
}

void MumbleFile::updateData()
{
    if (!isValid()) {
        return;
    }

    SDL_Log("Updating static mumble file data");

    // why cant you just utf8
    std::wstring wstring(lm->identity);
    std::string jsonString(wstring.begin(), wstring.end());
    if (jsonString.empty()) {
        return;
    }
    SDL_Log("Got: %s", jsonString.c_str());

    auto doc = json::parse(jsonString);
    name = doc.value("name", "???");
    profession = doc.value("profession", 0);
    race = doc.value("race", 0);
    mapId = doc.value("map_id", 0);
    worldId = doc.value("world_id", 0);
    teamColorId = doc.value("team_color_id", 0);
    commander = doc.value("commander", false);
}
