#include "gw2api.h"
#include <QSettings>
#include <QtNetwork/QNetworkRequest>
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QBitArray>

GW2Api* GW2Api::gApi = nullptr;

GW2Api::GW2Api(QObject *parent) : QObject(parent)
{
    Q_ASSERT(gApi == nullptr);
    gApi = this;
}

GW2Api::~GW2Api()
{
    Q_ASSERT(gApi == this);
    gApi = nullptr;

    for(auto image: tileCache.values()) {
        delete image;
    }
}

GW2Api *GW2Api::getApi()
{
    return gApi;
}

bool GW2Api::isGameRunning() const
{
    return mumbleFile.isRunning();
}

bool GW2Api::isApiAccessable() const
{
    return QSettings().value("apikey","").toString() != "";
}

const GW2MumbleFile &GW2Api::getCurrentPlayerData() const
{
    return mumbleFile;
}

QNetworkReply *GW2Api::get(QString endpoint, bool cache)
{

    QString urlString = QString("https://api.guildwars2.com/v2/")+endpoint;
    qDebug() << "Fetching " << urlString << "; Into Cache: " << cache;
    QUrl url = QUrl(urlString);
    QNetworkRequest request(url);
    QString key = QSettings().value("apikey", "").toString();
    request.setRawHeader("Authorization", (QString("Bearer ") + key).toUtf8());
    QNetworkReply* reply = networkManager.get(request);
    if (cache) {
        connect(reply,&QNetworkReply::finished,[this,reply,urlString]() {
           requestCache[urlString] = reply->peek(reply->size());
        });
    }

    return reply;
}

QByteArray GW2Api::getCached(QString endpoint)
{
    QString urlString = QString("https://api.guildwars2.com/v2/")+endpoint;
    if (requestCache.contains(urlString)) {
        return requestCache[urlString];
    }
    else {
        (void)get(endpoint,true);
    }

    return QByteArray();
}

QNetworkReply *GW2Api::getV1(QString endpoint)
{
    QUrl url = QUrl(QString("https://api.guildwars2.com/v1/")+endpoint);
    QNetworkRequest request(url);
    return networkManager.get(request);
}

void GW2Api::cacheTile(int continent, int floor, int zoom, int x, int y)
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

    auto result = networkManager.get(request);
    result->connect(result,&QNetworkReply::finished, [=]() {
        QFile cacheFile(cacheFileName);
        cacheFile.open(QIODevice::WriteOnly);
        cacheFile.write(result->readAll());
        cacheFile.close();
    });
}

QImage *GW2Api::getCachedTile(int continent, int floor, int zoom, int x, int y)
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

QImage *GW2Api::iconCached(QString name)
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

        auto reply = get(QString("files?ids=%1").arg(name));
        connect(reply,&QNetworkReply::finished,[this,reply,cacheFileName](){
            reply->deleteLater();
            auto iconInfo = QJsonDocument::fromJson(reply->readAll());
            qDebug() << iconInfo[0]["icon"].toString();
            QUrl iconUrl(iconInfo[0]["icon"].toString());
            QNetworkRequest request(iconUrl);
            auto iconReply = networkManager.get(request);
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

QImage *GW2Api::resourceCached(QString url)
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
        auto resourceReply = networkManager.get(request);
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
