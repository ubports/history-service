include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/historyprivate
    ${CMAKE_SOURCE_DIR}/tests/common
    ${TP_QT5_INCLUDE_DIRS}
)

generate_telepathy_test(ContactMatcherTest
                        SOURCES ContactMatcherTest.cpp
                        LIBRARIES historyprivate historyservice telepathytest
                        QT5_MODULES Core DBus Test Qml Contacts)
