#include <QDebug>

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

MatchResult* AbstractRule::makeMatchResult(int length, bool lineContinue) const {
    qDebug() << "\trule matched" << description() << length << "lookAhead" << lookAhead;
    if (lookAhead) {
        length = 0;
    }

    return new MatchResult(length, nullptr, lineContinue, context, style);
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


MatchResult* StringDetectRule::tryMatchImpl(const TextToMatch& textToMatch) const {

    if (value.isEmpty()) {
        return nullptr;
    }

#if 0 // TODO dynamic
    if self.dynamic:
        string = self._makeDynamicSubsctitutions(self.string, textToMatchObject.contextData)
    else:
        string = self.string
#endif

    if (textToMatch.text.startsWith(value)) {
        return makeMatchResult(value.length());
    }

    return nullptr;
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

MatchResult* KeywordRule::tryMatchImpl(const TextToMatch& textToMatch) const {
    if (textToMatch.word.isEmpty()) {
        return nullptr;
    }

    bool matched = false;
    if (this->caseSensitive) {
        matched = items.contains(textToMatch.word.toLower());
    } else {
        matched = items.contains(textToMatch.word);
    }

    if (matched) {
        return makeMatchResult(textToMatch.word.length(), false);
    } else {
        return nullptr;
    }
}

DetectCharRule::DetectCharRule(const AbstractRuleParams& params,
                               QChar value,
                               int index):
    AbstractRule(params),
    value(value),
    index(index)
{}

QString DetectCharRule::args() const {
    if (value == '\0') {
        return QString("index: %1").arg(index);
    } else {
        return value;
    }
}

MatchResult* DetectCharRule::tryMatchImpl(const TextToMatch& textToMatch) const {
    // TODO support dynamic

    if (textToMatch.text.at(0) == value) {
        return makeMatchResult(1, false);
    } else {
        return nullptr;
    }
}

MatchResult* AnyCharRule::tryMatchImpl(const TextToMatch& textToMatch) const {
    if (value.contains(textToMatch.text.at(0))) {
        return makeMatchResult(1);
    }

    return nullptr;
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
{
    if ( ! dynamic) {
        regExp = compileRegExp(value);
    }
}

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

QRegularExpression RegExpRule::compileRegExp(const QString& pattern) const {
   QRegularExpression::PatternOptions flags = QRegularExpression::NoPatternOption;

    if (insensitive) {
        flags |= QRegularExpression::CaseInsensitiveOption;
    }

    if (minimal) {
        /* NOTE
         * There are support for minimal flag in Qt5 reg exps.
         * InvertedGreedinessOption would work only if reg exps are
         * written as non-greedy in the syntax files
         */
        flags |= QRegularExpression::InvertedGreedinessOption;
    }

    QRegularExpression result(pattern, flags);
    if ( ! result.isValid()) {
        qWarning() << "Invalid regular expression pattern" << pattern;
    }

    return result;
}

MatchResult* RegExpRule::tryMatchImpl(const TextToMatch& textToMatch) const {
    // Special case. if pattern starts with \b, we have to check it manually,
    // because string is passed to .match(..) without beginning
    if (wordStart && ( ! textToMatch.isWordStart)) {
        return nullptr;
    }

    //Special case. If pattern starts with ^ - check column number manually
    if (lineStart && textToMatch.currentColumnIndex > 0) {
        return nullptr;
    }


    QRegularExpressionMatch match;
    if (dynamic) {
#if 0 // TODO dynamic
        QString pattern = makeDynamicSubsctitutions(textToMatchObject.contextData)
        QRegularExpression dynamicRegExp = compileRegExp(pattern)
        match = dynamicRegExp.match(text);
#endif
    } else {
        match = regExp.match(textToMatch.text);
    }

    if (match.hasMatch() && match.capturedStart() == 0) {
        return makeMatchResult(match.capturedLength()); // TODO dynamic data
    } else {
        return nullptr;
    }
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

MatchResult* AbstractNumberRule::tryMatchImpl(const TextToMatch& textToMatch) const {
    // andreikop: This condition is not described in kate docs, and I haven't found it in the code
    if ( ! textToMatch.isWordStart) {
        return nullptr;
    }

    int matchedLength = tryMatchText(textToMatch.text);

    if (matchedLength <= 0) {
        return nullptr;
    }

    if (matchedLength < textToMatch.text.length()){
        TextToMatch textToMatchCopy = textToMatch;
        textToMatchCopy.shift(matchedLength);

        foreach(RulePtr rule, childRules) {
            MatchResult* matchRes = rule->tryMatch(textToMatchCopy);
            if (matchRes != nullptr) {
                matchedLength += matchRes->length;
                delete matchRes;
                break;
            }
            // child rule context and attribute ignored
        }
    }

    return makeMatchResult(matchedLength);
}

int AbstractNumberRule::countDigits(const QStringRef& text) const {
    int index = 0;
    for(index = 0; index < text.length(); index++) {
        if ( ! text.at(index).isDigit()) {
            break;
        }
    }
    return index;
}


int IntRule::tryMatchText(const QStringRef& text) const {
    return countDigits(text);
}


int FloatRule::tryMatchText(const QStringRef& text) const {
    bool haveDigit = false;
    bool havePoint = false;

    int matchedLength = 0;

    int digitCount = countDigits(text);

    if (digitCount > 0) {
        haveDigit = true;
        matchedLength += digitCount;
    }

    if (text.length() > matchedLength && text.at(matchedLength) == '.') {
        havePoint = true;
        matchedLength++;
    }

    digitCount = countDigits(text.mid(matchedLength));
    if (digitCount > 0) {
        haveDigit = true;
        matchedLength += digitCount;
    }

    if (text.length() > matchedLength && text.at(matchedLength).toLower() == 'e') {
        matchedLength++;

        if (text.length() > matchedLength &&
            (text.at(matchedLength) == '+' ||
             text.at(matchedLength) == '-')) {
            matchedLength++;
        }

        bool haveDigitInExponent = false;

        digitCount = countDigits(text.mid(matchedLength));
        if (digitCount > 0) {
            haveDigitInExponent = true;
            matchedLength += digitCount;
        }

        if ( ! haveDigitInExponent) {
            return -1;
        }

        return matchedLength;
    } else {
        if ( ! havePoint) {
            return -1;
        }
    }

    if (matchedLength > 0 && haveDigit) {
        return matchedLength;
    } else {
        return -1;
    }
}


// For HlCOctRule and HlCHexRule
static bool isNumberLengthSpecifier(QChar ch) {
    return ch == 'l' ||
           ch == 'L' ||
           ch == 'u' ||
           ch == 'U';
}

bool HlCOctRule::isOctal(QChar ch) {
    return ch >= '0' && ch <= '7';
}

MatchResult* HlCOctRule::tryMatchImpl(const TextToMatch& textToMatch) const {
    if (textToMatch.text.at(0) != '0') {
        return nullptr;
    }

    int index = 1;
    while (index < textToMatch.text.length() &&
           isOctal(textToMatch.text.at(index))) {
        index ++;
    }

    if (index == 1){
        return nullptr;
    }

    if(index < textToMatch.text.length() &&
       isNumberLengthSpecifier(textToMatch.text.at(index))) {
        index++;
    }

    return makeMatchResult(index);
}


bool HlCHexRule::isHex(QChar ch) {
    return (ch >= '0' && ch <= '9') ||
           (ch >= 'a' && ch <= 'f') ||
           (ch >= 'A' && ch <= 'F');
}

MatchResult* HlCHexRule::tryMatchImpl(const TextToMatch& textToMatch) const {
    if (textToMatch.text.length() < 3) {
        return nullptr;
    }

    if (textToMatch.text.at(0) != '0' ||
        textToMatch.text.at(1).toUpper() != 'X') {
        return nullptr;
    }

    int index = 2;
    while (index < textToMatch.text.length() &&
           isHex(textToMatch.text.at(index))) {
        index++;
    }

    if (index == 2) {
        return nullptr;
    }

    if(index < textToMatch.text.length() &&
       isNumberLengthSpecifier(textToMatch.text.at(index))) {
        index++;
    }

    return makeMatchResult(index);
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

MatchResult* IncludeRulesRule::tryMatchImpl(const TextToMatch& textToMatch) const {
    if (context == nullptr) {
        qWarning() << "IncludeRules called for null context" << description();
        return nullptr;
    }

    return context->tryMatch(textToMatch);
}


MatchResult* DetectIdentifierRule::tryMatchImpl(const TextToMatch& textToMatch) const {
    if (textToMatch.text[0].isLetter()) {
        int count = 1;
        while(count < textToMatch.text.length()) {
            QChar ch = textToMatch.text.at(count);
            if (ch.isLetterOrNumber() || ch == '_') {
                count ++;
            } else {
                break;
            }
        }

        return makeMatchResult(count);
    }
    else {
        return nullptr;
    }
}
