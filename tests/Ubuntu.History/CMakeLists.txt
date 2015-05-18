include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/Ubuntu/History
    ${CMAKE_SOURCE_DIR}/src
    )

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/Ubuntu/History)
generate_test(HistoryGroupedThreadsModelTest
              SOURCES HistoryGroupedThreadsModelTest.cpp
              LIBRARIES history-qml
              QT5_MODULES Core DBus Qml Test)

