include_directories(
    ${CMAKE_SOURCE_DIR}/plugins/sqlite
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    )

generate_test(SqlitePluginTest SOURCES SqlitePluginTest.cpp LIBRARIES historyservice sqlitehistoryplugin QT5_MODULES Core DBus Test Sql)
generate_test(SqliteThreadViewTest SOURCES SqliteThreadViewTest.cpp LIBRARIES historyservice sqlitehistoryplugin QT5_MODULES Core DBus Test Sql)
generate_test(SqliteEventViewTest SOURCES SqliteEventViewTest.cpp LIBRARIES historyservice sqlitehistoryplugin QT5_MODULES Core DBus Test Sql)
