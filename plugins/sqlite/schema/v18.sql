CREATE TRIGGER IF NOT EXISTS text_threads_delete_trigger AFTER DELETE ON threads
FOR EACH ROW WHEN old.type=0
BEGIN
    DELETE FROM text_events WHERE
        accountId=old.accountId AND
        threadId=old.threadId;
END;
CREATE TRIGGER IF NOT EXISTS voice_threads_delete_trigger AFTER DELETE ON threads
FOR EACH ROW WHEN old.type=1
BEGIN
    DELETE FROM voice_events WHERE
        accountId=old.accountId AND
        threadId=old.threadId;
END;
