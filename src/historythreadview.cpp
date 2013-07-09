#include "historythreadview.h"
#include "historythreadview_p.h"

HistoryThreadViewPrivate::HistoryThreadViewPrivate(HistoryItem::ItemType theType,
                                                   const HistorySort &theSort,
                                                   const HistoryFilter &theFilter)
    : type(theType), sort(theSort), filter(theFilter)
{
}

HistoryThreadViewPrivate::~HistoryThreadViewPrivate()
{
}

HistoryThreadView::HistoryThreadView(HistoryThreadViewPrivate &p)
    : d_ptr(&p)
{
}

HistoryThreadView::~HistoryThreadView()
{
}
