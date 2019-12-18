#pragma once

#include <memory>

#include <QObject>
#include <QString>
#include <QTextBlock>
#include <QKeyEvent>

#include "qutepart.h"


namespace Qutepart {

class IndentAlgImpl {
public:
    void setConfig(int width, bool useTabs);

    virtual const QString& triggerCharacters() const;

    /* Compute line with proper indentation
     * and other formatting.
     * i.e. split line onto multiple lines.
     *
     * Default implementation indents the line with computeSmartIndent()

     * Return value QString::null means 'do not modify the string'
     */
    virtual QString autoFormatLine(QTextBlock block) const;

    /* Compute indent for line.
     * The majority of algorithms should override this method.
     *
     * cursorPos is -1 if autoIndent shortcut is triggered,
     * actual position if text is being typed
     *
     * Default implementation returns empty string
     */
    virtual QString computeSmartIndent(QTextBlock block, int cursorPos) const;

protected:
    int width_;
    bool useTabs_;

    QString indentText() const;
};


class Indenter: public QObject {
public:
    Indenter();

    void setAlgorithm(IndentAlg alg);

    QString indentText() const;

    int width() const;
    void setWidth(int);

    bool useTabs() const;
    void setUseTabs(bool);

    bool shouldAutoIndentOnEvent(QKeyEvent* event) const;
    bool shouldUnindentWithBackspace(const QTextCursor& cursor) const;
#if 0
    void autoIndentBlock(QTextBlock block, QChar typedKey) const;
#endif
    void indentBlock(QTextBlock block, int column, QChar typedKey) const;

public slots:
    void onShortcutIndent(QTextCursor cursor) const;
    void onShortcutUnindentWithBackspace(QTextCursor& cursor) const;

private:
    std::unique_ptr<IndentAlgImpl> alg_;
    bool useTabs_;
    int width_;
};
};
