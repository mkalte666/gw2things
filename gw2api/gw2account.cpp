#include "gw2account.h"

namespace GW2 {

Account::Account(const QJsonObject &data)
{
    id = data["id"].toString();
    name = data["name"].toString();
    age = data["age"].toInt();
    world = data["world"].toInt();
    created = QDateTime::fromString(data["created"].toString(),Qt::DateFormat::ISODate);
    commander = data["commander"].toBool();
    fractalLevel = data["fractal_level"].toInt();
    dailyAp = data["daily_ap"].toInt();
    monthlyAp = data["monthly_ap"].toInt();
    wvwRank = data["wvw_rank"].toInt();
}

QString Account::getId() const
{
    return id;
}

QString Account::getName() const
{
    return name;
}

int Account::getAge() const
{
    return age;
}

int Account::getWorld() const
{
    return world;
}

QDateTime Account::getCreated() const
{
    return created;
}

bool Account::getCommander() const
{
    return commander;
}

int Account::getFractalLevel() const
{
    return fractalLevel;
}

int Account::getDailyAp() const
{
    return dailyAp;
}

int Account::getMonthlyAp() const
{
    return monthlyAp;
}

int Account::getWvwRank() const
{
    return wvwRank;
}

}; // namespace GW2
