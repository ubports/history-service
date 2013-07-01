set(reader_SRCS main.cpp)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    )

add_executable(history-reader ${reader_SRCS})
qt5_use_modules(history-reader Core)

target_link_libraries(history-reader commhistory)
