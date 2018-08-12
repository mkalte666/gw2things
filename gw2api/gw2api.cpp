#include "gw2api.h"

#include <QNetworkReply>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

namespace GW2 {

Api* Api::_gApi = nullptr;

Api::Api(QString apiKey, QObject* parent)
    : QObject(parent)
    , _apiKey(apiKey)
{
    Q_ASSERT(_gApi == nullptr);

    _gApi = this;
}

Api::~Api()
{
    Q_ASSERT(_gApi == this);
    _gApi = nullptr;

    // lots of async stuff, so we this
    auto copyCache = std::move(tileCache);
    tileCache.clear();

    for(auto image: copyCache.values()) {
        delete image;
    }
}

QString Api::apiKey() const
{
    return _apiKey;
}

void Api::setApiKey(const QString &apiKey)
{
    _apiKey = apiKey;
}

bool Api::isGameRunning() const
{
    return _mumbleFile.isRunning();
}

bool Api::isApiAccessable() const
{
    return !_apiKey.isEmpty() && _apiKey != "";
}

const MumbleFile &Api::getCurrentPlayerData() const
{
    return _mumbleFile;
}

void Api::get(QString endpoint, bool cache, Api::GenericCallback callback)
{
    // construct the url
    QString urlString = QString("https://api.guildwars2.com/v2/")+endpoint;
    qDebug() << "Fetching " << urlString << "; Into/From Cache: " << cache;
    QUrl url = QUrl(urlString);
    // check against cache
    if (cache && callback && _requestCache.contains(urlString)) {
        qDebug() << ">>>> Was from Cache!";
        callback(_requestCache[urlString]);
        return;
    }

    // construct request
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", (QString("Bearer ") + _apiKey).toUtf8());
    QNetworkReply* reply = _networkManager.get(request);

    // handle reply
    connect(reply,&QNetworkReply::finished, [=](){
        auto data = reply->readAll();
        if (cache) {
            _requestCache[urlString] = data;
        }
        if (callback) {
            callback(data);
        }

        // cleanup is important
        reply->deleteLater();
    });
}

void Api::cacheTile(int continent, int floor, int zoom, int x, int y)
{
    // check if file exsists
    QString identifier = QString("%1-%2-%3-%4-%5.jpg");
    if (tileCache.contains(identifier)) {
        return;
    }


    identifier = identifier.arg(continent).arg(floor).arg(zoom).arg(x).arg(y);
    QString path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir cacheDir(path);
    cacheDir.mkpath(path);
    QString cacheFileName(cacheDir.filePath(identifier));
    if (QFile::exists(cacheFileName)) {
        return;
    }


    qDebug() << "Fetching tile " << identifier << "into cache";
    // touch file so we dont dup requests
    QFile tmp(cacheFileName);
    tmp.open(QIODevice::WriteOnly);
    tmp.close();

    // fetch and create file
    QString urlbase("https://tiles.guildwars2.com/%1/%2/%3/%4/%5.jpg");
    QString urlstring = urlbase.arg(continent).arg(floor).arg(zoom).arg(x).arg(y);
    QUrl url = QUrl(urlstring);
    QNetworkRequest request(url);
    QString key = QSettings().value("apikey", "").toString();
    request.setRawHeader("Authorization", (QString("Bearer ") + key).toUtf8());

    auto result = _networkManager.get(request);
    result->connect(result,&QNetworkReply::finished, [=]() {
        QFile cacheFile(cacheFileName);
        cacheFile.open(QIODevice::WriteOnly);
        cacheFile.write(result->readAll());
        cacheFile.close();
    });
}

QImage *Api::getCachedTile(int continent, int floor, int zoom, int x, int y)
{
    QString identifier = QString("%1-%2-%3-%4-%5.jpg");
    identifier = identifier.arg(continent).arg(floor).arg(zoom).arg(x).arg(y);

    if (tileCache.contains(identifier)) {
        return tileCache[identifier];
    }

    QString path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir cacheDir(path);
    cacheDir.mkpath(path);
    QString cacheFileName(cacheDir.filePath(identifier));
    if (!QFile::exists(cacheFileName)) {
        return nullptr;
    }

    QImage* i = new QImage();
    if (i->load(cacheFileName)) {
        tileCache[identifier] = i;
        return i;
    }

    return nullptr;
}

QImage *Api::iconCached(QString name)
{
    if (tileCache.contains(name)) {
        return tileCache[name];
    }

    QString path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir cacheDir(path);
    cacheDir.mkpath(path);
    QString cacheFileName(cacheDir.filePath(name));

    if (!QFile::exists(cacheFileName)) {
        qDebug() << "Fetching file/icon " << name << "Into Cache";
        // we avoid double requests to iconCached with this
        QFile tmp(cacheFileName);
        tmp.open(QIODevice::WriteOnly);
        tmp.close();

        get(QString("files?ids=%1").arg(name),true,[=](QByteArray data){;
            auto iconInfo = QJsonDocument::fromJson(data);
            qDebug() << iconInfo[0]["icon"].toString();
            QUrl iconUrl(iconInfo[0]["icon"].toString());
            QNetworkRequest request(iconUrl);
            auto iconReply = _networkManager.get(request);
            connect(iconReply,&QNetworkReply::finished,[cacheFileName,iconReply](){
                iconReply->deleteLater();
                QFile cacheFile(cacheFileName);
                cacheFile.open(QIODevice::WriteOnly);
                cacheFile.write(iconReply->readAll());
                cacheFile.close();
            });
        });

        return nullptr;
    }

    QImage* i = new QImage();
    if (i->load(cacheFileName)) {
        tileCache[name] = i;
        return i;
    }

    return nullptr;
}

QImage *Api::resourceCached(QString url)
{
    if (tileCache.contains(url)) {
        return tileCache[url];
    }

    // generate filename-safe thing
    QString safeName = url.toUtf8().toBase64();

    // the usual
    QString path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir cacheDir(path);
    cacheDir.mkpath(path);
    QString cacheFileName(cacheDir.filePath(safeName));

    if (!QFile::exists(cacheFileName)) {
        qDebug() << "Fetching Resource " << url << "Into Cache";
        // we avoid double requests to iconCached with this
        QFile tmp(cacheFileName);
        tmp.open(QIODevice::WriteOnly);
        tmp.close();
        QUrl requestUrl(url);
        QNetworkRequest request(requestUrl);
        auto resourceReply = _networkManager.get(request);
        connect(resourceReply,&QNetworkReply::finished,[cacheFileName,resourceReply](){
            resourceReply->deleteLater();
            QFile cacheFile(cacheFileName);
            cacheFile.open(QIODevice::WriteOnly);
            cacheFile.write(resourceReply->readAll());
            cacheFile.close();
        });

        return nullptr;
    }

    QImage* i = new QImage();
    if (i->load(cacheFileName)) {
        tileCache[url] = i;
        return i;
    }

    return nullptr;
}

void Api::getAccount(Account::Callback callback)
{
    if (callback) {
        get("account", false, [callback](QByteArray data){
            auto json = QJsonDocument::fromJson(data).object();
            Account a(json);
            callback(a);
        });
    }
}

void Api::getCharacter(QString name, Character::Callback callback)
{
    if (callback) {
        auto endpoint = QString("characters/%1").arg(name);
        get(endpoint, false, [callback](QByteArray data){
            auto json = QJsonDocument::fromJson(data).object();
            Character c(json);
            callback(c);
        });
    }
}

void Api::getCharacters(Character::Callback callback)
{
    if (callback) {
        get("characters", false, [this,callback](QByteArray data){
            auto json = QJsonDocument::fromJson(data).array();
            for (auto characterName : json) {
                getCharacter(characterName.toString(), callback);
            }
        });
    }
}

Api *Api::getApi()
{
    return _gApi;
}

}; //namespace GW2
