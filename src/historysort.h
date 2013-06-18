class HistorySort
{
public:
    HistorySort(const QString &sortField = "timestamp",
                Qt::SortOrder sortOrder = Qt::AscendingOrder,
                Qt::CaseSensitivity sortCase = Qt::CaseInsensitive);
    ~HistorySort();

    QString sortField();
    Qt::SortOrder sortOrder();
    Qt::CaseSensitivity sortCase();
};

