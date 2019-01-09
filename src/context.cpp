#include "context.h"
#include "rules.h"
#include "text_to_match.h"



Context::Context(const QString& name,
                 const QString& attribute,
                 const ContextSwitcher& lineEndContext,
                 const ContextSwitcher& lineBeginContext,
                 const ContextSwitcher& fallthroughContext,
                 bool dynamic,
                 const QList<RulePtr>& rules):
    _name(name),
    attribute(attribute),
    _lineEndContext(lineEndContext),
    _lineBeginContext(lineBeginContext),
    fallthroughContext(fallthroughContext),
    _dynamic(dynamic),
    rules(rules)
{}

void Context::printDescription(QTextStream& out) const {
    out << "\tContext " << this->_name << "\n";
    out << "\t\tattribute: " << attribute << "\n";
    if( ! _lineEndContext.isNull()) {
        out << "\t\tlineEndContext: " << _lineEndContext.toString() << "\n";
    }
    if( ! _lineBeginContext.isNull()) {
        out << "\t\tlineBeginContext: " << _lineBeginContext.toString() << "\n";
    }
    if( ! fallthroughContext.isNull()) {
        out << "\t\tfallthroughContext: " << fallthroughContext.toString() << "\n";
    }
    if(_dynamic) {
        out << "\t\tdynamic\n";
    }

    foreach(RulePtr rule, rules) {
        rule->printDescription(out);
    }
}

QString Context::name() const {
    return _name;
}

void Context::resolveContextReferences(const QHash<QString, ContextPtr>& contexts, QString& error) {
    _lineEndContext.resolveContextReferences(contexts, error);
    if ( ! error.isNull()) {
        return;
    }

    _lineBeginContext.resolveContextReferences(contexts, error);
    if ( ! error.isNull()) {
        return;
    }

    fallthroughContext.resolveContextReferences(contexts, error);
    if ( ! error.isNull()) {
        return;
    }

    foreach(RulePtr rule, rules) {
        rule->resolveContextReferences(contexts, error);
        if ( ! error.isNull()) {
            return;
        }
    }
}

void Context::setKeywordParams(const QHash<QString, QStringList>& lists,
                               const QString& deliminators,
                               bool caseSensitive,
                               QString& error) {
    foreach(RulePtr rule, rules) {
        rule->setKeywordParams(lists, deliminators, caseSensitive, error);
        if ( ! error.isNull()) {
            break;
        }
    }
}

void Context::setStyles(const QHash<QString, Style>& styles, QString& error) {
    if ( ! attribute.isNull()) {
        if ( ! styles.contains(attribute)) {
            error = QString("Not found context '%1' attribute '%2'").arg(_name, attribute);
            return;
        }
        style = styles[attribute];
        style.updateTextType(attribute);
    }

    foreach(RulePtr rule, rules) {
        rule->setStyles(styles, error);
        if ( ! error.isNull()) {
            break;
        }
    }
}

ContextSwitcher* Context::parseBlock(TextToMatch& textToMatch,
                                     QVector<QTextLayout::FormatRange>& formats,
                                     QString& textTypeMap,
                                     bool& lineContinue) const {

    return nullptr;
}
