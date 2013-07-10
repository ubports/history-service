#ifndef HISTORYITEMVIEW_H
#define HISTORYITEMVIEW_H

#include <Types>

class HistoryItemView
{
public:
    HistoryItemView() {}
    virtual ~HistoryItemView() {}

    virtual QList<HistoryItemPtr> nextPage() = 0;
    virtual bool isValid() const = 0;
};

#endif // HISTORYITEMVIEW_H
