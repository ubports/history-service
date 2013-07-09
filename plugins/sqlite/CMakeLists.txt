# SQLite Plugin

set(plugin_SRCS
    sqlitedatabase.cpp
    sqlitehistorythreadview.cpp
    sqlitehistoryplugin.cpp
    sqlitehistoryreader.cpp
    sqlitehistorywriter.cpp
    )

set (plugin_HDRS
    sqlitedatabase.h
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

target_link_libraries(sqlitehistoryplugin commhistory)
#target_link_libraries(sqlite
#    ${TP_QT5_LIBRARIES}
#    ${TPL_QT5_LIBRARIES}
#    ${GLIB_LIBRARIES}
#    phoneapp
#    )

install(TARGETS sqlitehistoryplugin DESTINATION ${HISTORY_PLUGIN_PATH})
