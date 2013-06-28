CREATE TABLE threads (
    accountId varchar(255),
    threadId varchar(255),
    type tinyint,
    lastItemId varchar(255),
    count int, unreadCount int
)#

CREATE TABLE thread_participants (
    accountId varchar(255),
    threadId varchar(255),
    type tinyint,
    participantId varchar(255)
)#

CREATE TABLE voice_items (
    accountId varchar(255),
    threadId varchar(255),
    itemId varchar(255),
    senderId varchar(255),
    timestamp datetime,
    newItem bool,
    duration int,
    missed bool
)#

CREATE TABLE text_items (
    accountId varchar(255),
    threadId varchar(255),
    itemId varchar(255),
    senderId varchar(255),
    timestamp datetime,
    newItem bool,
    message varchar(512),
    messageType tinyint,
    messageFlags tinyint,
    readTimestamp datetime
)#

CREATE TRIGGER voice_items_insert_trigger  AFTER INSERT ON voice_items
FOR EACH ROW
BEGIN
    UPDATE threads SET count=(SELECT count(itemId) FROM voice_items WHERE
        accountId=new.accountId AND
        threadId=new.threadId)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=1;
    UPDATE threads SET unreadCount=(SELECT count(itemId) FROM voice_items WHERE
        accountId=new.accountId AND threadId=new.threadId AND newItem=1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=1;
    UPDATE threads SET lastItemId=(SELECT itemId FROM voice_items WHERE
        accountId=new.accountId AND
        threadId=new.threadId
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=1;
END#

CREATE TRIGGER voice_items_update_trigger  AFTER UPDATE ON voice_items
FOR EACH ROW
BEGIN
    UPDATE threads SET count=(SELECT count(itemId) FROM voice_items WHERE
        accountId=new.accountId AND
        threadId=new.threadId)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=1;
    UPDATE threads SET unreadCount=(SELECT count(itemId) FROM voice_items WHERE
        accountId=new.accountId AND threadId=new.threadId AND newItem=1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=1;
    UPDATE threads SET lastItemId=(SELECT itemId FROM voice_items WHERE
        accountId=new.accountId AND
        threadId=new.threadId
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=1;
END#

CREATE TRIGGER voice_items_delete_trigger  AFTER DELETE ON voice_items
FOR EACH ROW
BEGIN
    UPDATE threads SET count=(SELECT count(itemId) FROM voice_items WHERE
        accountId=new.accountId AND
        threadId=new.threadId)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=1;
    UPDATE threads SET unreadCount=(SELECT count(itemId) FROM voice_items WHERE
        accountId=new.accountId AND threadId=new.threadId AND newItem=1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=1;
    UPDATE threads SET lastItemId=(SELECT itemId FROM voice_items WHERE
        accountId=new.accountId AND
        threadId=new.threadId
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=1;
END#

CREATE TRIGGER text_items_insert_trigger  AFTER INSERT ON text_items
FOR EACH ROW
BEGIN
    UPDATE threads SET count=(SELECT count(itemId) FROM text_items WHERE
        accountId=new.accountId AND
        threadId=new.threadId)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
    UPDATE threads SET unreadCount=(SELECT count(itemId) FROM text_items WHERE
        accountId=new.accountId AND threadId=new.threadId AND newItem=1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
    UPDATE threads SET lastItemId=(SELECT itemId FROM text_items WHERE
        accountId=new.accountId AND
        threadId=new.threadId
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
END#

CREATE TRIGGER text_items_update_trigger  AFTER UPDATE ON text_items
FOR EACH ROW
BEGIN
    UPDATE threads SET count=(SELECT count(itemId) FROM text_items WHERE
        accountId=new.accountId AND
        threadId=new.threadId)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
    UPDATE threads SET unreadCount=(SELECT count(itemId) FROM text_items WHERE
        accountId=new.accountId AND threadId=new.threadId AND newItem=1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
    UPDATE threads SET lastItemId=(SELECT itemId FROM text_items WHERE
        accountId=new.accountId AND
        threadId=new.threadId
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
END#

CREATE TRIGGER text_items_delete_trigger  AFTER DELETE ON text_items
FOR EACH ROW
BEGIN
    UPDATE threads SET count=(SELECT count(itemId) FROM text_items WHERE
        accountId=new.accountId AND
        threadId=new.threadId)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
    UPDATE threads SET unreadCount=(SELECT count(itemId) FROM text_items WHERE
        accountId=new.accountId AND threadId=new.threadId AND newItem=1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
    UPDATE threads SET lastItemId=(SELECT itemId FROM text_items WHERE
        accountId=new.accountId AND
        threadId=new.threadId
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
END#

