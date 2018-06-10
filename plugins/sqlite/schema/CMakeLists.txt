file(GLOB SCHEMA_FILES ${CMAKE_CURRENT_SOURCE_DIR}/v*.sql)

set(SCHEMA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/schema.sql)
set(VERSION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/version.info)

configure_file(update_schema.sh.in ${CMAKE_CURRENT_BINARY_DIR}/update_schema.sh @ONLY)
add_custom_command(
    OUTPUT ${SCHEMA_FILE} ${VERSION_FILE}
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/update_schema.sh ${CMAKE_CURRENT_SOURCE_DIR} ${SCHEMA_FILE} ${VERSION_FILE}
    WORKING DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${SCHEMA_FILES} generate_schema
    )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
add_executable(generate_schema generate_schema.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../sqlitedatabase.cpp)
target_link_libraries(generate_schema Qt5::Core Qt5::DBus Qt5::Sql)
target_link_libraries(generate_schema historyservice ${SQLITE3_LIBRARIES})

add_custom_target(schema_update DEPENDS ${SCHEMA_FILE} ${VERSION_FILE})
