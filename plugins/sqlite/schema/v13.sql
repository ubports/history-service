ALTER TABLE threads ADD COLUMN chatType tinyint;
ALTER TABLE thread_participants ADD COLUMN alias varchar(255);
CREATE TABLE chat_room_info (
    accountId varchar(255),
    type tinyint,
    threadId varchar(255),
    roomName varchar(255),
    server varchar(255),
    creator varchar(255),
    creationTimestamp datetime,
    anonymous boolean,
    inviteOnly boolean,
    participantLimit integer,
    moderated boolean,
    title varchar(1024),
    description varchar(1024),
    persistent boolean,
    private boolean,
    passwordProtected boolean,
    password varchar(512),
    passwordHint varchar(512),
    canUpdateConfiguration boolean,
    subject varchar(1024),
    actor varchar(512),
    timestamp datetime
);
UPDATE threads SET chatType = 0;
UPDATE threads SET chatType=1 WHERE (SELECT COUNT(participantId) from thread_participants WHERE thread_participants.threadId=threads.threadId and thread_participants.accountId=threads.accountId AND thread_participants.type=threads.type)=1;

DROP TRIGGER threads_delete_trigger;
CREATE TRIGGER threads_delete_trigger AFTER DELETE ON threads
FOR EACH ROW
BEGIN
    DELETE FROM thread_participants WHERE
        accountId=old.accountId AND
        threadId=old.threadId AND
        type=old.type;
    DELETE FROM chat_room_info WHERE
        accountId=old.accountId AND
        threadId=old.threadId AND
        type=old.type;
END;
