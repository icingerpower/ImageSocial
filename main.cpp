#include "MainWindow.h"

#include "../common/config/SettingsManager.h"
#include "../common/config/DialogOpenSettingsFile.h"
#include "../common/types/types.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<QList<QList<QVariant>>>();
    QApplication a(argc, argv);
    DialogOpenSettingsFile dialog;
    dialog.exec();
    if (!dialog.wasAccepted())
    {
        return 0;
    }
    SettingsManager::instance()->open(
                dialog.getFilePath());
    MainWindow w;
    w.show();
    return a.exec();
}
