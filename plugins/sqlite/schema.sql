CREATE TABLE threads (accountId varchar(255), threadId varchar(255), type tinyint, lastItemId varchar(255), count int, unreadCount int);
CREATE TABLE thread_participants (accountId varchar(255), threadId varchar(255), type tinyint, participantId varchar(255));
