include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/Ubuntu/History
    ${CMAKE_SOURCE_DIR}/src
    )

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/Ubuntu/History)
generate_test(HistoryGroupedThreadsModelTest
              SOURCES HistoryGroupedThreadsModelTest.cpp
              LIBRARIES history-qml historyservice
              USE_DBUS
              TASKS --task ${CMAKE_BINARY_DIR}/daemon/history-daemon --ignore-return --task-name history-daemon
              WAIT_FOR com.canonical.HistoryService)
