/**
 * PDFViewer implementation – Orchestrator pattern
 * ---------------------------------------------------------------
 * Coordinates specialized components:
 * - PageManager: page widget lifecycle & lazy rendering
 * - ZoomController: zoom state + auto-fit logic
 * - NavigationController: keyboard navigation & current page tracking
 */

#include "pdfviewer.h"
#include <QKeyEvent>
#include <QScrollBar>
#include <QResizeEvent>
#include <QShortcut>
#include <QKeySequence>

PDFViewer::PDFViewer(QWidget *parent) : QScrollArea(parent), m_pageManager(nullptr), m_zoomController(nullptr), m_navigationController(nullptr)
{
    setupUI();
}

void PDFViewer::setupUI()
{
    // Basic QScrollArea configuration
    setBackgroundRole(QPalette::Dark);
    setFocusPolicy(Qt::StrongFocus);

    // Create collaborating components
    m_pageManager = new PageManager();
    m_zoomController = new ZoomController();
    m_navigationController = new NavigationController(this);

    // Wire zoom + navigation related signals
    setupZoomController();

    // Provide runtime context to NavigationController
    m_navigationController->setContext(m_pageManager, verticalScrollBar(), viewport());

    // Connect navigation action signals
    connect(m_navigationController, &NavigationController::requestScrollTo, this, &PDFViewer::moveScrollBarTo);
    connect(m_navigationController, &NavigationController::requestRenderPage, this, &PDFViewer::renderPageAt);

    // React to vertical scroll changes
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &PDFViewer::renderVisiblePages);

    // Propagate navigation events outward
    connect(m_navigationController, &NavigationController::currentPageChanged, this, &PDFViewer::currentPageChanged);
}

bool PDFViewer::setDocument(std::unique_ptr<PDFDocument> document)
{
    if (!document || !document->isLoaded())
    {
        return false;
    }

    clearDocument();
    m_document = std::move(document);

    // Build page widgets via PageManager
    m_pageManager->buildPages(m_document.get());
    if (m_pageManager->contentWidget())
    {
        setWidget(m_pageManager->contentWidget());
    }

    // Pre-render first N pages at initial DPI
    int initialDPI = int(DEFAULT_DPI * m_zoomController->currentZoom());
    m_pageManager->preRenderInitialPages(5, initialDPI);

    // Pass current DPI to navigation (for targeted prerendering)
    m_navigationController->setRenderDPI(initialDPI);

    // Jump to the first page
    m_navigationController->goToFirstPage();

    return true;
}

void PDFViewer::clearDocument()
{
    if (QWidget *w = takeWidget())
    {
        w->deleteLater();
    }

    if (m_pageManager)
    {
        m_pageManager->clear();
    }

    m_document.reset();
}

// Public Convenience Methods -------------------------------------

void PDFViewer::goToPage(int pageIndex)
{
    if (m_navigationController)
    {
        m_navigationController->goToPage(pageIndex);
    }
}

int PDFViewer::currentPage() const
{
    return m_navigationController ? m_navigationController->currentPage() : 0;
}

void PDFViewer::setZoom(double factor)
{
    if (m_zoomController)
    {
        m_zoomController->setZoom(factor);
    }
}

double PDFViewer::zoom() const
{
    return m_zoomController ? m_zoomController->currentZoom() : 1.0;
}

void PDFViewer::zoomFitWidth()
{
    if (m_zoomController)
    {
        ViewportInfo viewport = getViewportInfo();
        PageInfo page = getPageInfo();
        m_zoomController->fitToWidth(viewport, page);
    }
}

void PDFViewer::zoomFitPage()
{
    if (m_zoomController)
    {
        ViewportInfo viewport = getViewportInfo();
        PageInfo page = getPageInfo();
        m_zoomController->fitToPage(viewport, page);
    }
}

bool PDFViewer::isFitWidth() const
{
    return m_zoomController && m_zoomController->currentMode() == ZoomMode::FitWidth;
}

bool PDFViewer::isFitPage() const
{
    return m_zoomController && m_zoomController->currentMode() == ZoomMode::FitPage;
}

// Event Overrides -------------------------------------------------

void PDFViewer::keyPressEvent(QKeyEvent *event)
{
    if (!m_document || !m_document->isLoaded())
    {
        QScrollArea::keyPressEvent(event);
        return;
    }

    // Handle zoom shortcuts first (Ctrl + =/+/-/0) including layouts where '+' needs Shift
    if (event->modifiers() & Qt::ControlModifier)
    {
        const int k = event->key();
        if (k == Qt::Key_Plus || k == Qt::Key_Equal)
        {
            if (m_zoomController)
                m_zoomController->zoomIn();
            event->accept();
            return;
        }
        if (k == Qt::Key_Minus || k == Qt::Key_Underscore)
        {
            if (m_zoomController)
                m_zoomController->zoomOut();
            event->accept();
            return;
        }
        if (k == Qt::Key_0)
        {
            if (m_zoomController)
                m_zoomController->resetZoom();
            event->accept();
            return;
        }
    }

    // Delegate navigation keys to NavigationController
    if (m_navigationController && m_navigationController->handleKeyPress(event))
    {
        return; // Evento manejado
    }

    // Fallback: let base class handle
    QScrollArea::keyPressEvent(event);
}

void PDFViewer::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);

    // Notify ZoomController so auto-fit modes can recalculate
    if (m_zoomController)
    {
        ViewportInfo viewport = getViewportInfo();
        PageInfo page = getPageInfo();
        m_zoomController->onViewportResize(viewport, page);
    }
}

void PDFViewer::renderVisiblePages()
{
    // Render visible + buffered pages lazily
    if (m_pageManager && m_document)
    {
        int scrollValue = verticalScrollBar()->value();
        int viewportHeight = viewport()->height();
        int dpi = int(DEFAULT_DPI * zoom());

        m_pageManager->renderVisiblePages(scrollValue, viewportHeight,
                                          PRERENDER_PAGES, dpi);
    }

    // Update current page based on scroll position
    if (m_navigationController)
    {
        m_navigationController->updateCurrentPageFromScroll();
    }
}

// Private Helpers -------------------------------------------------

void PDFViewer::setupZoomController()
{
    // Setup ZoomController connections
    if (m_zoomController)
    {
        m_zoomController->setLimits(MIN_ZOOM, MAX_ZOOM);

        // On zoom change: update DPI & rerender visible pages
        connect(m_zoomController, &ZoomController::zoomChanged, this, [this](double factor, ZoomMode mode)
                {
                // Actualizar DPI de navegación
                if (m_navigationController)
                {
                    m_navigationController->setRenderDPI(int(DEFAULT_DPI * factor));
                }

                // Re-renderizar páginas visibles con nuevo DPI
                if (m_pageManager && m_document)
                {
                    int scrollValue = verticalScrollBar()->value();
                    int viewportHeight = viewport()->height();
                    int dpi = int(DEFAULT_DPI * factor);
                    m_pageManager->renderVisiblePages(scrollValue, viewportHeight, PRERENDER_PAGES, dpi);
            }
            
            emit zoomChanged(factor); });
    }
}

// Convenience methods -----------------------

void PDFViewer::moveScrollBarTo(int value){
    verticalScrollBar()->setValue(value);
}


void PDFViewer::renderPageAt(int i, int dpi){
    m_pageManager->renderPageAt(i, dpi);
}

QRect PDFViewer::getPageGeometry(int pageIndex) const
{
    if (!m_pageManager)
        return QRect();

    PDFPage *page = m_pageManager->pageAt(pageIndex);
    if (!page)
        return QRect();

    return page->geometry();
}

ViewportInfo PDFViewer::getViewportInfo() const
{
    ViewportInfo info;
    info.width = viewport()->width();
    info.height = viewport()->height();

    if (m_pageManager && m_pageManager->contentLayout())
    {
        QMargins margins = m_pageManager->contentLayout()->contentsMargins();
        info.marginsH = margins.left() + margins.right();
        info.marginsV = margins.top() + margins.bottom();
    }

    return info;
}

PageInfo PDFViewer::getPageInfo() const
{
    PageInfo info;

    if (m_pageManager && m_pageManager->pageCount() > 0)
    {
        PDFPage *firstPage = m_pageManager->pageAt(0);
        if (firstPage && firstPage->isRendered())
        {
            info.width = firstPage->width();
            info.height = firstPage->height();
        }
    }

    return info;
}
