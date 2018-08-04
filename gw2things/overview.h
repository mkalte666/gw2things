#ifndef OVERVIEW_H
#define OVERVIEW_H

#include <QMainWindow>
#include "gw2/gw2api.h"
#include "mapview.h"

namespace Ui {
class Overview;
}

class Overview : public QMainWindow
{
    Q_OBJECT

public:
    explicit Overview(QWidget *parent = nullptr);
    ~Overview();

public slots:
    void setApiKey();
    void updateData();
    void toggleMapView();
private:
    Ui::Overview *ui;
    GW2Api api;
    MapView* mapview;
};

#endif // OVERVIEW_H
