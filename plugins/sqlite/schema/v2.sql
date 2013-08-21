DROP TRIGGER voice_events_delete_trigger;
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
END;
