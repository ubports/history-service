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
    ${SQLITE3_INCLUDE_DIRS}
    )

qt5_add_resources(plugin_RES sqlitehistoryplugin.qrc)

add_library(sqlitehistoryplugin SHARED ${plugin_SRCS} ${plugin_HDRS} ${plugin_RES})
qt5_use_modules(sqlitehistoryplugin Core Sql)

# update the .qrc file automatically when there are new schema files
file(GLOB QRC_RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/schema/*.sql ${CMAKE_CURRENT_SOURCE_DIR}/schema/*.info)
set(QRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/sqlitehistoryplugin.qrc)
add_custom_command(
    OUTPUT ${QRC_FILE}
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/update_qrc.sh ${QRC_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${QRC_RESOURCE_FILES}
    )
add_custom_target(qrc_update DEPENDS ${QRC_FILE} schema_update)

add_dependencies(sqlitehistoryplugin schema_update qrc_update)
target_link_libraries(sqlitehistoryplugin historyservice ${SQLITE3_LIBRARIES})
install(TARGETS sqlitehistoryplugin DESTINATION ${HISTORY_PLUGIN_PATH})

add_subdirectory(schema)
