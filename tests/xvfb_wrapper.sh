LOG=xvfb.err
echo Running in virtual frame buffer...
/usr/bin/xvfb-run "-e" "$LOG" "-a" "-s" "-screen 0 640x480x24" "/usr/bin/dbus-test-runner" "--keep-env" "--dbus-config=$@/tests/common/dbus-session.conf" "--max-wait=30" "--task" "$@/daemon/history-daemon" "--ignore-return" "--task-name" "history-daemon" "--task" "$@/tests/Ubuntu.History/HistoryGroupedThreadsModelTest" "-p" "-o" "-p" "-,txt" "-p" "-o" "-p" "$@/test_HistoryGroupedThreadsModelTest.xml,xunitxml" "--wait-for" "com.canonical.HistoryService" "--task-name" "HistoryGroupedThreadsModelTest" 2>$LOG
RETVAL=$?
if [ $RETVAL -eq 0 ]; then
    echo $@ finished successfully...
else
    echo $@ in virtual frame buffer failed...
    cat $LOG >&2
    echo Tail of xvfb-run output:
    tail $LOG >&2
    exit $RETVAL
fi
