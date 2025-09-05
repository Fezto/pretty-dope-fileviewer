#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QStringList>
#include <QPdfDocument>
#include <QPdfView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);

    QStringList mimeTypeFilters({"application/pdf"});
    dialog.setMimeTypeFilters(mimeTypeFilters);

    if(!dialog.exec()) {
        return;
    }

    QStringList selectedFile = dialog.selectedFiles();

    if(selectedFile.isEmpty()){
        return;
    }

    QString filePath = selectedFile.first();

    QPdfDocument *pdf = new QPdfDocument(this);

    pdf->load(filePath);

    QPdfView *pdfViewer = new QPdfView(this);
    pdfViewer->setDocument(pdf);
    pdfViewer->setPageMode(QPdfView::PageMode::MultiPage);

    setCentralWidget(pdfViewer);
}

