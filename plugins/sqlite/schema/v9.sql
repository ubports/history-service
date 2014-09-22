DROP TRIGGER text_events_insert_trigger;
DROP TRIGGER text_events_update_trigger;
DROP TRIGGER text_events_delete_trigger;

CREATE TRIGGER text_events_insert_trigger  AFTER INSERT ON text_events
FOR EACH ROW
BEGIN
    UPDATE threads SET count=(SELECT count(eventId) FROM text_events WHERE
        accountId=new.accountId AND
        threadId=new.threadId AND
        messageType!=2)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
    UPDATE threads SET unreadCount=(SELECT count(eventId) FROM text_events WHERE
        accountId=new.accountId AND threadId=new.threadId AND newEvent='1' AND messageType!=2)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
    UPDATE threads SET lastEventId=(SELECT eventId FROM text_events WHERE
        accountId=new.accountId AND
        threadId=new.threadId AND
        messageType!=2
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
    UPDATE threads SET lastEventTimestamp=(SELECT timestamp FROM text_events WHERE
        accountId=new.accountId AND
        threadId=new.threadId AND
        messageType!=2
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
END;
CREATE TRIGGER text_events_update_trigger  AFTER UPDATE ON text_events
FOR EACH ROW
BEGIN
    UPDATE threads SET count=(SELECT count(eventId) FROM text_events WHERE
        accountId=new.accountId AND
        threadId=new.threadId AND
        messageType!=2)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
    UPDATE threads SET unreadCount=(SELECT count(eventId) FROM text_events WHERE
        accountId=new.accountId AND threadId=new.threadId AND newEvent='1' AND messageType!=2)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
    UPDATE threads SET lastEventId=(SELECT eventId FROM text_events WHERE
        accountId=new.accountId AND
        threadId=new.threadId AND
        messageType!=2)
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
    UPDATE threads SET lastEventTimestamp=(SELECT timestamp FROM text_events WHERE
        accountId=new.accountId AND
        threadId=new.threadId AND
        messageType!=2
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=new.accountId AND threadId=new.threadId AND type=0;
END;
CREATE TRIGGER text_events_delete_trigger  AFTER DELETE ON text_events
FOR EACH ROW
BEGIN
    UPDATE threads SET count=(SELECT count(eventId) FROM text_events WHERE
        accountId=old.accountId AND
        threadId=old.threadId AND
        messageType!=2)
        WHERE accountId=old.accountId AND threadId=old.threadId AND type=0;
    UPDATE threads SET unreadCount=(SELECT count(eventId) FROM text_events WHERE
        accountId=old.accountId AND threadId=old.threadId AND newEvent='1' AND messageType!=2)
        WHERE accountId=old.accountId AND threadId=old.threadId AND type=0;
    UPDATE threads SET lastEventId=(SELECT eventId FROM text_events WHERE
        accountId=old.accountId AND
        threadId=old.threadId AND
        messageType!=2
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=old.accountId AND threadId=old.threadId AND type=0;
    UPDATE threads SET lastEventTimestamp=(SELECT timestamp FROM text_events WHERE
        accountId=old.accountId AND
        threadId=old.threadId AND
        messageType!=2
        ORDER BY timestamp DESC LIMIT 1)
        WHERE accountId=old.accountId AND threadId=old.threadId AND type=0;
    DELETE from text_event_attachments WHERE
        accountId=old.accountId AND
        threadId=old.threadId AND
        eventId=old.eventId;
END;

