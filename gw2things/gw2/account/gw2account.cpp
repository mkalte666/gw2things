#include "gw2account.h"
#include "gw2/gw2api.h"
#include <QTimer>
#include <QDebug>

GW2Account::GW2Account(QObject *parent)
    : QAbstractTableModel(parent)
    , valid(false)
{
    updateData();
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GW2Account::updateData);
    timer->start(60000); // ever minute should be enought
}

GW2Account::~GW2Account()
{

}

int GW2Account::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 10;
}

int GW2Account::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 2;
}

QVariant GW2Account::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            switch(index.row()) {
            case 0: return "Name";
            case 1: return "Id";
            case 2: return "Age";
            case 3: return "World";
            case 4: return "Created";
            case 5: return "Commander";
            case 6: return "Fractal Level";
            case 7: return "Daily AP";
            case 8: return "Monthly AP";
            case 9: return "WVW Rank";
            default: return QVariant();
            }
        } else {
            switch(index.row()) {
            case 0: return name;
            case 1: return id;
            case 2: return age;
            case 3: return world;
            case 4: return created;
            case 5: return commander;
            case 6: return fractalLevel;
            case 7: return dailyAp;
            case 8: return monthlyAp;
            case 9: return wvwRank;
            default: return QVariant();
            }
        }
    }

    // FIXME: Implement me!
    return QVariant();
}

QString GW2Account::getId() const
{
    return id;
}

QString GW2Account::getName() const
{
    return name;
}

int GW2Account::getAge() const
{
    return age;
}

int GW2Account::getWorld() const
{
    return world;
}

QDateTime GW2Account::getCreated() const
{
    return created;
}

bool GW2Account::getCommander() const
{
    return commander;
}

int GW2Account::getFractalLevel() const
{
    return fractalLevel;
}

int GW2Account::getDailyAp() const
{
    return dailyAp;
}

int GW2Account::getMonthlyAp() const
{
    return monthlyAp;
}

int GW2Account::getWvwRank() const
{
    return wvwRank;
}

void GW2Account::updateData()
{
    auto reply = GW2Api::getApi()->get("account");
    reply->connect(reply,&QNetworkReply::finished,[this,reply](){
        auto result = reply->readAll();
        auto document = QJsonDocument::fromJson(result);
        id = document["id"].toString();
        name = document["name"].toString();
        age = document["age"].toInt();
        world = document["world"].toInt();
        created = QDateTime::fromString(document["created"].toString(),Qt::DateFormat::ISODate);
        commander = document["commander"].toBool();
        fractalLevel = document["fractal_level"].toInt();
        dailyAp = document["daily_ap"].toInt();
        monthlyAp = document["monthly_ap"].toInt();
        wvwRank = document["wvw_rank"].toInt();
        valid = true;
        QModelIndex topLeft = createIndex(0,0);
        emit this->dataChanged(topLeft,topLeft);
    });
}

bool GW2Account::getValid() const
{
    return valid;
}
