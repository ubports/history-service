ALTER TABLE thread_participants ADD COLUMN normalizedId varchar(255);
UPDATE thread_participants SET normalizedId = normalizeId(accountId, participantId);
