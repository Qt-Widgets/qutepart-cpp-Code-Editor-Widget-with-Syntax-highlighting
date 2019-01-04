#pragma once


#include <QTextStream>
#include <QList>
#include <QSharedPointer>
#include <QTextBlock>

#include "context.h"


class Language {
public:
    Language(const QString& name,
             const QStringList& extensions,
             const QStringList& mimetypes,
             int priority,
             bool hidden,
             const QString& indenter,
             const QList<ContextPtr>& contexts);

    void printDescription(QTextStream& out) const;

    void highlightBlock(QTextBlock block, QVector<QTextLayout::FormatRange>& formats);

protected:
    QString name;
    QStringList extensions;
    QStringList mimetypes;
    int priority;
    bool hidden;
    QString indenter;

    QList<ContextPtr> contexts;
};
