#ifndef PDFPAGE_H
#define PDFPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QString>
#include <memory>
#include <poppler-qt6.h>

/**
 * PDFPage
 * ---------------------------------------------------------------
 * Visual representation of ONE PDF page.
 *
 *  - Owns a QLabel where the rendered QPixmap is placed.
 *  - Lazy rendering: only happens when render() is called.
 *  - After rendering we set isRendered() to avoid duplicate work.
 *  - Can be invalidated by calling setPage() again (e.g. after zoom).
 *
 * Design notes:
 *  - Owns Poppler::Page via unique_ptr.
 *  - Does not mix scroll or global zoom logic: that lives in PDFViewer!
 *  - Kept intentionally small for easy isolated rendering and self-health
 */
class PDFPage : public QWidget
{
    Q_OBJECT

public:
    explicit PDFPage(QWidget *parent = nullptr);

    // Assign underlying page data (resets render state).
    void setPage(std::unique_ptr<Poppler::Page> page, int pageIndex);
    void render(int dpi = 150); // No-op if already rendered.

    // Quick metadata.
    int pageIndex() const { return m_pageIndex; }
    bool isRendered() const { return m_isRendered; }
    QSize pageSize() const; // Logical (pt) size from Poppler.

private:
    QLabel *m_imageLabel;                  // Presentation surface.
    std::unique_ptr<Poppler::Page> m_page; // Underlying page data.
    int m_pageIndex;                       // Index inside document.
    bool m_isRendered;                     // Render cache flag.
    int m_lastDpi = -1;                    // Last DPI used to render (for zoom re-render)

    void setupUI(); // Initialize layout & styling.
};

#endif // PDFPAGE_H
