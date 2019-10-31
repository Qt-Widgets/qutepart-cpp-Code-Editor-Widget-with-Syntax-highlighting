#pragma once

#include <QPlainTextEdit>
#include <QSharedPointer>
#include <QSyntaxHighlighter>

#include "indent/indenter.h"


namespace Qutepart {

class Qutepart: public QPlainTextEdit {
public:
    Qutepart(QWidget *parent = nullptr);
    Qutepart(const QString &text, QWidget *parent = nullptr);

    // Not copyable or movable
    Qutepart(const Qutepart&) = delete;
    Qutepart& operator=(const Qutepart&) = delete;
    Qutepart(Qutepart&&) = delete;
    Qutepart& operator=(Qutepart&&) = delete;

    void initHighlighter(const QString& filePath);

private:
    QSharedPointer<QSyntaxHighlighter> highlighter;
    Indenter indenter;
};

}; // namespace Qutepart
