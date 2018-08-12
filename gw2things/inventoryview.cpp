#include "inventoryview.h"
#include "gw2/characters/gw2characters.h"


InventoryView::InventoryView(QWidget *parent) : QWidget(parent)
{
    setWindowFlag(Qt::Window, true);
    setAttribute(Qt::WA_DeleteOnClose);

    characterSelector = new QComboBox(this);
    characterSelector->setModel(new GW2Characters(this));

}
