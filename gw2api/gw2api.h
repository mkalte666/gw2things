#ifndef GW2Api_H
#define GW2Api_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QNetworkAccessManager>
#include <QImage>

#include "gw2common.h"
#include "gw2mumblefile.h"
#include "gw2account.h"
#include "gw2character.h"

/**
 * Holds are GW2 Api classes
 */
namespace GW2 {

class Api : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Generic callback used by the get function
     */
    using GenericCallback = std::function<void(QByteArray)>;

    /**
     * @brief Api
     * @param apiKey api key
     * @param parent parent object
     */
    Api(QString apiKey, QObject *parent = nullptr);

    /**
    */
    ~Api();

    /**
     * @brief apiKey
     * @return The Api Key
     */
    QString apiKey() const;
    /**
     * @brief setApiKey
     * @param apiKey the new api key
     */
    void setApiKey(const QString &apiKey);

    /**
     * @brief isGameRunning
     * @return true if the game is detected as running
     */
    bool isGameRunning() const;
    /**
     * @brief isApiAccessable
     * @return true if the set api key appears valid
     */
    bool isApiAccessable() const;
    /**
     * @brief getCurrentPlayerData
     * @return the connection to the currently running game
     */
    const MumbleFile& getCurrentPlayerData() const;

    /**
     * @brief get - make a (authorized) request against the v2 api
     * @param endpoint the endpoint to ask, something like "characters" or "account"
     * @param cache if set to true, the request may be fetched from the internal cache. Not that this is not reloaded automatically
     * @param callback the callback to call when the data is fetched sucessfully
     */
    void get(QString endpoint, bool cache, GenericCallback callback);

    /**
     * @brief Caches the specified tile
     * @param continent The continent id of the continent this tile is on
     * @param floor Id of the floor
     * @param zoom Zoom level
     * @param x X Coordinate of the tile
     * @param y Y Coordinate of the tile
     */
    void cacheTile(int continent, int floor, int zoom, int x, int y);
    /**
     * @brief getCachedTile
     * @param continent
     * @param floor
     * @param zoom
     * @param x
     * @param y
     * @see cacheTile
     * @return QImage* with the cached tile. nullptr if there are problems
     */
    QImage* getCachedTile(int continent, int floor, int zoom, int x, int y);

    /**
     * @brief Caches and, if cached, returns an icon from file endpoint
     * @param name
     * @return QImage* or nullptr, depending if its cached or not
     */
    QImage* iconCached(QString name);
    /**
     * @brief resourceCached
     * @param url
     * @return a cached resource QImage*, or nullptr if its not yet cached
     */
    QImage* resourceCached(QString url);

    void getAccount(Account::Callback callback);

    void getCharacter(QString name, Character::Callback callback);
    void getCharacters(Character::Callback callback);
    /**
     * @brief gets the api singleton
     * @return pointer to the api
     */
    static Api *getApi();

private:
    static Api* _gApi;
    QString _apiKey; /// api key
    MumbleFile _mumbleFile; ///< mumble file connection

    QNetworkAccessManager _networkManager; ///< network manager used by this classes requests
    QMap<QString,QImage*> tileCache; ///< caches all the images
    QMap<QString,QByteArray> _requestCache; ///< caches all the requests
};

}; // namespace GW2

#endif // GW2Api_H
