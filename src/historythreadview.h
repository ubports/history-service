#ifndef HISTORYTHREADVIEW_H
#define HISTORYTHREADVIEW_H

#include <Types>
#include <QScopedPointer>

class HistoryThreadViewPrivate;

class HistoryThreadView
{
public:
    HistoryThreadView() {}
    virtual ~HistoryThreadView() {}

    virtual QList<HistoryThreadPtr> nextPage() = 0;
    virtual bool isValid() const = 0;
};

#endif // HISTORYTHREADVIEW_H
