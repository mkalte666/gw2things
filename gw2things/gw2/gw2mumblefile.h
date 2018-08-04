#ifndef GW2MUMBLEFILE_H
#define GW2MUMBLEFILE_H

#include <QObject>
#include <QVector3D>

struct LinkedMem {
    uint32_t uiVersion;
    uint32_t uiTick;
    float	fAvatarPosition[3];
    float	fAvatarFront[3];
    float	fAvatarTop[3];
    wchar_t	name[256];
    float	fCameraPosition[3];
    float	fCameraFront[3];
    float	fCameraTop[3];
    wchar_t	identity[256];
    uint32_t context_len;
    unsigned char context[256];
    wchar_t description[2048];
};

class GW2MumbleFile : public QObject
{
    Q_OBJECT
public:
    explicit GW2MumbleFile(QObject *parent = nullptr);
    ~GW2MumbleFile();

    bool isValid() const;
    bool isRunning() const;


    QVector3D getPos() const;

    QString getName() const;

    int getProfession() const;

    int getRace() const;

    int getMapId() const;

    int getWorldId() const;

    int getTeamColorId() const;

    bool getCommander() const;

signals:

public slots:
    void updateData();

private:
    bool valid;
    LinkedMem* lm;
    QString name;
    int profession;
    int race;
    int mapId;
    int worldId;
    int teamColorId;
    bool commander;
};

#endif // GW2MUMBLEFILE_H
