ALTER TABLE voice_events ADD COLUMN remoteParticipant varchar(255);
UPDATE voice_events SET remoteParticipant=(SELECT participantId FROM thread_participants
                                           WHERE thread_participants.accountId = voice_events.accountId
                                           AND thread_participants.threadId = voice_events.threadId
                                           AND thread_participants.type = 1
                                           LIMIT 1);
