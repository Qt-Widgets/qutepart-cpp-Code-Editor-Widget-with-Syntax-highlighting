#pragma once

#include <QString>
#include <QSharedPointer>
#include <QTextStream>


#include "context.h"

class Context;
typedef QSharedPointer<Context> ContextPtr;
class TextToMatch;


struct AbstractRuleParams {
    QString attribute;          // may be null
    ContextSwitcher context;
    bool lookAhead;
    bool firstNonSpace;
    int column;                 // -1 if not set
    bool dynamic;
};


class AbstractRule {
public:
    AbstractRule(const AbstractRuleParams& params);
    virtual ~AbstractRule() {};

    virtual void printDescription(QTextStream& out) const;
    virtual QString description() const;

    virtual void resolveContextReferences(const QHash<QString, ContextPtr>& contexts, QString& error);
    virtual void setKeywordParams(const QHash<QString, QStringList>&, const QString&, bool, QString&) {};
    void setStyles(const QHash<QString, Style>& styles, QString& error);

    bool lookAhead;

    const Style& style() const {return _style;}
    const ContextSwitcher& context() const {return _context;}

    MatchResult tryMatch(const TextToMatch& textToMatch) const;

protected:
    virtual QString name() const {return "AbstractRule";};
    virtual QString args() const {return QString::null;};

    QString attribute;          // may be null
    ContextSwitcher _context;
    bool firstNonSpace;
    int column;                 // -1 if not set
    bool dynamic;

    Style _style;
};

// A rule which has 1 string as a parameter
class AbstractStringRule: public AbstractRule {
public:
    AbstractStringRule (const AbstractRuleParams& params,
                        const QString& value,
                        bool insensitive);

protected:
    QString args() const override;
    QString value;
    bool insensitive;
};


class KeywordRule: public AbstractRule {
public:
    KeywordRule(const AbstractRuleParams& params,
                const QString& listName);

    void setKeywordParams(const QHash<QString, QStringList>& lists,
                          const QString& deliminators,
                          bool caseSensitive,
                          QString& error) override;

    QString name() const override {return "Keyword";};
    QString args() const override {return listName;};

private:
    QString listName;
    QStringList items;
    bool caseSensitive;
    QString deliminators;
};


class DetectCharRule: public AbstractRule {
public:
    DetectCharRule(const AbstractRuleParams& params,
                   const QString& value,
                   int index);

    QString name() const override {return "DetectChar";};
    QString args() const override;

private:
    QString value;
    int index;
};


class Detect2CharsRule: public AbstractStringRule {
    using AbstractStringRule::AbstractStringRule;
public:

    QString name() const override {return "Detect2Chars";};
};


class AnyCharRule: public AbstractStringRule {
    using AbstractStringRule::AbstractStringRule;
public:

    QString name() const override {return "AnyChar";};
};


class StringDetectRule: public AbstractStringRule {
    using AbstractStringRule::AbstractStringRule;
public:

    QString name() const override {return "StringDetect";};
};


class WordDetectRule: public AbstractStringRule {
    using AbstractStringRule::AbstractStringRule;
public:
    QString name() const override {return "WordDetect";};
};

class RegExpRule: public AbstractRule {
public:
    RegExpRule(const AbstractRuleParams& params,
               const QString& value, bool insensitive,
               bool minimal, bool wordStart, bool lineStart);

    QString name() const override {return "RegExpr";};
    QString args() const override;
private:
    QString value;
    bool insensitive;
    bool minimal;
    bool wordStart;
    bool lineStart;
};


class AbstractNumberRule: public AbstractRule {
public:
    AbstractNumberRule(const AbstractRuleParams& params,
                       const QList<RulePtr>& childRules);

    void printDescription(QTextStream& out) const override;
protected:
    QList<RulePtr> childRules;
};

class IntRule: public AbstractNumberRule {
    using AbstractNumberRule::AbstractNumberRule;

public:
    QString name() const override {return "Int";};
};

class FloatRule: public AbstractNumberRule {
    using AbstractNumberRule::AbstractNumberRule;

public:
    QString name() const override {return "Float";};
};

class HlCOctRule: public AbstractRule {
    using AbstractRule::AbstractRule;

public:
    QString name() const override {return "HlCOct";};
};

class HlCHexRule: public AbstractRule {
    using AbstractRule::AbstractRule;

public:
    QString name() const override {return "HlCHex";};
};

class HlCStringCharRule: public AbstractRule {
    using AbstractRule::AbstractRule;

public:
    QString name() const override {return "HlCStringChar";};
};


class HlCCharRule: public AbstractRule {
    using AbstractRule::AbstractRule;

public:
    QString name() const override {return "HlCChar";};
};

class RangeDetectRule: public AbstractRule {
public:
    RangeDetectRule(const AbstractRuleParams& params, const QString& char0, const QString& char1);
    QString name() const override {return "RangeDetect";};
    QString args() const override;

private:
    const QString char0;
    const QString char1;
};

class LineContinueRule: public AbstractRule {
    using AbstractRule::AbstractRule;

public:
    QString name() const override {return "LineContinue";};
};


class IncludeRulesRule: public AbstractRule {
public:
    IncludeRulesRule(const AbstractRuleParams& params, const QString& contextName);

    QString name() const override {return "IncludeRules";};
    QString args() const override {return contextName;};

    void resolveContextReferences(const QHash<QString, ContextPtr>& contexts, QString& error) override;

private:
    QString contextName;
    ContextPtr context;
};


class DetectSpacesRule: public AbstractRule {
    using AbstractRule::AbstractRule;

public:
    QString name() const override {return "DetectSpaces";};
};

class DetectIdentifierRule: public AbstractRule {
    using AbstractRule::AbstractRule;

public:
    QString name() const override {return "DetectIdentifier";};
};
