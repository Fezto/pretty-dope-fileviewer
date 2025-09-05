#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication prettyDopeFileviewer(argc, argv);

    QApplication::setStyle("windowsvista");

    MainWindow mainWindow;
    mainWindow.show();
    return prettyDopeFileviewer.exec();
}
