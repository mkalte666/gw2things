#ifndef GW2CHARACTER_H
#define GW2CHARACTER_H

#include <QObject>
#include <QJsonObject>

class GW2Character : public QObject
{
    Q_OBJECT
public:
    explicit GW2Character(QJsonObject data, QObject *parent = nullptr);

signals:

public slots:
};

#endif // GW2CHARACTER_H
