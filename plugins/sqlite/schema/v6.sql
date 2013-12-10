DROP TRIGGER text_events_insert_trigger;
DROP TRIGGER text_events_update_trigger;
DROP TRIGGER text_events_delete_trigger;

ALTER TABLE text_events RENAME TO text_events_old;

CREATE TABLE text_events (
    accountId varchar(255),
    threadId varchar(255),
    eventId varchar(255),
    senderId varchar(255),
    timestamp datetime,
    newEvent bool,
    message varchar(512),
    messageType tinyint,
    messageStatus tinyint,
    readTimestamp datetime,
    subject varchar(256)
);

INSERT INTO text_events (
    accountId,
    threadId,
    eventId,
    senderId,
    timestamp,
    newEvent,
    message,
    messageType,
    messageStatus,
    readTimestamp,
    subject)
SELECT accountId,
       threadId,
       eventId,
       senderId,
       timestamp,
       newEvent,
       message,
       messageType,
       messageFlags,
       readTimestamp,
       subject
FROM text_events_old;

DROP TABLE text_events_old;

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
    DELETE from text_event_attachments WHERE
        accountId=old.accountId AND
        threadId=old.threadId AND
        eventId=old.eventId;
END;
