ALTER TABLE text_events ADD COLUMN informationType integer;
UPDATE text_events SET informationType = 1;
