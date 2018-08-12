#ifndef OVERVIEW_H
#define OVERVIEW_H

#include <QMainWindow>
#include "gw2api.h"
#include "mapview.h"
#include "inventoryview.h"
#include "manualquery.h"

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
    void toggleInventoryView();
    void toggleManualQuery();
private:
    Ui::Overview *ui;
    GW2::GW2Api api;
    MapView* mapview;
    InventoryView* inventoryview;
    ManualQuery* manualquery;
};

#endif // OVERVIEW_H
