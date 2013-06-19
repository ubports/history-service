#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

class HistoryManager : public QObject
{
    Q_OBJECT
public:
    enum HistoryType {
        HistoryTypeVoice,
        HistoryTypeText
    }

    HistoryManager(const QString &backendPlugin = QString::null);
    ~HistoryManager();

    QList<HistoryThreadPtr> queryThreads(HistoryType type,
                                         const HistorySort &sort = HistoryDefaultSort(),  
                                         const HistoryFilter &filter = HistoryNoFilter(),
                                         int startOffset,
                                         int pageSize);

    QList<HistoryItemPtr> queryItems(HistoryType type,
                                     const HistorySort &sort = HistorySort(),
                                     const HistoryFilter &filter = HistoryFilter(),
                                     int startOffset,
                                     int pageSize);

    bool removeThreads(HistoryType type, const QList<QString> &threadIds);
    bool removeItems(HistoryType type, const QList<QString> &itemIds);
};

#endif

