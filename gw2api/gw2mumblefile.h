#ifndef GW2MUMBLEFILE_H
#define GW2MUMBLEFILE_H

#include <QObject>
#include <QVector3D>

namespace GW2 {

/**
 * @brief The LinkedMem struct
 * @see https://wiki.guildwars2.com/wiki/API:MumbleLink
 * @see https://wiki.mumble.info/wiki/Link
 */
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

/**
 * @brief The GW2MumbleFile class
 * Interfaces with the MumbleLink file
 */
class MumbleFile : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief GW2MumbleFile
     * @param parent
     */
    explicit MumbleFile(QObject *parent = nullptr);
    ~MumbleFile();

    /**
     * @brief isValid
     * @return
     */
    bool isValid() const;
    /**
     * @brief isRunning
     * @return
     */
    bool isRunning() const;

    /**
     * @brief getPos
     * @return the position of the player. map coordinates; unit is feet
     */
    QVector3D getPos() const;

    /**
     * @brief getName
     * @return
     */
    QString getName() const;

    /**
     * @brief getProfession
     * @return
     */
    int getProfession() const;

    /**
     * @brief getRace
     * @return
     */
    int getRace() const;

    /**
     * @brief getMapId
     * @return
     */
    int getMapId() const;

    /**
     * @brief getWorldId
     * @return
     */
    int getWorldId() const;

    /**
     * @brief getTeamColorId
     * @return
     */
    int getTeamColorId() const;

    /**
     * @brief getCommander
     * @return
     */
    bool getCommander() const;

signals:

public slots:
    /**
     * @brief updateData update the data from the mumble link file. Called from a timer
     */
    void updateData();

private:
    bool valid; ///< if the thing is valid
    LinkedMem* lm; ///< the mumblelink
    QString name; ///< player name
    int profession; ///< player profession
    int race; ///< player race
    int mapId; ///< player map id
    int worldId; ///< player world id
    int teamColorId; ///< player color id
    bool commander; ///< if the player is a commander
};

}; // namespace GW2

#endif // GW2MUMBLEFILE_H
