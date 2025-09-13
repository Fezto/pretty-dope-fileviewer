#include "pdfdocument.h"
#include <QFileInfo>

PDFDocument::PDFDocument()
    : m_document(nullptr)
{
    // Simple construction; defer actual loading to loadFromFile().
}

PDFDocument::~PDFDocument()
{
    // Release any opened document.
    close();
}

bool PDFDocument::loadFromFile(const QString &filePath)
{
    // Always start clean (idempotent if already empty).
    close();

    // Poppler does the parsing. If it fails we just return false silently.
    m_document = Poppler::Document::load(filePath);
    if (!m_document)
    {
        return false; // Corrupt / missing file / invalid format.
    }

    if (m_document->isLocked())
    {
        // Skip exposing a locked document that would require password UI.
        m_document.reset();
        return false;
    }

    m_filePath = filePath;
    return true;
}

void PDFDocument::close()
{
    // unique_ptr releases Poppler::Document automatically.
    m_document.reset();
    m_filePath.clear();
}

bool PDFDocument::isLoaded() const
{
    return m_document != nullptr;
}

int PDFDocument::pageCount() const
{
    return m_document ? m_document->numPages() : 0;
}

QString PDFDocument::title() const
{
    if (!m_document)
    {
        return QString();
    }

    // Try metadata; fallback to file name.
    QString title = m_document->info("Title");
    if (title.isEmpty())
    {
        title = QFileInfo(m_filePath).fileName();
    }
    return title;
}

QString PDFDocument::filePath() const
{
    return m_filePath;
}

bool PDFDocument::isLocked() const
{
    return m_document ? m_document->isLocked() : false;
}

std::unique_ptr<Poppler::Page> PDFDocument::getPage(int pageIndex) const
{
    if (!m_document || pageIndex < 0 || pageIndex >= pageCount())
    {
        return nullptr; // Out-of-range protection.
    }
    return m_document->page(pageIndex);
}
