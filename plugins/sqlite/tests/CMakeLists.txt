include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

macro(generate_tests)
    foreach(test ${ARGN})
        add_executable(${test} ${test}.cpp)
        qt5_use_modules(${test} Core DBus Test Sql)
        target_link_libraries(${test}
            historyservice
            sqlitehistoryplugin
            )
        add_test(${test} ${CMAKE_CURRENT_BINARY_DIR}/${test} -xunitxml -o ${CMAKE_BINARY_DIR}/test_${test}.xml)
        set(TEST_ENVIRONMENT "HISTORY_SQLITE_DBPATH=:memory:")
        set_tests_properties(${test} PROPERTIES
                             ENVIRONMENT ${TEST_ENVIRONMENT})
    endforeach(test)
endmacro(generate_tests)

generate_tests(
    SqlitePluginTest
    SqliteThreadViewTest
    )
