set(reader_SRCS main.cpp)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    )

add_executable(history-reader ${reader_SRCS})
target_link_libraries(history-reader Qt5::Core)

target_link_libraries(history-reader historyservice)
