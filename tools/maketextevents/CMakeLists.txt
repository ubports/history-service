set(maketextevents_SRCS main.cpp)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    )

add_executable(history-maketextevents ${maketextevents_SRCS})
target_link_libraries(history-maketextevents Qt5::Core)

target_link_libraries(history-maketextevents historyservice)
install(TARGETS history-maketextevents RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
