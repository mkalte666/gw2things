#include "gw2mumblefile.h"

#ifdef _WIN32
    #include <Windows.h>
#else
    #include <sys/mman.h>
    #include <fcntl.h> /* For O_* constants */
#endif // _WIN32

#include <QDebug>
#include <QTimer>
#include <gw2/gw2api.h>

GW2MumbleFile::GW2MumbleFile(QObject *parent)
    : QObject(parent)
    , valid(false)
    , lm(nullptr)

{
#ifdef _WIN32
    HANDLE hMapObject = OpenFileMapping(FILE_MAP_READ, FALSE, L"MumbleLink");
    if (!hMapObject) {
        qDebug() << "Cannot open, so will create mumble link file!";
        hMapObject = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(LinkedMem), L"MumbleLink");
    }
    if (hMapObject) {
        lm = static_cast<LinkedMem*>(MapViewOfFile(hMapObject, FILE_MAP_READ, 0, 0, sizeof(LinkedMem)));
        if (lm) {
            valid = true;
            Sleep(10);
            qDebug() << "Opend the file! Active thing is " << QString::fromWCharArray(lm->name);
        } else {
            qDebug() << "Cannot map MumbeLink file!";
            CloseHandle(hMapObject);
            hMapObject = nullptr;
        }
    } else {
        qDebug() <<  "Cannot open or create handle to MumbleLink!";
    }
#else
    char memname[256];
    snprintf(memname, 256, "/MumbleLink.%d", getuid());
    int shmfd = shm_open(memname, O_RDWR, S_IRUSR );
    if (shmfd >= 0) {
        lm = (LinkedMem *)(mmap(NULL, sizeof(struct LinkedMem), PROT_READ, MAP_SHARED, shmfd,0));
        if (lm != (void *)(-1)) {
            valid = true;
        }
    }
#endif

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GW2MumbleFile::updateData);
    timer->start(1000); // ever minute should be enought
}

GW2MumbleFile::~GW2MumbleFile()
{

}

bool GW2MumbleFile::isValid() const
{
    return valid;
}

bool GW2MumbleFile::isRunning() const
{
    return isValid() && QString::fromWCharArray(lm->name) == "Guild Wars 2";
}

QVector3D GW2MumbleFile::getPos() const
{
    return QVector3D(lm->fAvatarPosition[0]*39.3701f,
            lm->fAvatarPosition[1]*39.3701f,
            lm->fAvatarPosition[2]*39.3701f);
}

QString GW2MumbleFile::getName() const
{
    return name;
}

int GW2MumbleFile::getProfession() const
{
    return profession;
}

int GW2MumbleFile::getRace() const
{
    return race;
}

int GW2MumbleFile::getMapId() const
{
    return mapId;
}

int GW2MumbleFile::getWorldId() const
{
    return worldId;
}

int GW2MumbleFile::getTeamColorId() const
{
    return teamColorId;
}

bool GW2MumbleFile::getCommander() const
{
    return commander;
}

void GW2MumbleFile::updateData()
{
    if (!isValid()) {
        return;
    }

    auto doc = QJsonDocument::fromJson(QString::fromWCharArray(lm->identity).toUtf8()); // why
    name = doc["name"].toString();
    profession = doc["profession"].toInt();
    race = doc["race"].toInt();
    mapId = doc["map_id"].toInt();
    worldId = doc["world_id"].toInt();
    teamColorId = doc["team_color_id"].toInt();
    commander = doc["commander"].toBool();
}
