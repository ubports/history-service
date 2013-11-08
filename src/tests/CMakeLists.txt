include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    )

macro(generate_tests)
    foreach(test ${ARGN})
        add_executable(${test} ${test}.cpp)
        qt5_use_modules(${test} Core DBus Test)
        target_link_libraries(${test}
            historyservice
            )
        add_test(${test} ${CMAKE_CURRENT_BINARY_DIR}/${test} -xunitxml -o ${CMAKE_BINARY_DIR}/test_${test}.xml)
    endforeach(test)
endmacro(generate_tests)

generate_tests(
    FilterTest
    IntersectionFilterTest
    PhoneUtilsTest
    SortTest
    ThreadTest
    TextEventTest
    TextEventAttachmentTest
    UnionFilterTest
    VoiceEventTest
    )
