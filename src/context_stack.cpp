#include <QDebug>

#include "context_switcher.h"
#include "context.h"

#include "context_stack.h"

// FIXME avoid data where possible


ContextStackItem::ContextStackItem():
    context(nullptr),
    data(nullptr)
{}

ContextStackItem::ContextStackItem(const Context* context, const void* data):
    context(context),
    data(data)
{}

bool ContextStackItem::operator==(const ContextStackItem& other) const {
    return context == other.context && data == other.data;
}

ContextStack::ContextStack(Context* context)
{
    items.append(ContextStackItem(context, nullptr));
}

ContextStack::ContextStack(const QVector<ContextStackItem>& items):
    items(items)
{}

bool ContextStack::operator==(const ContextStack& other) const{
    return items == other.items;
}

const Context* ContextStack::currentContext() {
    return items.last().context;
}

const void* ContextStack::currentData() {
    return items.last().data;
}

ContextStack ContextStack::switchContext(const ContextSwitcher& operation, const void* data) const{
    auto newItems = items;

    if (operation.popsCount() > 0) {
        if (newItems.size() - 1 < operation.popsCount()) {
            qWarning() << "#pop value is too big " << newItems.size() << operation.popsCount();

            if (newItems.size() > 1) {
                newItems = newItems.mid(0, 1);
            }
        }
        newItems = newItems.mid(0, newItems.size() - operation.popsCount());
    }

    if ( ! operation.context().isNull() ) {
        if ( ! operation.context()->dynamic()) {
            data = nullptr;
        }

        newItems.append(ContextStackItem(operation.context().data(), data));
    }

    return ContextStack(newItems);
}
