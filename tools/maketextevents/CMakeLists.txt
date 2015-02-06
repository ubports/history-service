set(maketextevents_SRCS main.cpp)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    )

add_executable(history-maketextevents ${maketextevents_SRCS})
qt5_use_modules(history-maketextevents Core)

target_link_libraries(history-maketextevents historyservice)
install(TARGETS history-maketextevents RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
