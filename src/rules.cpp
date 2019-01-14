#include "match_result.h"
#include "text_to_match.h"

#include "rules.h"


AbstractRule::AbstractRule(const AbstractRuleParams& params):
    lookAhead(params.lookAhead),
    attribute(params.attribute),
    context(params.context),
    firstNonSpace(params.firstNonSpace),
    column(params.column),
    dynamic(params.dynamic)
{}

void AbstractRule::printDescription(QTextStream& out) const {
    out << "\t\t" << description() << "\n";
}

QString AbstractRule::description() const {
    return QString("%1(%2)").arg(name()).arg(args());
}

void AbstractRule::resolveContextReferences(const QHash<QString, ContextPtr>& contexts, QString& error) {
    context.resolveContextReferences(contexts, error);
}

void AbstractRule::setStyles(const QHash<QString, Style>& styles, QString& error) {
    if ( ! attribute.isNull()) {
        if ( ! styles.contains(attribute)) {
            error = QString("Not found rule %1 attribute %2").arg(description(), attribute);
            return;
        }
        style = styles[attribute];
        style.updateTextType(attribute);
    }
}

MatchResult* AbstractRule::tryMatch(const TextToMatch& textToMatch) const {
    if (column != -1 && column != textToMatch.currentColumnIndex) {
        return nullptr;
    }

    if (firstNonSpace && (not textToMatch.firstNonSpace)) {
        return nullptr;
    }

    return tryMatchImpl(textToMatch);
}

AbstractStringRule::AbstractStringRule(const AbstractRuleParams& params,
                                       const QString& value,
                                       bool insensitive):
    AbstractRule(params),
    value(value),
    insensitive(insensitive)
{}

QString AbstractStringRule::args() const {
    QString result = value;
    if (insensitive) {
        result += " insensitive";
    }

    return result;
}


MatchResult* AbstractRule::tryMatchImpl(const TextToMatch& textToMatch) const {
    return nullptr;
}


KeywordRule::KeywordRule(const AbstractRuleParams& params,
                         const QString& listName):
    AbstractRule(params),
    listName(listName),
    caseSensitive(true)
{}

void KeywordRule::setKeywordParams(const QHash<QString, QStringList>& lists,
                                   const QString& deliminators,
                                   bool caseSensitive,
                                   QString& error) {
    if ( ! lists.contains(listName)) {
        error = QString("List '%1' not found").arg(error);
        return;
    }
    items = lists[listName];
    this->caseSensitive = caseSensitive;
    this->deliminators = deliminators;
}


DetectCharRule::DetectCharRule(const AbstractRuleParams& params,
                               const QString& value,
                               int index):
    AbstractRule(params),
    value(value),
    index(index)
{}

QString DetectCharRule::args() const {
    if (value.isNull()) {
        return QString("index: %1").arg(index);
    } else {
        return value;
    }
}


RegExpRule::RegExpRule(const AbstractRuleParams& params,
                       const QString& value, bool insensitive,
                       bool minimal, bool wordStart, bool lineStart):
    AbstractRule(params),
    value(value),
    insensitive(insensitive),
    minimal(minimal),
    wordStart(wordStart),
    lineStart(lineStart)
{}

QString RegExpRule::args() const {
    QString result = value;
    if (insensitive) {
        result += " insensitive";
    }
    if (minimal) {
        result += " minimal";
    }
    if (wordStart) {
        result += " wordStart";
    }
    if (lineStart) {
        result += " lineStart";
    }

    return result;
}

AbstractNumberRule::AbstractNumberRule(const AbstractRuleParams& params,
                                       const QList<RulePtr>& childRules):
    AbstractRule(params),
    childRules(childRules)
{}

void AbstractNumberRule::printDescription(QTextStream& out) const {
    AbstractRule::printDescription(out);

    foreach(RulePtr rule, childRules) {
        out << "\t\t\t" << rule->description() << "\n";
    }
}


RangeDetectRule::RangeDetectRule(const AbstractRuleParams& params, const QString& char0, const QString& char1):
    AbstractRule(params),
    char0(char0),
    char1(char1)
{}

QString RangeDetectRule::args() const {
    return QString("%1 - %2").arg(char0, char1);
}

IncludeRulesRule::IncludeRulesRule(const AbstractRuleParams& params, const QString& contextName):
    AbstractRule(params),
    contextName(contextName)
{}

void IncludeRulesRule::resolveContextReferences(const QHash<QString, ContextPtr>& contexts, QString& error) {
    AbstractRule::resolveContextReferences(contexts, error);
    if ( ! error.isNull()) {
        return;
    }

    if (contextName.startsWith('#')) {
        return; //TODO
    }

    if ( ! contexts.contains(contextName)) {
        error = QString("Failed to include rules from context '%1' because not exists").arg(contextName);
        return;
    }

    context = contexts[contextName];
}
