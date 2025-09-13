/**
 * PDFPage implementation
 * ---------------------------------------------------------------
 * Encapsulates rendering of a single PDF page. Intentionally lean: receives a
 * Poppler::Page, converts it to QImage/QPixmap and adopts the image size so
 * scrolling inside the viewer feels natural.
 */

#include "pdfpage.h"
#include <QVBoxLayout>
#include <QDebug>

PDFPage::PDFPage(QWidget *parent)
    : QWidget(parent), m_imageLabel(nullptr), m_page(nullptr), m_pageIndex(-1), m_isRendered(false)
{
    // Initialize the visual container and prepare internal state.
    // We start with no page, no image, and a clean slate.
    setupUI();
}

void PDFPage::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    // We don't need margins because we want the page to be shown 'as is'
    layout->setContentsMargins(0, 0, 0, 0);

    // Our PDF page should adopt a fixed size based on rendered content
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_imageLabel = new QLabel();

    // ? m_imageLabel->setAlignment(Qt::AlignCenter);

    // Our label will be by default a simple "white rectangle" which is going to be replaced
    // with the real PDF pages
    m_imageLabel->setStyleSheet("background: white; border: 1px solid lightgray;");

    // If the PDF page is fixed size, so is its label
    m_imageLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    layout->addWidget(m_imageLabel);
}

void PDFPage::setPage(std::unique_ptr<Poppler::Page> page, int pageIndex)
{
    m_page = std::move(page);
    m_pageIndex = pageIndex;
    m_isRendered = false;

    // Show temporary loading text
    m_imageLabel->setText(QString("Cargando página %1...").arg(pageIndex + 1));
}

void PDFPage::render(int dpi)
{
    if (!m_page)
    {
        qDebug() << "PDFPage::render - No page set for index" << m_pageIndex;
        return;
    }

    if (m_isRendered)
    {
        qDebug() << "PDFPage::render - Page" << m_pageIndex << "already rendered, skipping";
        return;
    }

    qDebug() << "PDFPage::render - Rendering page" << m_pageIndex << "at DPI" << dpi;

    // Render page to an QImage
    QImage image = m_page->renderToImage(dpi, dpi);

    if (image.isNull())
    {
        qDebug() << "PDFPage::render - Failed to render page" << m_pageIndex;
        m_imageLabel->setText(QString("Failed to render page %1").arg(m_pageIndex + 1));
        return;
    }

    qDebug() << "PDFPage::render - Successfully rendered page" << m_pageIndex
             << "size:" << image.size();

    // Create an image display (QPixmap) from an image
    QPixmap pixmap = QPixmap::fromImage(image);
    m_imageLabel->setPixmap(pixmap);

    // Adjust the QLabel's size to the QPixmap's one via sizeHint()
    m_imageLabel->adjustSize();

    // Ensure container (PDFPage) adopts image size to avoid thin stripes
    // The PDFPage own size has changed...
    setFixedSize(m_imageLabel->size());

    // So we need to notify it to his parent for (possible) resizing events!
    updateGeometry();

    qDebug() << "PDFPage::render - Final pixmap size:" << pixmap.size()
             << "Label size:" << m_imageLabel->size();

    // Force layout update as in original logic
    // Here we are just really retrieving the same QVBoxLayout defined in PDFPage::setupUI()
    // Remember that Qt recovers the layout as a QObject, so you need to cast it!
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(this->layout());
    if (layout)
    {
        // And of course, we recalculate the layout's dimensions
        layout->activate();
    }

    m_isRendered = true;
}

QSize PDFPage::pageSize() const
{
    if (!m_page)
    {
        // No page loaded, so we can't report any dimensions
        return QSize();
    }

    // Retrieve the logical size of the PDF page in typographic points (1/72 inch).
    // This is not in pixels yet — it's the abstract canvas size.
    QSizeF sizef = m_page->pageSize();

    // Convert to integer-based QSize for layout purposes.
    // We lose precision... but we gain compatibility with Qt's geometry system instead
    return QSize(static_cast<int>(sizef.width()), static_cast<int>(sizef.height()));
}


