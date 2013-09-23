DELETE FROM thread_participants WHERE (SELECT count(threadId) FROM threads WHERE threads.threadId=thread_participants.threadId)=0;
