include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    )

macro(generate_test TESTNAME USE_DBUS)
    add_executable(${TESTNAME} ${TESTNAME}.cpp)
    qt5_use_modules(${TESTNAME} Core DBus Test)
    target_link_libraries(${TESTNAME}
        historyservice
        )
    if (${USE_DBUS})
        add_test(${TESTNAME} ${DBUS_RUNNER} --keep-env
                                            --task ${CMAKE_BINARY_DIR}/daemon/history-daemon --ignore-return
                                            --task ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME} -p -xunitxml -p -o -p ${CMAKE_BINARY_DIR}/test_${TESTNAME}.xml
                                            --wait-for=com.canonical.HistoryService)
    else (${USE_DBUS})
        add_test(${TESTNAME} ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME} -xunitxml -o ${CMAKE_BINARY_DIR}/test_${TESTNAME}.xml)
    endif(${USE_DBUS})
    set(TEST_ENVIRONMENT "HOME=/tmp/history_test_home\;HISTORY_SQLITE_DBPATH=:memory:\;HISTORY_PLUGIN_PATH=${CMAKE_BINARY_DIR}/plugins/sqlite")
    set_tests_properties(${TESTNAME} PROPERTIES
                          ENVIRONMENT ${TEST_ENVIRONMENT}
                          TIMEOUT 60)
endmacro(generate_test)

generate_test(FilterTest False)
generate_test(IntersectionFilterTest False)
generate_test(PhoneUtilsTest False)
generate_test(SortTest False)
generate_test(ThreadTest False)
generate_test(TextEventTest False)
generate_test(TextEventAttachmentTest False)
generate_test(UnionFilterTest False)
generate_test(VoiceEventTest False)

if (DBUS_RUNNER)
    generate_test(ManagerTest True)
endif (DBUS_RUNNER)
