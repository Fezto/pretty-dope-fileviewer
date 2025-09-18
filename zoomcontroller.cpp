#include "zoomcontroller.h"
#include <algorithm>
#include <QtMath>

ZoomController::ZoomController(QObject *parent)
    : QObject(parent), m_currentZoom(DEFAULT_ZOOM), m_currentMode(ZoomMode::Free), m_minZoom(MIN_ZOOM_DEFAULT), m_maxZoom(MAX_ZOOM_DEFAULT)
{
}

void ZoomController::setLimits(double minZoom, double maxZoom)
{
    m_minZoom = minZoom;
    m_maxZoom = maxZoom;

    // Re-apply limits to current zoom if now out of range
    double clampedZoom = clampZoom(m_currentZoom);
    if (clampedZoom != m_currentZoom)
    {
        applyZoom(clampedZoom, m_currentMode);
    }
}

void ZoomController::setZoom(double factor)
{
    applyZoom(factor, ZoomMode::Free);
}

void ZoomController::fitToWidth(const ViewportInfo &viewport, const PageInfo &page)
{
    double fitZoom = calculateFitWidth(viewport, page);
    applyZoom(fitZoom, ZoomMode::FitWidth);
}

void ZoomController::fitToPage(const ViewportInfo &viewport, const PageInfo &page)
{
    double fitZoom = calculateFitPage(viewport, page);
    applyZoom(fitZoom, ZoomMode::FitPage);
}

double ZoomController::calculateFitWidth(const ViewportInfo &viewport, const PageInfo &page) const
{
    // Input validation
    if (page.width <= 0)
    {
        return m_currentZoom;
    }

    // Compute available width inside viewport margins
    double availableWidth = viewport.width - viewport.marginsH;
    if (availableWidth <= 0)
    {
        return m_currentZoom;
    }

    // Derive zoom factor that fits width
    return availableWidth / static_cast<double>(page.width);
}

double ZoomController::calculateFitPage(const ViewportInfo &viewport, const PageInfo &page) const
{
    // Input validation
    if (page.width <= 0 || page.height <= 0)
    {
        return m_currentZoom;
    }

    // Compute available width & height
    double availableWidth = viewport.width - viewport.marginsH;
    double availableHeight = viewport.height - viewport.marginsV;

    if (availableWidth <= 0 || availableHeight <= 0)
    {
        return m_currentZoom;
    }

    // Ratios for each dimension
    double widthRatio = availableWidth / static_cast<double>(page.width);
    double heightRatio = availableHeight / static_cast<double>(page.height);

    // Use the most restrictive ratio so entire page is visible
    return std::min(widthRatio, heightRatio);
}

void ZoomController::onViewportResize(const ViewportInfo &viewport, const PageInfo &page)
{
    // Only recalc if in an auto-fit mode
    switch (m_currentMode)
    {
    case ZoomMode::FitWidth:
        fitToWidth(viewport, page);
        break;
    case ZoomMode::FitPage:
        fitToPage(viewport, page);
        break;
    case ZoomMode::Free:
        // Manual mode: do nothing
        break;
    }
}

// Private Helpers -------------------------------------------------

double ZoomController::clampZoom(double zoom) const
{
    return std::clamp(zoom, m_minZoom, m_maxZoom);
}

void ZoomController::applyZoom(double newZoom, ZoomMode newMode)
{
    double clampedZoom = clampZoom(newZoom);

    // Emit signal only if something actually changed
    if (clampedZoom != m_currentZoom || newMode != m_currentMode)
    {
        m_currentZoom = clampedZoom;
        m_currentMode = newMode;
        emit zoomChanged(m_currentZoom, m_currentMode);
    }
}
