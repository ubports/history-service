include_directories(
    ${CMAKE_SOURCE_DIR}/plugins/sqlite
    ${CMAKE_SOURCE_DIR}/src
    )

generate_test(SqlitePluginTest SOURCES SqlitePluginTest.cpp LIBRARIES historyservice sqlitehistoryplugin QT5_MODULES Core DBus Test Sql)
generate_test(SqliteThreadViewTest SOURCES SqliteThreadViewTest.cpp LIBRARIES historyservice sqlitehistoryplugin QT5_MODULES Core DBus Test Sql)
generate_test(SqliteEventViewTest SOURCES SqliteEventViewTest.cpp LIBRARIES historyservice sqlitehistoryplugin QT5_MODULES Core DBus Test Sql)
