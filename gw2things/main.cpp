#include "overview.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication::setOrganizationName("mkalte");
    QApplication::setOrganizationDomain("https://github.com/mkalte666");
    QApplication::setApplicationName("GW2Things");
    QApplication::setApplicationDisplayName("GW2Things");

    QApplication a(argc, argv);
    Overview w;
    w.show();

    return a.exec();
}
