/**
 *  PrettyDopeFileviewer – entry point.
 *
 *  Small application with a clear goal: open a PDF and make it feel friendly.
 *  It then evolved into layered components (document, pages, viewer, main
 *  window) to showcase clean Qt architecture practices!
 *
 *  Some things are still WIP.
 *
 *  Here we create QApplication, set a style and show the main window. No
 *  extra logic – keep it clean.
 */

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    // QApplication drives the Qt event loop.
    QApplication prettyDopeFileviewer(argc, argv);

    // Visual style (can be changed per platform / preference).
    // QApplication::setStyle("windowsvista");

    // Main window (contains PDFViewer and actions).
    MainWindow mainWindow;
    mainWindow.show();

    // Enter event loop – app lives until window is closed.
    return prettyDopeFileviewer.exec();
}
