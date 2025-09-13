#ifndef PDFVIEWER_H
#define PDFVIEWER_H

#include <QScrollArea>
#include <QWidget>
#include <QVBoxLayout>
#include <QVector>
#include <QPointer>
#include <memory>
#include "pdfdocument.h"
#include "pdfpage.h"

/**
 * PDFViewer
 * ---------------------------------------------------------------
 * Orchestrates presentation of multiple PDFPage widgets inside a QScrollArea.
 *
 * Key features:
 *  - Lazy rendering: only paints visible pages (+ a buffer) to reduce cost.
 *  - Zoom: triggers re-render with DPI scaled by zoom factor.
 *  - Tracks current page based on scroll position.
 *  - Aggregated text extraction.
 */
class PDFViewer : public QScrollArea
{
    Q_OBJECT

public:
    explicit PDFViewer(QWidget *parent = nullptr);

    // Document -------------------------------------------------------
    bool setDocument(PDFDocument *document);
    void clearDocument();

    // Navigation -----------------------------------------------------
    void goToPage(int pageIndex);
    int currentPage() const { return m_currentPage; }

    // Zoom -----------------------------------------------------------
    void setZoom(double factor);
    double zoom() const { return m_zoomFactor; }
    void zoomIn() { setZoom(m_zoomFactor * 1.1); }
    void zoomOut() { setZoom(m_zoomFactor / 1.1); }
    void zoomReset() { setZoom(1.0); }

    void zoomFitWidth();
    void zoomFitPage();
    bool isFitWidth() const { return m_zoomMode == ZoomMode::FitWidth; }
    bool isFitPage() const { return m_zoomMode == ZoomMode::FitPage; }

    // Utilities ------------------------------------------------------
    QString extractAllText() const;

signals:
    void currentPageChanged(int pageIndex);
    void zoomChanged(double factor);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onScrollValueChanged();

private:
    enum class ZoomMode { Free, FitWidth, FitPage };

    void setupUI();
    void buildPageWidgets();
    void renderVisiblePages();
    void updateCurrentPage();

    void applyZoom(double factor, bool preserveAnchor);
    void updateAutoFitZoom();
    double computeFitWidthFactor() const;
    double computeFitPageFactor() const;
    void preserveScrollAnchorPreZoom(int &anchorPage, double &relOffset) const;
    void restoreScrollAnchorPostZoom(int anchorPage, double relOffset);

    // FUNCIONES TOCADAS POR AYRTON
    void updateContentGeometry();

    // PROPERTIES

    QWidget *m_contentWidget{nullptr};
    QVBoxLayout *m_contentLayout{nullptr};

    std::unique_ptr<PDFDocument> m_document;
    QVector<QPointer<PDFPage>> m_pageWidgets;

    int m_currentPage{0};
    double m_zoomFactor{1.0};
    ZoomMode m_zoomMode{ZoomMode::Free};

    static constexpr int DEFAULT_DPI = 200;
    static constexpr int PRERENDER_PAGES = 2;
    static constexpr double MIN_ZOOM = 0.5;
    static constexpr double MAX_ZOOM = 10.0;
};

#endif // PDFVIEWER_H
