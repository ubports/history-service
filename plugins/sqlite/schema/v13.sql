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
