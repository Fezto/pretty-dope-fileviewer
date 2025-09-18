#ifndef PDFVIEWER_H
#define PDFVIEWER_H

#include <QScrollArea>
#include <memory>
#include "pdfdocument.h"
#include "pagemanager.h"
#include "zoomcontroller.h"
#include "navigationcontroller.h"

/**
 * PDFViewer
 * ---------------------------------------------------------------
 * Central orchestrator that coordinates specialized components to
 * display PDF documents efficiently and with clean separation of concerns.
 *
 * Component architecture:
 *  - PageManager: Creates, owns and schedules page widgets (lazy rendering)
 *  - ZoomController: Maintains zoom state and auto-fit calculations
 *  - NavigationController: Keyboard/page navigation and current page tracking
 *  - PDFViewer: Wires everything together and handles UI events (scroll, resize, keys)
 */
class PDFViewer : public QScrollArea
{
    Q_OBJECT

public:
    explicit PDFViewer(QWidget *parent = nullptr);

    // Document ------------------------------------------------------
    bool setDocument(std::unique_ptr<PDFDocument> document);
    void clearDocument();
    PDFDocument *document() const { return m_document.get(); }
    bool hasDocument() const { return m_document && m_document->isLoaded(); }

    // Navigation ----------------------------------------------------
    void goToPage(int pageIndex);
    int currentPage() const;

    // Zoom ----------------------------------------------------------
    void setZoom(double factor);
    double zoom() const;
    void zoomIn() { setZoom(zoom() * 1.1); }
    void zoomOut() { setZoom(zoom() / 1.1); }
    void zoomReset() { setZoom(1.0); }

    void zoomFitWidth();
    void zoomFitPage();
    bool isFitWidth() const;
    bool isFitPage() const;

    // Utilities -----------------------------------------------------
    QString extractAllText() const;

signals:
    void currentPageChanged(int pageIndex);
    void zoomChanged(double factor);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void renderVisiblePages();

private:
    void setupUI();
    void setupZoomController();

    // Geometry helper used by navigation logic
    QRect getPageGeometry(int pageIndex) const;

    void renderPageAt(int i, int dpi);
    void moveScrollBarTo(int i);

    // Helpers passed to ZoomController for auto-fit calculations
    ViewportInfo getViewportInfo() const;
    PageInfo getPageInfo() const;

    // Core collaborating components (owned or aggregated)
    std::unique_ptr<PDFDocument> m_document;
    PageManager *m_pageManager;
    ZoomController *m_zoomController;
    NavigationController *m_navigationController;

    // Config constants
    static constexpr int DEFAULT_DPI = 200;
    static constexpr int PRERENDER_PAGES = 2;
    static constexpr double MIN_ZOOM = 0.5;
    static constexpr double MAX_ZOOM = 10.0;
};

#endif // PDFVIEWER_H
