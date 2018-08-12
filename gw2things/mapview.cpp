#include "mapview.h"
#include <QPainter>
#include <QTimer>
#include <QImage>
#include <QVector2D>
#include <cmath>
#include <QGridLayout>
#include <QPushButton>

#include "gw2api.h"

#define TILE_SIZE 256.0F

QVector2D pixelToWorld(QSize screen, QVector2D center, float zoom, QPoint p)
{
    float worldPerPixel = 32768.0f/(TILE_SIZE*powf(2.0f,zoom));
    float screenW = worldPerPixel * screen.width();
    float screenH = worldPerPixel * screen.height();
    float worldx = center.x()-screenW/2.0f+p.x()*worldPerPixel;
    float worldy = center.y()-screenH/2.0f+p.y()*worldPerPixel;

    return QVector2D(worldx, worldy);
}

QPoint worldToPixel(QSize screen, QVector3D center, float zoom, QVector2D p)
{
    float worldPerPixel = 32768.0f/(TILE_SIZE*powf(2.0f,zoom));
    float screenW = worldPerPixel * screen.width();
    float screenH = worldPerPixel * screen.height();
    // i asked wolfram alpha for this inverse and am to lazy to simplify this
    float pixelX = (p.x()+(screenW/2.0f)-center.x())/worldPerPixel;
    float pixelY = (p.y()+(screenH/2.0f)-center.y())/worldPerPixel;

    return QPointF(static_cast<double>(pixelX),static_cast<double>(pixelY)).toPoint();
}

QVector2D mapToWorld(QRectF mapRect, QRectF continentRect, QVector2D p)
{
    double x = continentRect.left() + continentRect.width()*(static_cast<double>(p.x())-mapRect.left())/mapRect.width();
    double y = continentRect.top() + continentRect.height()*(1.0-(static_cast<double>(p.y())-mapRect.top())/mapRect.height());
    return QVector2D(static_cast<float>(x),static_cast<float>(y));
}

MapView::MapView(QWidget *parent)
    : QWidget(parent)
    , zoom(3)
{
    center = QVector2D(32768.0/2.0,32768.0/2.0);

    setWindowFlag(Qt::Window, true);
    setAttribute(Qt::WA_DeleteOnClose);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MapView::updateData);
    timer->start(1000); // twice per second maybe?

    moved = false;
    QTimer *timer2 = new QTimer(this);
    connect(timer2, &QTimer::timeout, this, &MapView::updateMovement);
    timer2->start(1000/5); // twice per second maybe?

    // zoom buttons
    QGridLayout *layout = new QGridLayout(this); // The layout
    QPushButton *addButton = new QPushButton(QString("+"),this);
    QPushButton *subButton = new QPushButton(QString("-"),this);
    layout->setContentsMargins(10,10,10,10);
    layout->addWidget(addButton,1,1);
    layout->addWidget(subButton,1,2);
    layout->setRowStretch(0, 1);
    layout->setColumnStretch(0, 1);

    connect(addButton,&QPushButton::pressed,[this]() {
       zoom++;
       updateData();
    });
    connect(subButton,&QPushButton::pressed,[this]() {
       zoom--;
       updateData();
    });
}

void MapView::paintEvent(QPaintEvent *event)
{
    (void)event; // not using this atm
    QPainter painter(this);
    auto api = GW2::Api::getApi();

    for (auto t: visibleTiles) {
        auto p = worldToPixel(size(),center,t.zoom,t.getTopLeft());
        auto i = api->getCachedTile(t.continent,t.floor,t.zoom,t.x,t.y);
        if (i) {
            QRect targetRect(p,QSize(256,256));
            painter.drawImage(targetRect,*i,QRect(0,0,255,255));
        }
    }

    for (auto i: visibleIcons) {
        auto p = worldToPixel(size(),center,zoom,i.pos);
        QImage* image = nullptr;
        if (i.isRawResource) {
            image = api->resourceCached(i.id);
        } else {
            image = api->iconCached(i.id);
        }
        if (image) {
            auto s = image->size();
            QRect targetRect(QPoint(p.x()-s.width()/2,p.y()-s.height()/2),s);
            painter.drawImage(targetRect,*image,QRect(0,0,s.width(),s.height()));
        }
    }

    // finally, draw the character position indicator
    QPen pen;
    pen.setWidth(20);
    pen.setColor(Qt::red);
    painter.setPen(pen);
    painter.drawPoint(worldToPixel(size(),center,zoom,center));
    QWidget::paintEvent(event);
}

void MapView::updateData()
{
    //auto pos = api->getCurrentPlayerData().getPos();
    //center = QVector2D(pos.x(),pos.z()); // x is east/west; z is north/south
    visibleTiles.clear();
    //center = QVector2D(32768.0/2.0,32768.0/2.0);
    for (int x = -128; x <= size().width()+256; x+= 256) {
        for (int y = -128; y <= size().height()+256; y+= 256) {
            Tile tile;
            tile.zoom = zoom;
            tile.continent = 1;
            tile.floor = 1;
            auto worldCoord = pixelToWorld(size(),center,zoom,QPoint(x,y));
            tile.x = static_cast<int>(floor(worldCoord.x()*powf(2.f,zoom)/32768.0f));
            tile.y = static_cast<int>(floor(worldCoord.y()*powf(2.f,zoom)/32768.0f));
            if (tile.x >= 0 && tile.x < pow(2,zoom) && tile.y >= 0 && tile.y <= pow(2,zoom)) {
                GW2::Api::getApi()->cacheTile(1,1,zoom,tile.x,tile.y);
                visibleTiles.push_back(tile);
            }
        }
    }

    // reocurring tasks, more or less. fetch map, then fetch pois about map, populate pois array, etc.
    if (GW2::Api::getApi()->isGameRunning()) {
        QString endpoint = QString("maps/%1").arg(GW2::Api::getApi()->getCurrentPlayerData().getMapId());
        GW2::Api::getApi()->get(endpoint,true,[=](QByteArray mapDataArray){
            if (!mapDataArray.isEmpty()) {
                auto mapData = QJsonDocument::fromJson(mapDataArray);
                continentRect.setLeft(mapData["continent_rect"][0][0].toDouble());
                continentRect.setTop(mapData["continent_rect"][0][1].toDouble());
                continentRect.setRight(mapData["continent_rect"][1][0].toDouble());
                continentRect.setBottom(mapData["continent_rect"][1][1].toDouble());
                mapRect.setLeft(mapData["map_rect"][0][0].toDouble()); // top and bottom be switched for what ever fucking reason
                mapRect.setTop(mapData["map_rect"][0][1].toDouble());
                mapRect.setRight(mapData["map_rect"][1][0].toDouble());
                mapRect.setBottom(mapData["map_rect"][1][1].toDouble());
                moved = true;
                //floor = mapData["default_floor"].toInt();

                // up next are pois.
                int region = mapData["region_id"].toInt();
                int continent = mapData["continent_id"].toInt();
                int nFloor = mapData["default_floor"].toInt();
                QString poisEndpoint = QString("continents/%1/floors/%2/regions/%3/maps/%4")
                        .arg(continent)
                        .arg(nFloor)
                        .arg(region)
                        .arg(GW2::Api::getApi()->getCurrentPlayerData().getMapId());
                GW2::Api::getApi()->get(poisEndpoint,true,[=](QByteArray poiDataArray) {
                    if (!poiDataArray.isEmpty()) {
                        auto poiData = QJsonDocument::fromJson(poiDataArray);
                        visibleIcons.clear();
                        auto pois = poiData["points_of_interest"].toObject();
                        for(auto poiRef : pois) {
                            QJsonObject poi = poiRef.toObject();
                            Icon i;
                            i.isRawResource = false;
                            i.name = poi["name"].toString();
                            i.pos.setX(static_cast<float>(poi["coord"].toArray()[0].toDouble()));
                            i.pos.setY(static_cast<float>(poi["coord"].toArray()[1].toDouble()));

                            QString type = poi["type"].toString();
                            if (type == "waypoint") {
                                i.id = "map_waypoint";
                            } else if (type == "vista") {
                                i.id = "map_vista";
                            } else if (type == "landmark") {
                                i.id = "map_poi";
                            } else if (poi.contains("icon")) {
                                i.id = poi["icon"].toString();
                                i.isRawResource = true;
                            }
                            else{
                                continue;
                            }

                            visibleIcons.push_back(i);
                        }
                    }
                });
                moved = true;
            }
        });
    }

    update();
}

void MapView::updateMovement()
{
    if (moved && GW2::Api::getApi()->isGameRunning()) {
        // update map center over here
        auto pos = GW2::Api::getApi()->getCurrentPlayerData().getPos();
        center = mapToWorld(mapRect,continentRect,QVector2D(pos.x(),pos.z()));
        update();
    }
}

QVector2D MapView::Tile::getTopLeft() const
{
    QVector2D p;
    float worldPerPixel = 32768.0f/(TILE_SIZE*powf(2.0f,zoom));;
    p.setX(worldPerPixel*x*TILE_SIZE);
    p.setY(worldPerPixel*y*TILE_SIZE);
    return p;
}
