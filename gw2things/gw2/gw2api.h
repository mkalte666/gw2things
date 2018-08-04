#ifndef GW2API_H
#define GW2API_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QImage>
#include <QMap>

#include "gw2mumblefile.h"

class GW2Api : public QObject
{
    Q_OBJECT
public:
    explicit GW2Api(QObject *parent = nullptr);
    ~GW2Api();
    static GW2Api *getApi();

    bool isGameRunning() const;
    bool isApiAccessable() const;
    const GW2MumbleFile& getCurrentPlayerData() const;

    QNetworkReply* get(QString endpoint, bool cache = false);
    QByteArray getCached(QString endpoint);

    QNetworkReply *getV1(QString endpoint);
    void cacheTile(int continent, int floor, int zoom, int x, int y);
    QImage* getCachedTile(int continent, int floor, int zoom, int x, int y);

    QImage* iconCached(QString name);
    QImage* resourceCached(QString url);
signals:

public slots:

private:
    static GW2Api* gApi;
    GW2MumbleFile mumbleFile;
    QNetworkAccessManager networkManager;
    QMap<QString,QImage*> tileCache;
    QMap<QString,QByteArray> requestCache;
};

#endif // GW2API_H
