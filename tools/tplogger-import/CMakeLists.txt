set(tplogger_SRCS
    main.cpp
    telepathylogreader.cpp
    telepathylogimporter.cpp
    )

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${TPL_QT5_INCLUDE_DIRS}
    ${QTGLIB_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
    )

add_executable(history-tplogger-import ${tplogger_SRCS})
qt5_use_modules(history-tplogger-import Core DBus)

target_link_libraries(history-tplogger-import
    ${TP_QT5_LIBRARIES}
    ${TPL_QT5_LIBRARIES}
    ${QTGLIB_LIBRARIES}
    historyservice
    )

install(TARGETS history-tplogger-import RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
