#ifndef NAVIGATIONCONTROLLER_H
#define NAVIGATIONCONTROLLER_H

#include <QObject>
#include <QKeyEvent>
#include <QRect>

class PageManager;
class QScrollBar;
class QWidget;

/**
 * NavigationController
 * --------------------------------------------------------
 * Handles logical page navigation within a PDF document.
 *
 * Responsibilities:
 * - Keyboard navigation (arrows, Page Up/Down, Home/End)
 * - Track current page based on scroll position
 * - Programmatic navigation to specific pages
 * - Auto-centering target page after jump
 */
class NavigationController : public QObject
{
    Q_OBJECT

public:
    explicit NavigationController(QObject *parent = nullptr);

    // Context Configuration -----------------------------------------
    void setContext(PageManager *pageManager,
                    QScrollBar *verticalScrollBar,
                    QWidget *viewportWidget);

    void setRenderDPI(int dpi) { m_renderDPI = dpi; }

    // Navigation Operations -----------------------------------------
    void goToPage(int pageIndex);
    void goToNextPage();
    void goToPreviousPage();
    void goToFirstPage();
    void goToLastPage();

    // State ---------------------------------------------------------
    int currentPage() const { return m_currentPage; }
    void updateCurrentPageFromScroll();

    // Event Handling ------------------------------------------------
    bool handleKeyPress(QKeyEvent *event);

signals:
    void currentPageChanged(int pageIndex);
    void requestScrollTo(int value);
    void requestRenderPage(int pageIndex, int dpi);

private:
    int m_currentPage;
    int m_renderDPI;

    // Concrete collaborators (non-owning)
    PageManager *m_pageManager = nullptr;
    QScrollBar *m_scrollBar = nullptr;
    QWidget *m_viewport = nullptr;

    // Defaults
    static constexpr int DEFAULT_DPI = 200;
};

#endif // NAVIGATIONCONTROLLER_H