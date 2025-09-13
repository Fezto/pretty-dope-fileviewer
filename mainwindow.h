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

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void openFile();
    void quit();

private:
    void updateWindowTitle();
    void setupShortcuts();

private:
    Ui::MainWindow *ui;
    PDFDocument *m_document;
    PDFViewer *m_viewer;
};

#endif // MAINWINDOW_H
