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

// Main application window implementation.
// Responsible for file loading, zoom handling and navigation.
// Holds a PDFDocument (data) and a PDFViewer (presentation).

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_document(new PDFDocument()),
    m_viewer(new PDFViewer())
{
    ui->setupUi(this);

    // Set viewer as central widget
    setCentralWidget(m_viewer);

    // Wire toolbar actions
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::quit);

    // Wire signals
    connect(m_viewer, &PDFViewer::currentPageChanged, this, &MainWindow::updateWindowTitle);
    connect(m_viewer, &PDFViewer::zoomChanged, this, &MainWindow::updateWindowTitle);

    setupShortcuts();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_document;
}

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
    qDebug() << "MainWindow: Loading PDF from" << filePath;

    if (!m_document->loadFromFile(filePath))
    {
        qDebug() << "MainWindow: Failed to load PDF";
        QMessageBox::warning(this, tr("Error"), tr("Failed to open PDF. It may be damaged or password protected?"));
        return;
    }

    qDebug() << "MainWindow: PDF loaded successfully, pages:" << m_document->pageCount();

    // Show in viewer
    if (!m_viewer->setDocument(m_document))
    {
        qDebug() << "MainWindow: Failed to set document in viewer";
        QMessageBox::warning(this, tr("Error"), tr("Error configuring the PDF viewer."));
        return;
    }

    updateWindowTitle();
}

void MainWindow::quit()
{
    QApplication::quit();
}

void MainWindow::updateWindowTitle()
{
    if (!m_document->isLoaded())
    {
        setWindowTitle("PrettyDopeFileviewer");
        return;
    }

    QString title = QString("PrettyDopeFileviewer - %1 (%2/%3) - %4%")
                        .arg(m_document->title())
                        .arg(m_viewer->currentPage() + 1)
                        .arg(m_document->pageCount())
                        .arg(int(m_viewer->zoom() * 100));

    setWindowTitle(title);
}

void MainWindow::setupShortcuts()
{
    // Zoom in (Ctrl + '+' or Ctrl + '=')
    auto *zoomInShortcut = new QShortcut(QKeySequence::ZoomIn, this);
    connect(zoomInShortcut, &QShortcut::activated, this, [this]()
            {
                if (m_document->isLoaded()) m_viewer->zoomIn(); });

    // Zoom out (Ctrl + '-')
    auto *zoomOutShortcut = new QShortcut(QKeySequence::ZoomOut, this);
    connect(zoomOutShortcut, &QShortcut::activated, this, [this]()
            {
                if (m_document->isLoaded()) m_viewer->zoomOut(); });

    // Reset zoom (Ctrl+0)
    auto *zoomResetShortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+0")), this);
    connect(zoomResetShortcut, &QShortcut::activated, this, [this]()
            {
                if (m_document->isLoaded()) m_viewer->setZoom(1.0); });

    // Fit width (Ctrl+Shift+W)
    auto *fitWidthShortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+W")), this);
    connect(fitWidthShortcut, &QShortcut::activated, this, [this]()
            {
                if (m_document->isLoaded()) m_viewer->zoomFitWidth(); });

    // Fit page (Ctrl+Shift+P)
    auto *fitPageShortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+P")), this);
    connect(fitPageShortcut, &QShortcut::activated, this, [this]()
            {
                if (m_document->isLoaded()) m_viewer->zoomFitPage(); });
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (!m_document->isLoaded())
    {
        QMainWindow::keyPressEvent(event);
        return;
    }

    switch (event->key())
    {
    case Qt::Key_Right:
    case Qt::Key_Down:
    case Qt::Key_PageDown:
        m_viewer->goToPage(m_viewer->currentPage() + 1);
        break;

    case Qt::Key_Left:
    case Qt::Key_Up:
    case Qt::Key_PageUp:
        m_viewer->goToPage(m_viewer->currentPage() - 1);
        break;

    default:
        QMainWindow::keyPressEvent(event);
        break;
    }
}
