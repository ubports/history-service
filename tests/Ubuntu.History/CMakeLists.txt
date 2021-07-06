include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/Ubuntu/History
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests/common
    ${TP_QT5_INCLUDE_DIRS}
    )

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/Ubuntu/History)
set(HistoryQml_SOURCES
    ${SOURCE_DIR}/historyeventmodel.cpp
    ${SOURCE_DIR}/historyeventmodel.h
    ${SOURCE_DIR}/historymanager.h
    ${SOURCE_DIR}/historymanager.cpp
    ${SOURCE_DIR}/historygroupedeventsmodel.cpp
    ${SOURCE_DIR}/historygroupedeventsmodel.h
    ${SOURCE_DIR}/historygroupedthreadsmodel.cpp
    ${SOURCE_DIR}/historygroupedthreadsmodel.h
    ${SOURCE_DIR}/historymodel.cpp
    ${SOURCE_DIR}/historymodel.h
    ${SOURCE_DIR}/historyqmlfilter.cpp
    ${SOURCE_DIR}/historyqmlfilter.h
    ${SOURCE_DIR}/historyqmlintersectionfilter.cpp
    ${SOURCE_DIR}/historyqmlintersectionfilter.h
    ${SOURCE_DIR}/historyqmlplugin.cpp
    ${SOURCE_DIR}/historyqmlplugin.h
    ${SOURCE_DIR}/historyqmlsort.cpp
    ${SOURCE_DIR}/historyqmlsort.h
    ${SOURCE_DIR}/historyqmltexteventattachment.cpp
    ${SOURCE_DIR}/historyqmltexteventattachment.h
    ${SOURCE_DIR}/historyqmlunionfilter.cpp
    ${SOURCE_DIR}/historyqmlunionfilter.h
    ${SOURCE_DIR}/historythreadmodel.cpp
    ${SOURCE_DIR}/historythreadmodel.h
    )

set(HistoryGroupedThreadsModelTest_SOURCES
    ${HistoryQml_SOURCES}
    HistoryGroupedThreadsModelTest.cpp
    )
generate_test(HistoryGroupedThreadsModelTest
              SOURCES ${HistoryGroupedThreadsModelTest_SOURCES}
              LIBRARIES historyservice
              QT5_MODULES Core Qml Test
              USE_DBUS
              USE_XVFB
              TASKS --task ${CMAKE_BINARY_DIR}/daemon/history-daemon --ignore-return --task-name history-daemon
              WAIT_FOR com.canonical.HistoryService)
set(HistoryManagerTest_SOURCES
  ${HistoryQml_SOURCES}
  HistoryManagerTest.cpp
  )
generate_test(HistoryManagerTest
            SOURCES ${HistoryManagerTest_SOURCES}
            LIBRARIES historyservice
            QT5_MODULES Core Qml Test
            USE_DBUS
            USE_XVFB
            TASKS --task ${CMAKE_BINARY_DIR}/daemon/history-daemon --ignore-return --task-name history-daemon
            WAIT_FOR com.canonical.HistoryService)

set(HistoryEventModelTest_SOURCES
    ${HistoryQml_SOURCES}
    HistoryEventModelTest.cpp
    )
generate_telepathy_test(HistoryEventModelTest
                        SOURCES ${HistoryEventModelTest_SOURCES}
                        LIBRARIES ${TP_QT5_LIBRARIES} mockcontroller telepathytest historyservice
                        USE_XVFB
                        TASKS --task ${CMAKE_BINARY_DIR}/daemon/history-daemon --ignore-return --task-name history-daemon
                        WAIT_FOR com.canonical.HistoryService)
