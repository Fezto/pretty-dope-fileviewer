#ifndef PAGEMANAGER_H
#define PAGEMANAGER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QVector>
#include <QPointer>
#include "pdfpage.h"
#include "pdfdocument.h"

/**
 * PageManager
 * --------------------------------------------------------
 * Manages lifecycle and rendering of PDF page widgets.
 *
 * Responsibilities:
 * - Create and arrange page widgets
 * - Visibility-aware (lazy) rendering strategy
 * - Maintain overall content geometry
 * - Pre-render an initial window of pages for fast first paint
 */
class PageManager
{
public:
    PageManager();
    ~PageManager();

    // Document Lifecycle --------------------------------------------
    void buildPages(PDFDocument *document);
    void clear();

    // Component Access ----------------------------------------------
    QWidget *contentWidget() const { return m_contentWidget; }
    QVBoxLayout *contentLayout() const { return m_contentLayout; }

    // Page Information ----------------------------------------------
    int pageCount() const { return m_pageWidgets.size(); }
    PDFPage *pageAt(int index) const;
    bool isEmpty() const { return m_pageWidgets.isEmpty(); }

    // Rendering Operations ------------------------------------------
    void preRenderInitialPages(int count, int dpi);
    void renderVisiblePages(int scrollValue, int viewportHeight, int preRenderBuffer, int dpi);

    // Geometry Maintenance ------------------------------------------
    void updateContentGeometry();

    // Layout Configuration ------------------------------------------
    void setLayoutSpacing(int spacing);
    void setLayoutMargins(int left, int top, int right, int bottom);

    void renderPageAt(int index, int dpi);

private:
    void createContentWidget();
    void addPageWidget(int pageIndex);

    // Layout defaults
    static constexpr int DEFAULT_SPACING = 20;
    static constexpr int DEFAULT_MARGINS = 50;
    static constexpr int AVG_PAGE_HEIGHT = 600; // Heuristic for visibility calculations

    QWidget *m_contentWidget;
    QVBoxLayout *m_contentLayout;
    QVector<QPointer<PDFPage>> m_pageWidgets;
    PDFDocument *m_document;
};

#endif // PAGEMANAGER_H
