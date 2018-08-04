#include "overview.h"
#include "ui_overview.h"

#include <QInputDialog>
#include <QSettings>
#include <QTimer>

#include <gw2/account/gw2account.h>
#include <gw2/gw2currentplayer.h>

Overview::Overview(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Overview),
    mapview(nullptr)
{
    ui->setupUi(this);


    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Overview::updateData);
    timer->start(500);
    ui->accountTable->setModel(new GW2Account(this));
    ui->playerTable->setModel(new GW2CurrentPlayer(this));
}

Overview::~Overview()
{
    delete ui;
}

void Overview::setApiKey()
{
    bool ok;
    QString key = QInputDialog::getText(this, tr("Set GW2 Api Key"), tr("Key: "), QLineEdit::Password, "",&ok);
    if (ok) {
        QSettings().setValue("apikey",key);
    }
}

void Overview::updateData()
{
    if (!api.isApiAccessable()) {
        emit statusBar()->showMessage(tr("Api is not accessable. Do you have internet connection and is the key set?"), 500);
    }
    else if (!api.isGameRunning()) {
        emit statusBar()->showMessage(tr("Cannot access the game. Is it running?"), 500);
    }

    ui->accountTable->viewport()->repaint();
    ui->playerTable->viewport()->repaint();
}

void Overview::toggleMapView()
{
    if (mapview) {
        mapview->close();
        mapview->deleteLater();
        return;
    }

    mapview = new MapView(this);
    mapview->show();
    connect(mapview,&QWidget::destroyed,[this]() {
        mapview = nullptr;
    });
}
