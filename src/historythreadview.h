#ifndef HISTORYTHREADVIEW_H
#define HISTORYTHREADVIEW_H

#include <Types>
#include <QObject>
#include <QScopedPointer>

class HistoryThreadViewPrivate;

class HistoryThreadView : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(HistoryThreadView)
public:
    virtual ~HistoryThreadView();

    virtual QList<HistoryThreadPtr> nextPage() = 0;
    virtual bool isValid() = 0;

protected:
    explicit HistoryThreadView(HistoryThreadViewPrivate &p);

private:
    QScopedPointer<HistoryThreadViewPrivate> d_ptr;
};

#endif // HISTORYTHREADVIEW_H
