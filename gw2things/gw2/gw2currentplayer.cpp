#include "gw2currentplayer.h"
#include "gw2api.h"
#include <QTimer>
GW2CurrentPlayer::GW2CurrentPlayer(QObject *parent)
    : QAbstractTableModel(parent)
{
    updateData();
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GW2CurrentPlayer::updateData);
    timer->start(300); // ever 300 ms
}

int GW2CurrentPlayer::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    // FIXME: Implement me!
    return 8;
}

int GW2CurrentPlayer::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    // FIXME: Implement me!
    return 2;
}

QVariant GW2CurrentPlayer::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            switch(index.row()) {
            case 0: return "Name";
            case 1: return "Position";
            case 2: return "Map Id";
            case 3: return "World ID";
            case 4: return "Profession";
            case 5: return "Race";
            case 6: return "Team Color Id";
            case 7: return "Commander";
            default: return QVariant();
            }
        } else {
            const auto& mumble = GW2Api::getApi()->getCurrentPlayerData();

            switch(index.row()) {
            case 0: return mumble.getName();
            case 1: {
                auto p = mumble.getPos();

                return QString("[%1,%2,%3]").arg(p.x()).arg(p.y()).arg(p.z());
            }
            case 2: return mumble.getMapId();
            case 3: return mumble.getWorldId();
            case 4: return mumble.getProfession();
            case 5: return mumble.getRace();
            case 6: return mumble.getTeamColorId();
            case 7: return mumble.getCommander();
            default: return QVariant();
            }
        }
    }

    // FIXME: Implement me!
    return QVariant();
}

void GW2CurrentPlayer::updateData()
{
    QModelIndex topLeft = createIndex(0,0);
    emit this->dataChanged(topLeft,topLeft);
}
