#include "navigationcontroller.h"
#include "pagemanager.h"
#include "pdfpage.h"
#include <QDebug>
#include <QtMath>
#include <QScrollBar>
#include <QWidget>

// Construction -----------------------------------------------------

NavigationController::NavigationController(QObject *parent) : QObject(parent), m_currentPage(0), m_renderDPI(DEFAULT_DPI)
{
}

// Context Setup ----------------------------------------------------

void NavigationController::setContext(PageManager *pageManager, QScrollBar *verticalScrollBar, QWidget *viewportWidget)
{
    m_pageManager = pageManager;
    m_scrollBar = verticalScrollBar;
    m_viewport = viewportWidget;
}

// Page Jump --------------------------------------------------------

void NavigationController::goToPage(int pageIndex)
{
    if (!m_pageManager || !m_scrollBar || !m_viewport)
    {
        qWarning() << "NavigationController: Context not set";
        return;
    }

    int pageCount = m_pageManager->pageCount();

    if (pageIndex < 0 || pageIndex >= pageCount)
    {
        return;
    }

    // Update current page index
    m_currentPage = pageIndex;

    // Ask for pre-render of target page (if needed)
    emit requestRenderPage(pageIndex, m_renderDPI);

    // Fetch target page geometry
    QRect pageGeometry;

    if (PDFPage *p = m_pageManager->pageAt(pageIndex))
    {
        pageGeometry = p->geometry();
    }

    if (pageGeometry.isNull())
    {
        return;
    }

    // Compute scroll position to vertically center the page
    int viewportHeight = m_viewport ? m_viewport->height() : 500;
    int pageTop = pageGeometry.y();
    int pageHeight = pageGeometry.height();

    // Center the page inside the viewport
    int centerPos = pageTop - (viewportHeight - pageHeight) / 2;
    int targetScroll = qMax(0, centerPos);

    // Request scroll movement
    emit requestScrollTo(targetScroll);

    // Notify observers of page change
    emit currentPageChanged(m_currentPage);
}

// Scroll-Based Page Tracking --------------------------------------

void NavigationController::updateCurrentPageFromScroll()
{
    if (!m_scrollBar || !m_viewport || !m_pageManager)
    {
        return;
    }

    int pageCount = m_pageManager->pageCount();
    if (pageCount == 0)
        return;

    int scrollValue = m_scrollBar->value();
    int viewportHeight = m_viewport->height();
    int viewportCenter = scrollValue + viewportHeight / 2;

    int newCurrentPage = 0;

    // Find page containing the vertical center of the viewport
    for (int i = 0; i < pageCount; ++i)
    {
        QRect pageGeometry;
        if (PDFPage *p = m_pageManager->pageAt(i))
        {
            pageGeometry = p->geometry();
        }

        if (pageGeometry.isNull())
        {
            continue;
        }

        int pageTop = pageGeometry.y();
        int pageBottom = pageTop + pageGeometry.height();

        if (viewportCenter >= pageTop && viewportCenter <= pageBottom)
        {
            newCurrentPage = i;
            break;
        }
    }

    // Commit change only if page actually changed
    if (newCurrentPage != m_currentPage)
    {
        m_currentPage = newCurrentPage;
        emit currentPageChanged(m_currentPage);
    }
}

// Keyboard Handling ------------------------------------------------

bool NavigationController::handleKeyPress(QKeyEvent *event)
{
    if (!event)
        return false;

    switch (event->key())
    {
    case Qt::Key_Right:
    case Qt::Key_Down:
    case Qt::Key_PageDown:
        goToNextPage();
        event->accept();
        return true;

    case Qt::Key_Left:
    case Qt::Key_Up:
    case Qt::Key_PageUp:
        goToPreviousPage();
        event->accept();
        return true;

    case Qt::Key_Home:
        goToFirstPage();
        event->accept();
        return true;

    case Qt::Key_End:
        goToLastPage();
        event->accept();
        return true;

    default:
        return false; // Key not handled here
    }
}

// Convenience Navigation Methods ----------------------------------

void NavigationController::goToNextPage()
{
    if (!m_pageManager)
        return;

    int pageCount = m_pageManager->pageCount();
    if (m_currentPage < pageCount - 1)
    {
        goToPage(m_currentPage + 1);
    }
}

void NavigationController::goToPreviousPage()
{
    if (m_currentPage > 0)
    {
        goToPage(m_currentPage - 1);
    }
}

void NavigationController::goToFirstPage()
{
    goToPage(0);
}

void NavigationController::goToLastPage()
{
    if (!m_pageManager)
        return;

    int pageCount = m_pageManager->pageCount();
    if (pageCount > 0)
    {
        goToPage(pageCount - 1);
    }
}
