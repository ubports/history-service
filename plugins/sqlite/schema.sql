CREATE TABLE threads (
    accountId varchar(255),
    threadId varchar(255),
    type tinyint,
    lastItemId varchar(255),
    count int, unreadCount int
);

CREATE TABLE thread_participants (
    accountId varchar(255),
    threadId varchar(255),
    type tinyint,
    participantId varchar(255)
);

CREATE TABLE voice_items (
    accountId varchar(255),
    threadId varchar(255),
    itemId varchar(255),
    senderId varchar(255),
    timestamp datetime,
    duration datetime,
    missed bool
);

CREATE TABLE text_items (
    accountId varchar(255),
    threadId varchar(255),
    itemId varchar(255),
    senderId varchar(255),
    timestamp datetime,
    message varchar(512),
    messageType tinyint,
    messageFlags tinyint,
    readTimestamp datetime
);
