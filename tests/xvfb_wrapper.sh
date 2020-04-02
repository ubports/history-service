LOG=xvfb.err
echo Running in virtual frame buffer...
/usr/bin/xvfb-run "-e" "$LOG" "-a" "-s" "-screen 0 640x480x24" "/usr/bin/dbus-test-runner" "--keep-env" "--dbus-config=/home/lduboeuf/history-service/obj-arm-linux-gnueabihf/tests/common/dbus-session.conf" "--max-wait=5" "--task" "/home/lduboeuf/history-service/obj-arm-linux-gnueabihf/daemon/history-daemon" "--ignore-return" "--task-name" "history-daemon" "--task" "/home/lduboeuf/history-service/obj-arm-linux-gnueabihf/tests/Ubuntu.History/HistoryGroupedThreadsModelTest" "-p" "-o" "-p" "-,txt" "-p" "-o" "-p" "/home/lduboeuf/history-service/obj-arm-linux-gnueabihf/test_HistoryGroupedThreadsModelTest.xml,xunitxml" "--wait-for" "com.canonical.HistoryService" "--task-name" "HistoryGroupedThreadsModelTest" 2>$LOG
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
