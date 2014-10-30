set(makevoiceevents_SRCS main.cpp)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    )

add_executable(history-makevoiceevents ${makevoiceevents_SRCS})
qt5_use_modules(history-makevoiceevents Core)

target_link_libraries(history-makevoiceevents historyservice)
install(TARGETS history-makevoiceevents RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
