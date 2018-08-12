#include "gw2characters.h"
#include "gw2api.h"
#include <QTimer>

GW2Characters::GW2Characters(QObject *parent)
    : QAbstractListModel(parent)
{
    QTimer *updateTimer = new QTimer(this);
    connect(updateTimer,&QTimer::timeout,this,&GW2Characters::updateData);
    updateTimer->start(60000);
    updateData();
}


int GW2Characters::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    // FIXME: Implement me!
    return characterNames.size();
}

QVariant GW2Characters::data(const QModelIndex &index, int role) const
{
    (void)role;

    if (!index.isValid())
        return QVariant();

    if (index.row() >= 0 && index.row() < characterNames.size()) {
        if (role == Qt::DisplayRole) {
            return characterNames[index.row()];
        }
    }

    // FIXME: Implement me!
    return QVariant();
}

void GW2Characters::updateData()
{
    GW2::Api::getApi()->get("characters",false,[=](QByteArray data) {
       auto json = QJsonDocument::fromJson(data);
       if(json.isArray()) {
           characterNames.clear();
           for (auto c : json.array()) {
               characterNames.push_back(c.toString());
           }
       }
    });
}
