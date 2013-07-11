# SQLite Plugin

set(plugin_SRCS
    sqlitedatabase.cpp
    sqlitehistoryeventview.cpp
    sqlitehistorythreadview.cpp
    sqlitehistoryplugin.cpp
    sqlitehistoryreader.cpp
    sqlitehistorywriter.cpp
    )

set (plugin_HDRS
    sqlitedatabase.h
    sqlitehistoryeventview.h
    sqlitehistorythreadview.h
    sqlitehistoryplugin.h
    sqlitehistoryreader.h
    sqlitehistorywriter.h
)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    )

qt5_add_resources(plugin_RES sqlitehistoryplugin.qrc)

add_library(sqlitehistoryplugin SHARED ${plugin_SRCS} ${plugin_HDRS} ${plugin_RES})
qt5_use_modules(sqlitehistoryplugin Core Sql)

target_link_libraries(sqlitehistoryplugin historyservice)
install(TARGETS sqlitehistoryplugin DESTINATION ${HISTORY_PLUGIN_PATH})
