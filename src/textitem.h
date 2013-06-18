class TextItem : HistoryItem
{
    enum MessageFlags 
    {
        Unread,
        Pending,
        Delivered
    }

    message: String
    messageType: enum
    messageFlags: int
    readTimestamp: DateTime
}

