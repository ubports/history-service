#ifndef HISTORYTHREADVIEW_P_H
#define HISTORYTHREADVIEW_P_H

#include <HistorySort>
#include <HistoryFilter>
#include <HistoryItem>

class HistoryThreadViewPrivate
{
public:
    HistoryThreadViewPrivate(HistoryItem::ItemType theType,
                             const HistorySort &theSort,
                             const HistoryFilter &theFilter);
    virtual ~HistoryThreadViewPrivate();

    HistoryItem::ItemType type;
    HistorySort sort;
    HistoryFilter filter;
};

#endif // HISTORYTHREADVIEW_P_H
