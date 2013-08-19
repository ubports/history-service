
CREATE TABLE schema_version (
    version int
);
CREATE TABLE threads (
    accountId varchar(255),
    threadId varchar(255),
    type tinyint,
    lastEventId varchar(255),
    lastEventTimestamp datetime,
    count int,
    unreadCount int
);
CREATE TABLE thread_participants (
    accountId varchar(255),
    threadId varchar(255),
    type tinyint,
    participantId varchar(255)
);
CREATE TABLE voice_events (
    accountId varchar(255),
    threadId varchar(255),
    eventId varchar(255),
    senderId varchar(255),
    timestamp datetime,
    newEvent bool,
    duration int,
    missed bool
);
CREATE TABLE text_events (
    accountId varchar(255),
    threadId varchar(255),
    eventId varchar(255),
    senderId varchar(255),
    timestamp datetime,
    newEvent bool,
    message varchar(512),
    messageType tinyint,
    messageFlags tinyint,
    readTimestamp datetime
);
CREATE TRIGGER voice_events_insert_trigger  AFTER INSERT ON voice_events
FOR EACH ROW
BEGIN
    UPDATE threads SET count=(SELECT count(eventId) FROM voice_events WHERE
        accountId=new.accountId AND
        threadId=new.threadId)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=1;
    UPDATE threads SET unreadCount=(SELECT count(eventId) FROM voice_events WHERE
        accountId=new.accountId AND threadId=new.threadId AND newEvent='true')
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=1;
    UPDATE threads SET lastEventId=(SELECT eventId FROM voice_events WHERE
        accountId=new.accountId AND
        threadId=new.threadId
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=1;
    UPDATE threads SET lastEventTimestamp=(SELECT timestamp FROM voice_events WHERE
        accountId=new.accountId AND
        threadId=new.threadId
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=1;
END;
CREATE TRIGGER voice_events_update_trigger  AFTER UPDATE ON voice_events
FOR EACH ROW
BEGIN
    UPDATE threads SET count=(SELECT count(eventId) FROM voice_events WHERE
        accountId=new.accountId AND
        threadId=new.threadId)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=1;
    UPDATE threads SET unreadCount=(SELECT count(eventId) FROM voice_events WHERE
        accountId=new.accountId AND threadId=new.threadId AND newEvent='true')
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=1;
    UPDATE threads SET lastEventId=(SELECT eventId FROM voice_events WHERE
        accountId=new.accountId AND
        threadId=new.threadId
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=1;
    UPDATE threads SET lastEventTimestamp=(SELECT timestamp FROM voice_events WHERE
        accountId=new.accountId AND
        threadId=new.threadId
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=1;
END;
CREATE TRIGGER text_events_insert_trigger  AFTER INSERT ON text_events
FOR EACH ROW
BEGIN
    UPDATE threads SET count=(SELECT count(eventId) FROM text_events WHERE
        accountId=new.accountId AND
        threadId=new.threadId)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
    UPDATE threads SET unreadCount=(SELECT count(eventId) FROM text_events WHERE
        accountId=new.accountId AND threadId=new.threadId AND newEvent='true')
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
    UPDATE threads SET lastEventId=(SELECT eventId FROM text_events WHERE
        accountId=new.accountId AND
        threadId=new.threadId
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
    UPDATE threads SET lastEventTimestamp=(SELECT timestamp FROM text_events WHERE
        accountId=new.accountId AND
        threadId=new.threadId
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
END;
CREATE TRIGGER text_events_update_trigger  AFTER UPDATE ON text_events
FOR EACH ROW
BEGIN
    UPDATE threads SET count=(SELECT count(eventId) FROM text_events WHERE
        accountId=new.accountId AND
        threadId=new.threadId)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
    UPDATE threads SET unreadCount=(SELECT count(eventId) FROM text_events WHERE
        accountId=new.accountId AND threadId=new.threadId AND newEvent='true')
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
    UPDATE threads SET lastEventId=(SELECT eventId FROM text_events WHERE
        accountId=new.accountId AND
        threadId=new.threadId
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
    UPDATE threads SET lastEventTimestamp=(SELECT timestamp FROM text_events WHERE
        accountId=new.accountId AND
        threadId=new.threadId
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
END;
CREATE TRIGGER voice_events_delete_trigger  AFTER DELETE ON voice_events
FOR EACH ROW
BEGIN
    UPDATE threads SET count=(SELECT count(eventId) FROM voice_events WHERE
        accountId=old.accountId AND
        threadId=old.threadId)
        WHERE accountId=old.accountId AND threadId=old.threadId AND type=1;
    UPDATE threads SET unreadCount=(SELECT count(eventId) FROM voice_events WHERE
        accountId=old.accountId AND threadId=old.threadId AND newEvent='true')
        WHERE accountId=old.accountId AND threadId=old.threadId AND type=1;
    UPDATE threads SET lastEventId=(SELECT eventId FROM voice_events WHERE
        accountId=old.accountId AND
        threadId=old.threadId
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=old.accountId AND threadId=old.threadId AND type=1;
    UPDATE threads SET lastEventTimestamp=(SELECT timestamp FROM voice_events WHERE
        accountId=old.accountId AND
        threadId=old.threadId
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=old.accountId AND threadId=old.threadId AND type=1;
END;
CREATE TRIGGER text_events_delete_trigger  AFTER DELETE ON text_events
FOR EACH ROW
BEGIN
    UPDATE threads SET count=(SELECT count(eventId) FROM text_events WHERE
        accountId=old.accountId AND
        threadId=old.threadId)
        WHERE accountId=old.accountId AND threadId=old.threadId AND type=0;
    UPDATE threads SET unreadCount=(SELECT count(eventId) FROM text_events WHERE
        accountId=old.accountId AND threadId=old.threadId AND newEvent='true')
        WHERE accountId=old.accountId AND threadId=old.threadId AND type=0;
    UPDATE threads SET lastEventId=(SELECT eventId FROM text_events WHERE
        accountId=old.accountId AND
        threadId=old.threadId
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=old.accountId AND threadId=old.threadId AND type=0;
    UPDATE threads SET lastEventTimestamp=(SELECT timestamp FROM text_events WHERE
        accountId=old.accountId AND
        threadId=old.threadId
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=old.accountId AND threadId=old.threadId AND type=0;
END;
