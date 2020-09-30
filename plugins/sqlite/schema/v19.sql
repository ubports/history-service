ALTER TABLE text_events ADD COLUMN sentTime datetime;
UPDATE text_events SET sentTime = timestamp;
