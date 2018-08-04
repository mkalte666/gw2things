#ifndef GW2CURRENTPLAYER_H
#define GW2CURRENTPLAYER_H

#include <QAbstractTableModel>

class GW2CurrentPlayer : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit GW2CurrentPlayer(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
public slots:
    void updateData();
private:
};

#endif // GW2CURRENTPLAYER_H
