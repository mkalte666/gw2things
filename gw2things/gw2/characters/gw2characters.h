#ifndef GW2CHARACTERS_H
#define GW2CHARACTERS_H

#include <QAbstractListModel>
#include <QList>

class GW2Characters : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit GW2Characters(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
    void updateData();
private:

    QList<QString> characterNames;
};

#endif // GW2CHARACTERS_H
