CREATE TRIGGER threads_delete_trigger  AFTER DELETE ON threads
FOR EACH ROW
BEGIN
    DELETE from thread_participants WHERE
        accountId=old.accountId AND
        threadId=old.threadId AND
        type=old.type;
END;
