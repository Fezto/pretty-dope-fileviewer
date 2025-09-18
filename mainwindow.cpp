#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QDebug>
#include <QShortcut>
#include <QKeySequence>
#include <QAction>

// Main application window implementation
// Responsibilities: file loading, zoom handling, navigation wiring
// Owns: PDFViewer (which owns PDFDocument once loaded)

// Construction -----------------------------------------------------
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), m_viewer(new PDFViewer())
{
    ui->setupUi(this);

    setCentralWidget(m_viewer);

    // Wire toolbar actions
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::quit);

    // Wire signals
    connect(m_viewer, &PDFViewer::currentPageChanged, this, &MainWindow::updateWindowTitle);
    connect(m_viewer, &PDFViewer::zoomChanged, this, &MainWindow::updateWindowTitle);

}

// Destruction ------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}

// File Loading -----------------------------------------------------
void MainWindow::openFile()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("PDF files (*.pdf)");

    if (!dialog.exec())
    {
        return;
    }

    QStringList selected = dialog.selectedFiles();
    if (selected.isEmpty())
    {
        return;
    }

    QString filePath = selected.first();

    // Load document
    auto doc = std::make_unique<PDFDocument>();
    if (!doc->loadFromFile(filePath))
    {
        // qDebug() << "MainWindow: Failed to load PDF";
        QMessageBox::warning(this, tr("Error"), tr("Failed to open PDF. It may be damaged or password protected?"));
        return;
    }

    // Show in viewer
    if (!m_viewer->setDocument(std::move(doc)))
    {
        qDebug() << "MainWindow: Failed to set document in viewer";
        QMessageBox::warning(this, tr("Error"), tr("Error configuring the PDF viewer."));
        return;
    }

    updateWindowTitle();
}

// Application Control ----------------------------------------------
void MainWindow::quit()
{
    QApplication::quit();
}

// Window Title Update ----------------------------------------------
void MainWindow::updateWindowTitle()
{
    if (!m_viewer->hasDocument())
    {
        setWindowTitle("PrettyDopeFileviewer");
        return;
    }

    QString title = QString("PrettyDopeFileviewer - %1 (%2/%3) - %4%")
                        .arg(m_viewer->document()->title())
                        .arg(m_viewer->currentPage() + 1)
                        .arg(m_viewer->document()->pageCount())
                        .arg(int(m_viewer->zoom() * 100));

    setWindowTitle(title);
}

