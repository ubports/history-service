ALTER TABLE text_events add column subject varchar(256);

CREATE TABLE IF NOT EXISTS text_event_attachments (
    accountId varchar(255),
    threadId varchar(255),
    eventId varchar(255),
    attachmentId varchar(255),
    contentType varchar(255),
    filePath varchar(255),
    status tinyint
);

DROP TRIGGER text_events_delete_trigger;
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
