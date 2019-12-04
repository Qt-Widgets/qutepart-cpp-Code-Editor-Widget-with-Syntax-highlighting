#include <QDebug>

#include "text_block_utils.h"

#include "alg_lisp.h"
#include "alg_xml.h"

#include "indenter.h"


namespace Qutepart {

namespace {

class IndentAlgNone: public IndentAlgImpl {
public:
    QString computeSmartIndent(
            QTextBlock block,
            const QString& configuredIndent,
            QChar typedKey) const override {
        return QString::null;
    }
};


class IndentAlgNormal: public IndentAlgImpl {
public:
    QString computeSmartIndent(
        QTextBlock block,
            const QString& configuredIndent,
        QChar typedKey) const override {
        return prevNonEmptyBlockIndent(block);
    }
};

const QString NULL_STRING = QString::null;
}  // namespace


const QString& IndentAlgImpl::triggerCharacters() const {
    return NULL_STRING;
}


Indenter::Indenter():
    alg_(std::make_unique<IndentAlgNormal>()),
    useTabs_(false),
    width_(4)
{
}

void Indenter::setAlgorithm(IndentAlg alg) {
    switch (alg) {
        case INDENT_ALG_NONE:
            alg_ = std::make_unique<IndentAlgNone>();
        break;
        case INDENT_ALG_NORMAL:
            alg_ = std::make_unique<IndentAlgNormal>();
        break;
        case INDENT_ALG_LISP:
            alg_ = std::make_unique<IndentAlgLisp>();
        break;
        case INDENT_ALG_XML:
            alg_ = std::make_unique<IndentAlgXml>();
        break;
        default:
            qWarning() << "Wrong indentation algorithm requested" << alg;
        break;
    }
}

QString Indenter::text() const {
    if (useTabs_){
        return "\t";
    } else {
        return QString().fill(' ', width_);
    }
}

int Indenter::width() const {
    return width_;
}

void Indenter::setWidth(int width) {
    width_ = width;
}

bool Indenter::useTabs() const {
    return useTabs_;
}

void Indenter::setUseTabs(bool use) {
    useTabs_ = use;
}

bool Indenter::shouldAutoIndentOnEvent(QKeyEvent* event) const {
    return ( ! event->text().isEmpty() &&
            alg_->triggerCharacters().contains(event->text()));
}

bool Indenter::shouldUnindentWithBackspace(const QTextCursor& cursor) const {
    return textBeforeCursor(cursor).endsWith(text()) &&
           ( ! cursor.hasSelection()) &&
           (cursor.atBlockEnd() ||
            ( ! cursor.block().text()[cursor.positionInBlock() + 1].isSpace()));
}

void Indenter::indentBlock(QTextBlock block, QChar typedKey) const {
    QString prevBlockText = block.previous().text();  // invalid block returns empty text
    QString indent;
    if (typedKey == '\r' &&
        prevBlockText.trimmed().isEmpty()) {  // continue indentation, if no text
        indent = prevBlockIndent(block);

        if ( ! indent.isNull()) {
            QTextCursor cursor(block);
            cursor.insertText(indent);
        }
    } else {  // be smart
        QString indentedLine = alg_->computeSmartIndent(block, text(), typedKey) + stripLeftWhitespace(block.text());
        if ( (! indentedLine.isNull()) &&
             indentedLine != block.text()) {
            QTextCursor cursor(block);
            cursor.select(QTextCursor::LineUnderCursor);
            cursor.insertText(indentedLine);
        }
    }
}

void Indenter::onShortcutIndent(QTextCursor cursor) const {
    if (cursor.positionInBlock() == 0) {  // if no any indent - indent smartly
        QTextBlock block = cursor.block();
        QString indentedLine = alg_->computeSmartIndent(block, text());
        if (indentedLine.isEmpty()) {
            indentedLine = text();
        }
        cursor.insertText(indentedLine);
    } else {  // have some indent, insert more
        QString indent;
        if (useTabs_){
            indent = "\t";
        } else {
            int charsToInsert = width_ - (textBeforeCursor(cursor).length() % width_);
            indent.fill(' ', charsToInsert);
        }
        cursor.insertText(indent);
    }
}

void Indenter::onShortcutUnindentWithBackspace(QTextCursor& cursor) const {
    int charsToRemove = textBeforeCursor(cursor).length() % text().length();

    if (charsToRemove == 0) {
        charsToRemove = text().length();
    }

    cursor.setPosition(cursor.position() - charsToRemove, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
}

}  // namespace Qutepart
