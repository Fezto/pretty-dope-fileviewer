#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "pdfdocument.h"
#include "pdfviewer.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;


private slots:
    void openFile();
    void quit();

private:
    void updateWindowTitle();

private:
    Ui::MainWindow *ui;
    PDFViewer *m_viewer;
};

#endif // MAINWINDOW_H
