include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/Ubuntu/History
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests/common
    ${TP_QT5_INCLUDE_DIRS}
    )

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/Ubuntu/History)
generate_test(HistoryGroupedThreadsModelTest
              SOURCES HistoryGroupedThreadsModelTest.cpp
              LIBRARIES history-qml
              QT5_MODULES Core DBus Test Qml
              USE_XVFB
              TASKS --task ${CMAKE_BINARY_DIR}/daemon/history-daemon --ignore-return --task-name history-daemon
              WAIT_FOR com.canonical.HistoryService)
generate_telepathy_test(HistoryEventModelTest
                        SOURCES HistoryEventModelTest.cpp
                        LIBRARIES ${TP_QT5_LIBRARIES} mockcontroller telepathytest history-qml
                        USE_XVFB
                        TASKS --task ${CMAKE_BINARY_DIR}/daemon/history-daemon --ignore-return --task-name history-daemon
                        WAIT_FOR com.canonical.HistoryService)
