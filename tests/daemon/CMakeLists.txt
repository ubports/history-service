include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests/common
    ${TP_QT5_INCLUDE_DIRS}
    )

generate_telepathy_test(DaemonTest
                        SOURCES DaemonTest.cpp handler.cpp approver.cpp
                        TASKS --task ${CMAKE_BINARY_DIR}/daemon/history-daemon --ignore-return --task-name history-daemon
                        WAIT_FOR com.canonical.HistoryService)
