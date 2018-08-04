#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QWidget>
#include <QVector>
#include <QVector2D>

class MapView : public QWidget
{
    Q_OBJECT
public:
    explicit MapView(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

signals:

public slots:
    void updateData();
    void updateMovement();
private:
    int zoom;
    QVector2D center;

    struct Tile {
        int x;
        int y;
        int floor;
        int continent;
        int zoom;
        QVector2D getTopLeft() const;
    };

    struct Icon {
        QVector2D pos;
        QString id;
        QString name;
        bool isRawResource;
    };

    QVector<Tile> visibleTiles;
    QVector<Icon> visibleIcons;
    QRectF mapRect;
    QRectF continentRect;
    bool moved;
};

#endif // MAPVIEW_H
