execute_process(
    COMMAND dpkg-architecture -qDEB_HOST_MULTIARCH
    OUTPUT_VARIABLE DEB_HOST_MULTIARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

SET(ENV{LD_LIBRARY_PATH} "/usr/lib/${DEB_HOST_MULTIARCH}/mesa-egl:$ENV{LD_LIBRARY_PATH}")

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/Ubuntu/History
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests/common
    ${TP_QT5_INCLUDE_DIRS}
    )

find_program(XVFB_RUN_BIN
    NAMES xvfb-run
)

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/Ubuntu/History)
generate_test(HistoryGroupedThreadsModelTest
              SOURCES HistoryGroupedThreadsModelTest.cpp
              LIBRARIES history-qml
              USE_DBUS
              USE_XVFB
              TASKS --task ${CMAKE_BINARY_DIR}/daemon/history-daemon --ignore-return --task-name history-daemon
              WAIT_FOR com.canonical.HistoryService)
generate_telepathy_test(HistoryEventModelTest
                        SOURCES HistoryEventModelTest.cpp
                        LIBRARIES ${TP_QT5_LIBRARIES} mockcontroller telepathytest history-qml
                        USE_XVFB
                        TASKS --task ${CMAKE_BINARY_DIR}/daemon/history-daemon --ignore-return --task-name history-daemon
                        WAIT_FOR com.canonical.HistoryService)
