# SQLite Plugin

set(plugin_SRCS
    sqlitedatabase.cpp
    sqlitehistoryplugin.cpp
    sqlitehistorywriter.cpp
    )

set (plugin_HDRS
    sqlitedatabase.h
    sqlitehistoryplugin.h
    sqlitehistorywriter.h
)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    )

add_library(sqlitehistoryplugin SHARED ${plugin_SRCS} ${plugin_HDRS})

qt5_use_modules(sqlitehistoryplugin Core Sql)

#target_link_libraries(sqlite
#    ${TP_QT5_LIBRARIES}
#    ${TPL_QT5_LIBRARIES}
#    ${GLIB_LIBRARIES}
#    phoneapp
#    )

install(TARGETS sqlitehistoryplugin DESTINATION ${HISTORY_PLUGIN_PATH})
