/**
 * PageManager implementation
 * ---------------------------------------------------------------
 * Handles creation, layout, and visibility-based rendering of page widgets.
 * Keeps geometry up to date for the scroll area.
 */

#include "pagemanager.h"
#include <QDebug>

// Construction & Destruction --------------------------------------
PageManager::PageManager() : m_contentWidget(nullptr), m_contentLayout(nullptr), m_document(nullptr)
{
}

PageManager::~PageManager()
{
    clear();
}

// Build Pages ------------------------------------------------------
// Creates (or recreates) all page widgets for the provided document

void PageManager::buildPages(PDFDocument *document)
{
    if (!document || !document->isLoaded())
    {
        qWarning() << "PageManager: Invalid document provided";
        return;
    }

    // Clear previous state
    clear();
    m_document = document;

    // Create container widget and layout
    createContentWidget();

    // Create page widgets
    int pageCount = m_document->pageCount();
    m_pageWidgets.resize(pageCount);

    for (int i = 0; i < pageCount; ++i)
    {
        addPageWidget(i);
    }

    // Add stretch at end for vertical centering
    m_contentLayout->addStretch(1);

    // Force geometry update
    m_contentWidget->adjustSize();
    m_contentLayout->update();
    m_contentLayout->activate();
}

// Clear State ------------------------------------------------------
// Releases widgets and resets internal pointers

void PageManager::clear()
{
    // QPointers auto-null when the parent widget is deleted
    m_pageWidgets.clear();

    if (m_contentWidget)
    {
        m_contentWidget->deleteLater();
        m_contentWidget = nullptr;
        m_contentLayout = nullptr; // Destroyed with parent widget
    }

    m_document = nullptr;
}

// Page Access ------------------------------------------------------
// Pointer returns nullptr if out of range

PDFPage *PageManager::pageAt(int index) const
{
    if (index < 0 || index >= m_pageWidgets.size())
    {
        return nullptr;
    }
    return m_pageWidgets[index].data();
}

// Initial Pre-render -----------------------------------------------
// Renders the first N pages to improve initial loading experience

void PageManager::preRenderInitialPages(int count, int dpi)
{
    if (!m_document)
        return;

    int pagesToRender = qMin(count, m_pageWidgets.size());

    for (int i = 0; i < pagesToRender; ++i)
    {
        renderPageAt(i, dpi);
    }

    // Update geometry after initial rendering
    updateContentGeometry();
}
// Visible Range Rendering -----------------------------------------
// Renders only pages within the visible scroll window (plus buffer)

void PageManager::renderVisiblePages(int scrollValue, int viewportHeight, int preRenderBuffer, int dpi)
{
    if (!m_document || m_pageWidgets.isEmpty())
        return;

    // Compute visible range using average page height heuristic
    int firstVisible = qMax(0, (scrollValue / AVG_PAGE_HEIGHT) - preRenderBuffer);
    int lastVisible = qMin(m_pageWidgets.size() - 1, ((scrollValue + viewportHeight) / AVG_PAGE_HEIGHT) + preRenderBuffer);

    // Render pages in visible range
    for (int i = firstVisible; i <= lastVisible; ++i)
    {
        renderPageAt(i, dpi);
    }

    // Update geometry after rendering
    updateContentGeometry();
}
// Geometry Update --------------------------------------------------
// Recomputes container min size based on rendered pages

void PageManager::updateContentGeometry()
{
    if (!m_contentWidget || m_pageWidgets.isEmpty())
        return;

    int maxWidth = 0;
    int totalHeight = 0;

    // Find maximum width & accumulate total heights
    for (const QPointer<PDFPage> &pagePointer : m_pageWidgets)
    {
        PDFPage *page = pagePointer.data();
        if (!page)
            continue;

        maxWidth = qMax(maxWidth, page->width());
        totalHeight += page->height();
    }

    // Add spacing and margins
    if (m_contentLayout)
    {
        int spacing = m_contentLayout->spacing();
        totalHeight += spacing * qMax(0, m_pageWidgets.size() - 1);

        // Add margins
        QMargins margins = m_contentLayout->contentsMargins();
        maxWidth += margins.left() + margins.right();
        totalHeight += margins.top() + margins.bottom();
    }

    // Set minimum size (+1 to avoid zero-dimension edge cases)
    if (maxWidth > 0 && totalHeight > 0)
    {
        m_contentWidget->setMinimumSize(maxWidth, totalHeight + 1);
        m_contentWidget->updateGeometry();
    }
}
// Container Creation -----------------------------------------------
// Allocates content widget & vertical layout

void PageManager::createContentWidget()
{
    m_contentWidget = new QWidget();
    m_contentLayout = new QVBoxLayout(m_contentWidget);

    // Default layout configuration
    m_contentLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    m_contentLayout->setSpacing(DEFAULT_SPACING);
    m_contentLayout->setContentsMargins(DEFAULT_MARGINS, DEFAULT_MARGINS, DEFAULT_MARGINS, DEFAULT_MARGINS);
}
// Single Page Addition ---------------------------------------------
// Builds a single PDFPage widget and inserts it

void PageManager::addPageWidget(int pageIndex)
{
    if (!m_document || !m_contentWidget)
        return;

    // Create page widget
    PDFPage *pageWidget = new PDFPage(m_contentWidget);

    // Retrieve and assign document page
    auto page = m_document->getPage(pageIndex);
    pageWidget->setPage(std::move(page), pageIndex);

    // Add to layout and store pointer
    m_contentLayout->addWidget(pageWidget);
    m_pageWidgets[pageIndex] = pageWidget;
}

// Convenience Methods --------------------------------------

void PageManager::renderPageAt(int index, int dpi)
{
    PDFPage *page = pageAt(index);

    if (page)
    {
        page->render(dpi);
    }
}

void PageManager::setLayoutSpacing(int spacing)
{
    if (m_contentLayout)
    {
        m_contentLayout->setSpacing(spacing);
    }
}

void PageManager::setLayoutMargins(int left, int top, int right, int bottom)
{
    if (m_contentLayout)
    {
        m_contentLayout->setContentsMargins(left, top, right, bottom);
    }
}
