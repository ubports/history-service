include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests/common
    ${CMAKE_CURRENT_BINARY_DIR}
    ${TP_QT5_INCLUDE_DIRS}
)

generate_test(FilterTest SOURCES FilterTest.cpp LIBRARIES historyservice)
generate_test(IntersectionFilterTest SOURCES IntersectionFilterTest.cpp LIBRARIES historyservice)
generate_test(ParticipantTest SOURCES ParticipantTest.cpp LIBRARIES historyservice)
generate_test(PhoneUtilsTest SOURCES PhoneUtilsTest.cpp LIBRARIES historyservice)
generate_test(SortTest SOURCES SortTest.cpp LIBRARIES historyservice)
generate_test(ThreadTest SOURCES ThreadTest.cpp LIBRARIES historyservice)
generate_test(TextEventTest SOURCES TextEventTest.cpp LIBRARIES historyservice)
generate_test(TextEventAttachmentTest SOURCES TextEventAttachmentTest.cpp LIBRARIES historyservice)
generate_test(UnionFilterTest SOURCES UnionFilterTest.cpp LIBRARIES historyservice)
generate_test(VoiceEventTest SOURCES VoiceEventTest.cpp LIBRARIES historyservice)

# DBus based tests
generate_test(ManagerTest
              SOURCES ManagerTest.cpp
              LIBRARIES historyservice
              USE_DBUS
              TASKS --task ${CMAKE_BINARY_DIR}/daemon/history-daemon --ignore-return --task-name history-daemon
              WAIT_FOR com.canonical.HistoryService)
generate_test(ThreadViewTest
              SOURCES ThreadViewTest.cpp
              LIBRARIES historyservice
              USE_DBUS
              TASKS --task ${CMAKE_BINARY_DIR}/daemon/history-daemon --ignore-return --task-name history-daemon
              WAIT_FOR com.canonical.HistoryService)
generate_test(EventViewTest
              SOURCES EventViewTest.cpp
              LIBRARIES historyservice
              USE_DBUS
              TASKS --task ${CMAKE_BINARY_DIR}/daemon/history-daemon --ignore-return --task-name history-daemon
              WAIT_FOR com.canonical.HistoryService)

# Telepathy-based tests
generate_telepathy_test(ContactMatcherTest
                        SOURCES ContactMatcherTest.cpp
                        QT5_MODULES Qt5::Core Qt5::DBus Qt5::Test Qt5::Qml Qt5::Contacts)
