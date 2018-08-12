#ifndef INVENTORYVIEW_H
#define INVENTORYVIEW_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>

class InventoryView : public QWidget
{
    Q_OBJECT
public:
    explicit InventoryView(QWidget *parent = nullptr);

signals:

private:
    QComboBox* characterSelector;
};

#endif // INVENTORYVIEW_H
