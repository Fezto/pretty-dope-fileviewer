#ifndef ZOOMCONTROLLER_H
#define ZOOMCONTROLLER_H

#include <QObject>

/**
 * Viewport information required for zoom calculations.
 * Plain data object with only the fields we need.
 */
struct ViewportInfo
{
    int width = 0;
    int height = 0;
    int marginsH = 0; // horizontal margins (left + right)
    int marginsV = 0; // vertical margins (top + bottom)
};

/**
 * Page reference dimensions used for auto-fit calculations.
 * Usually taken from the first (already rendered) page.
 */
struct PageInfo
{
    int width = 0;
    int height = 0;
};

/**
 * Zoom modes supported by the viewer.
 */
enum class ZoomMode
{
    Free,     // Manual / user-controlled zoom
    FitWidth, // Fit page width inside the viewport
    FitPage   // Fit entire page (width & height) inside the viewport
};

/**
 * ZoomController – concise academic version
 * ================================================================
 * Responsibilities:
 * 1. Hold current zoom factor + mode
 * 2. Derive zoom factors for auto-fit behaviors
 * 3. Enforce min/max zoom boundaries
 * 4. Emit unified change signal
 *
 * Explicitly does NOT:
 * - Render pages
 * - Touch widget layout
 * - Expose complex callback wiring
 *
 * Educational benefits:
 * - Linear, readable code
 * - Single, well-defined responsibility
 * - Uses idiomatic Qt signals/slots
 * - Easy to unit test in isolation
 */
class ZoomController : public QObject
{
    Q_OBJECT

public:
    explicit ZoomController(QObject *parent = nullptr);
    ~ZoomController() = default;

    // Configuration -------------------------------------------------
    void setLimits(double minZoom, double maxZoom);

    // Direct Zoom Control -------------------------------------------
    void setZoom(double factor);
    void zoomIn() { setZoom(currentZoom() * 1.2); }
    void zoomOut() { setZoom(currentZoom() / 1.2); }
    void resetZoom() { setZoom(1.0); }

    // Auto-fit Actions ----------------------------------------------
    void fitToWidth(const ViewportInfo &viewport, const PageInfo &page);
    void fitToPage(const ViewportInfo &viewport, const PageInfo &page);

    // State Inspection ----------------------------------------------
    double currentZoom() const { return m_currentZoom; }
    ZoomMode currentMode() const { return m_currentMode; }
    bool isFitWidth() const { return m_currentMode == ZoomMode::FitWidth; }
    bool isFitPage() const { return m_currentMode == ZoomMode::FitPage; }

    // Helper Computations -------------------------------------------
    double calculateFitWidth(const ViewportInfo &viewport, const PageInfo &page) const;
    double calculateFitPage(const ViewportInfo &viewport, const PageInfo &page) const;

    // Event Hooks ---------------------------------------------------
    void onViewportResize(const ViewportInfo &viewport, const PageInfo &page);

signals:
    /**
     * Emitted whenever zoom changes.
     * @param newZoom Current zoom factor
     * @param mode Current zoom mode
     */
    void zoomChanged(double newZoom, ZoomMode mode);

private:
    double clampZoom(double zoom) const;
    void applyZoom(double newZoom, ZoomMode newMode);

private:
    double m_currentZoom;
    ZoomMode m_currentMode;
    double m_minZoom;
    double m_maxZoom;

    // Constants
    static constexpr double DEFAULT_ZOOM = 1.0;
    static constexpr double MIN_ZOOM_DEFAULT = 0.25;
    static constexpr double MAX_ZOOM_DEFAULT = 10.0;
    static constexpr double ZOOM_STEP = 1.2;
};

#endif // ZOOMCONTROLLER_H
