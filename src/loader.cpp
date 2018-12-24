#include <QDebug>

#include "rules.h"
#include "loader.h"


QList<RulePtr> loadRules(QXmlStreamReader& xmlReader, QString& error);


QString getAttribute(QXmlStreamAttributes attrs, QString name,
                     QString defaultValue=QString::null,
                     bool warnIfNotSet=false) {
    if (attrs.hasAttribute(name)) {
        return attrs.value(name).toString();
    } else {
        if (warnIfNotSet) {
            qWarning() << "Required attribute " << name << " is not set";
        }
        return defaultValue;
    }
}

QString getRequiredAttribute(QXmlStreamAttributes attrs, QString name, QString& error) {
    if (attrs.hasAttribute(name)) {
        return attrs.value(name).toString();
    } else {
        error = QString("Required attribute %1 is not set").arg(name);
        return QString::null;
    }
}

bool parseBoolAttribute(const QString& value, QString& error) {
    QString lowerValue = value.toLower();

    if(lowerValue  == "true" || lowerValue == "1") {
        return true;
    } else if (lowerValue == "false" || lowerValue == "0") {
        return false;
    } else {
        error = QString("Invalid bool attribute value '%1'").arg(value);
        return false;
    }
}

int parseIntAttribute(const QString& value, QString& error) {
    bool intOk = true;
    int result = value.toInt(&intOk);
    if ( ! intOk) {
        error = QString("Bad integer value '%1'").arg(value);
    }

    return result;
}

QString processEscapeSequences(QString value) {
    value.replace("\\a", "\a");
    value.replace("\\b", "\b");
    value.replace("\\f", "\f");
    value.replace("\\n", "\n");
    value.replace("\\r", "\r");
    value.replace("\\t", "\t");
    value.replace("\\\\", "\\");

    return value;
}

ContextSwitcher makeContextSwitcher(QString contextOperation/*, parser, formatConverterFunction*/) {
    int popsCount = 0;
    ContextPtr contextToSwitch;

    QString rest = contextOperation;
    while(rest.startsWith("#pop")){
        popsCount += 1;
        rest = rest.remove(0, 4); // 4 is length of '#pop'
    }

    if(rest == "#stay"){
        if (popsCount > 0){
            qWarning() << QString("Invalid context operation '%1'").arg(contextOperation);
        }
    } else {
        // TODO
        // contextToSwitch = _getContext(rest, parser, formatConverterFunction, None)
    }

    if (popsCount > 0 || ( ! contextToSwitch.isNull())){
        return ContextSwitcher(popsCount, contextToSwitch, contextOperation);
    } else {
        return ContextSwitcher();
    }
}

AbstractRuleParams parseAbstractRuleParams(const QXmlStreamAttributes& attrs, QString& error) {
    QString attribute = getAttribute(attrs, "attribute");

    QString contextText = getAttribute(attrs, "context", "#stay");
    ContextSwitcher context = makeContextSwitcher(contextText);

    bool lookAhead = parseBoolAttribute(getAttribute(attrs, "lookAhead", "false"), error);
    if ( ! error.isNull()) {
        error = QString("Failed to parse 'lookAhead': %1").arg(error);
        return AbstractRuleParams();
    }

    bool firstNonSpace = parseBoolAttribute(getAttribute(attrs, "firstNonSpace", "false"), error);
    if ( ! error.isNull()) {
        error = QString("Failed to parse 'firstNonSpace': %1").arg(error);
        return AbstractRuleParams();
    }

    bool dynamic = parseBoolAttribute(getAttribute(attrs, "dynamic", "false"), error);
    if ( ! error.isNull()) {
        error = QString("Failed to parse 'dynamic': %1").arg(error);
        return AbstractRuleParams();
    }

    char textType = 0; // TODO

    // TODO beginRegion
    // TODO endRegion

    int column = -1;
    QString columnStr = getAttribute(attrs, "column");
    if ( ! columnStr.isNull()) {
        column = parseIntAttribute(columnStr, error);
        if ( ! error.isNull()) {
            error = QString("Bad integer column value: %1").arg(error);
            return AbstractRuleParams();
        }
    }

#if 0
    # attribute
    if attribute is not None:
        attribute = attribute.lower()  # not case sensitive
        try:
            format = attributeToFormatMap[attribute]
            textType = format.textType if format is not None else ' '
            if formatConverterFunction is not None and format is not None:
                format = formatConverterFunction(format)
        except KeyError:
            _logger.warning('Unknown rule attribute %s', attribute)
            format = parentContext.format
            textType = parentContext.textType
    else:
        format = None
        textType = None
#endif

    return AbstractRuleParams {
        textType, attribute,
        context, lookAhead,
        firstNonSpace, column,
        dynamic};
}

template<class RuleClass> RuleClass* loadStringRule(const QXmlStreamAttributes& attrs,
                                                    const AbstractRuleParams& params,
                                                    QString& error) {
    QString value = getRequiredAttribute(attrs, "String", error);
    if ( ! error.isNull()) {
        return nullptr;
    }

    bool insensitive = parseBoolAttribute(getAttribute(attrs, "insensitive", "false"), error);
    if ( ! error.isNull()) {
        error = QString("Failed to parse 'insensitive': %1").arg(error);
        return nullptr;
    }

    return new RuleClass(params, value, insensitive);
}

DetectCharRule* loadDetectChar(const QXmlStreamAttributes& attrs,
                               const AbstractRuleParams& params,
                               QString& error) {
    QString value = getRequiredAttribute(attrs, "char", error);
    if ( ! error.isNull()) {
        return nullptr;
    }

    value = processEscapeSequences(value);

    int index = 0;

    if (params.dynamic) {
        bool ok = false;
        index = value.toInt(&ok);
        if ( ! ok) {
            error = QString("Bad integer value: %1").arg(value);
            return nullptr;
        }

        if (index < 0) {
            error = QString("Bad integer value: %1").arg(value);
            return nullptr;
        }
        value = QString::null;
    }

    return new DetectCharRule(params, value, index);
}

Detect2CharsRule* loadDetect2Chars(const QXmlStreamAttributes& attrs,
                                   const AbstractRuleParams& params,
                                   QString& error) {

    QString char0 = getRequiredAttribute(attrs, "char", error);
    if ( ! error.isNull()) {
        return nullptr;
    }
    QString char1 = getRequiredAttribute(attrs, "char1", error);
    if ( ! error.isNull()) {
        return nullptr;
    }

    QString value = processEscapeSequences(char0 + char1);

    return new Detect2CharsRule(params, value, false);
}

RegExpRule* loadRegExp(const QXmlStreamAttributes& attrs,
                       const AbstractRuleParams& params,
                       QString& error) {
    QString value = getRequiredAttribute(attrs, "String", error);
    if ( ! error.isNull()) {
        return nullptr;
    }

    bool insensitive = parseBoolAttribute(getAttribute(attrs, "insensitive", "false"), error);
    if ( ! error.isNull()) {
        error = QString("Failed to parse 'insensitive': %1").arg(error);
        return nullptr;
    }

    bool minimal = parseBoolAttribute(getAttribute(attrs, "minimal", "false"), error);
    if ( ! error.isNull()) {
        error = QString("Failed to parse 'minimal': %1").arg(error);
        return nullptr;
    }

    bool wordStart = false; // TODO
    bool lineStart = false; // TODO

    return new RegExpRule(params, value, insensitive, minimal, wordStart, lineStart);
}

template<class RuleClass>
RuleClass* loadNumberRule(QXmlStreamReader& xmlReader,
                          const AbstractRuleParams& params,
                          QString& error) {
    QList<RulePtr> children = loadRules(xmlReader, error);

    if ( ! error.isNull()) {
        error = QString("Failed to load child rules of number: %1").arg(error);
        return nullptr;
    }

    xmlReader.readNextStartElement(); // go to next tag

    return new RuleClass(params, children);
}

AbstractRule* loadRule(QXmlStreamReader& xmlReader, QString& error) {
    QXmlStreamAttributes attrs = xmlReader.attributes();

    AbstractRuleParams params = parseAbstractRuleParams(attrs, error);
    if ( ! error.isNull()) {
        return nullptr;
    }

    QStringRef name = xmlReader.name();

    AbstractRule* result = nullptr;
    if (name == "keyword") {
        result = loadStringRule<KeywordRule>(attrs, params, error);
    } else if (name == "DetectChar") {
        result = loadDetectChar(attrs, params, error);
    } else if (name == "Detect2Chars") {
        result = loadDetect2Chars(attrs, params, error);
    } else if (name == "AnyChar") {
        result = loadStringRule<AnyCharRule>(attrs, params, error);
    } else if (name == "StringDetect") {
        result = loadStringRule<StringDetectRule>(attrs, params, error);
    } else if (name == "WordDetect") {
        result = loadStringRule<WordDetectRule>(attrs, params, error);
    } else if (name == "RegExpr") {
        result = loadRegExp(attrs, params, error);
    } else if (name == "Int") {
        result = loadNumberRule<IntRule>(xmlReader, params, error);
    } else if (name == "Float") {
        result = loadNumberRule<FloatRule>(xmlReader, params, error);
    } else if (name == "HlCHex") {
        result = new HlCHexRule(params);
    } else if (name == "HlCOct") {
        result = new HlCOctRule(params);
    } else if (name == "HlCStringChar") {
        result = new HlCStringCharRule(params);
    } else if (name == "HlCChar") {
        result = new HlCCharRule(params);
    } else {
        result = new AbstractRule(/*parentContext,*/ params);
    }

    if ( ! error.isNull()) {
        return nullptr;
    }

    bool ret = xmlReader.readNextStartElement();
    if (ret) {
        // TODO proper nested tags reading here
        // error = "Rule must not have nested tags";
        // delete result;
        // return nullptr;
        xmlReader.skipCurrentElement();
        xmlReader.readNextStartElement();
    }

    return result;
}

QList<RulePtr> loadRules(QXmlStreamReader& xmlReader, QString& error) {
    QList<RulePtr> rules;

    while (xmlReader.readNextStartElement()) {
        AbstractRule* rule = loadRule(xmlReader, error);
        if ( ! error.isNull()) {
            break;
        }
        rules.append(RulePtr(rule));

    }

    return rules;
}

Context* loadContext(QXmlStreamReader& xmlReader, QString& error) {
    QXmlStreamAttributes attrs = xmlReader.attributes();

    QString name = getRequiredAttribute(attrs, "name", error);
    if ( ! error.isNull()) {
        return nullptr;
    }

    QString attribute = getRequiredAttribute(attrs, "attribute", error).toLower();
    if ( ! error.isNull()) {
        return nullptr;
    }

    // TODO
    // if attribute != '<not set>':  // there are no attributes for internal contexts, used by rules. See perl.xml
    //     try:
    //         format = attributeToFormatMap[attribute]
    //     except KeyError:
    //         _logger.warning('Unknown context attribute %s', attribute)
    //         format = TextFormat()
    // else:
    //     format = None

    // textType = format.textType if format is not None else ' '
    // if formatConverterFunction is not None and format is not None:
    //     format = formatConverterFunction(format)

    QString lineEndContextText = getAttribute(attrs, "lineEndContext", "#stay");
    ContextSwitcher lineEndContext = makeContextSwitcher(lineEndContextText/*,  context.parser, formatConverterFunction*/);
    QString lineBeginContextText = getAttribute(attrs, "lineEndContext", "#stay");
    ContextSwitcher lineBeginContext = makeContextSwitcher(lineBeginContextText/*, context.parser, formatConverterFunction*/);

    bool fallthrough = parseBoolAttribute(getAttribute(attrs, "fallthrough", "false"), error);
    if ( ! error.isNull()) {
        error = QString("Failed to parse 'fallthrough': %1").arg(error);
        return nullptr;
    }

    ContextSwitcher fallthroughContext;

    if(fallthrough) {
        QString fallthroughContextText = getAttribute(attrs, "fallthroughContext", "#stay", true);
        fallthroughContext = makeContextSwitcher(fallthroughContextText/*, context.parser, formatConverterFunction*/);
    }

    bool dynamic = parseBoolAttribute(getAttribute(attrs, "dynamic", "false"), error);
    if ( ! error.isNull()) {
        error = QString("Failed to parse 'dynamic': %1").arg(error);
    }

    QList<RulePtr> rules = loadRules(xmlReader, error);
    if ( ! error.isNull()) {
        error = QString("Failed to parse context %1: %2").arg(name).arg(error);
        return nullptr;
    }

    // TODO add format, textType
    return new Context(name, attribute, lineEndContext, lineBeginContext, fallthroughContext, dynamic, rules);
}

Language* loadLanguage(QXmlStreamReader& xmlReader, QString& error) {
    if (! xmlReader.readNextStartElement()) {
        error = "Failed to read start element";
        return nullptr;
    }

    if (xmlReader.name() != "language") {
        error = "'name' attribute not found in <language>";
        return nullptr;
    }

    QXmlStreamAttributes attrs = xmlReader.attributes();

    QString name = getRequiredAttribute(attrs, "name", error);
    if ( ! error.isNull()) {
        return nullptr;
    }

    QString extensionsStr = getRequiredAttribute(attrs, "extensions", error);
    if ( ! error.isNull()) {
        return nullptr;
    }

    QString mimetypesStr = getAttribute(attrs, "mimetypes", QString::null);
    QString priorityStr = getAttribute(attrs, "priority", QString::null);
    QString hiddenStr = getAttribute(attrs, "hidden", "false");
    QString indenter = getAttribute(attrs, "indenter", QString::null);

    int priority = 0;
    if ( ! priorityStr.isNull()) {
        priority = parseIntAttribute(priorityStr, error);
        if ( ! error.isNull()) {
            error = QString("Bad integer priority value: %1").arg(error);
            return nullptr;
        }
    }

    bool hidden = parseBoolAttribute(hiddenStr, error);
    if ( ! error.isNull()) {
        error = QString("Failed to parse 'hidden' attribute: %1").arg(error);
        return nullptr;
    }

    QStringList extensions = extensionsStr.split(';', QString::SkipEmptyParts);
    QStringList mimetypes = extensionsStr.split(';', QString::SkipEmptyParts);

    if ( ! xmlReader.readNextStartElement() ||
         xmlReader.name() != "highlighting") {
        error = "<highlighting> tag not found";
        return nullptr;
    }

    while (xmlReader.readNextStartElement()) {
        if (xmlReader.name() == "list") {
            xmlReader.skipCurrentElement(); // TODO load list
        } else {
            break;
        }
    }

    if (xmlReader.name() != "contexts") {
        error = QString("<contexts> tag not found. Found <%1>").arg(xmlReader.name().toString());
        return nullptr;
    }

    QList<ContextPtr> contexts;
    while (xmlReader.readNextStartElement()) {
        if (xmlReader.name() != "context") {
            error = QString("Not expected tag when parsing contexts <%1>").arg(xmlReader.name().toString());
            return nullptr;
        }

        Context* ctx = loadContext(xmlReader, error);
        if (ctx == nullptr) {
            return nullptr;
        }
        contexts.append(ContextPtr(ctx));
    }

    Language* language = new Language(name, extensions, mimetypes,
                                      priority, hidden, indenter, contexts);
    return language;
}
