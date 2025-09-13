#ifndef PDFDOCUMENT_H
#define PDFDOCUMENT_H

#include <QString>
#include <memory>
#include <poppler-qt6.h>

/**
 * PDFDocument
 * ---------------------------------------------------------------
 * Thin domain layer over Poppler::Document.
 *
 * Responsibilities:
 *  - Open and close PDF files (ownership via unique_ptr).
 *  - Expose basic metadata (title, page count, file path).
 *  - Provide individual pages as unique_ptr<Poppler::Page> so the caller
 *    owns each page object and controls render lifetime.
 *
 * Design notes:
 *  - Does not cache pages: delegates to Poppler keeping the API minimal.
 *  - Never throws exceptions: error signaling via booleans / nullptr.
 *  - Thread-safety: not guaranteed (mirrors Poppler Qt backend limitations).
 */
class PDFDocument
{
public:
    PDFDocument();
    ~PDFDocument();

    // Lifecycle -----------------------------------------------------
    bool loadFromFile(const QString &filePath); // Returns false on failure or locked file.
    void close();                               // Release resources (idempotent).
    bool isLoaded() const;                      // Fast state check.

    // Metadata ------------------------------------------------------
    int pageCount() const;    // 0 if not loaded.
    QString title() const;    // Uses PDF metadata; fallback: file name.
    QString filePath() const; // Absolute current file path.
    bool isLocked() const;    // True if PDF is password protected.

    // Page access ---------------------------------------------------
    std::unique_ptr<Poppler::Page> getPage(int pageIndex) const; // nullptr if out of range.

private:
    std::unique_ptr<Poppler::Document> m_document; // Underlying Poppler document.
    QString m_filePath;                            // Source path (for title fallback).
};

#endif // PDFDOCUMENT_H
