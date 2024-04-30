#include "MainWindow.h"

#include "../common/types/types.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<QList<QList<QVariant>>>();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
