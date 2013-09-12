
set(qt_SRCS
    callchannelobserver.cpp
    channelobserver.cpp
    historydaemon.cpp
    historyservicedbus.cpp
    pluginmanager.cpp
    telepathyhelper.cpp
    textchannelobserver.cpp
    )

set(daemon_SRCS main.cpp ${qt_SRCS})

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    )

qt5_add_dbus_adaptor(daemon_SRCS HistoryService.xml historyservicedbus.h HistoryServiceDBus)

add_executable(history-daemon ${daemon_SRCS} ${daemon_HDRS})
qt5_use_modules(history-daemon Core DBus)

target_link_libraries(history-daemon
    ${TP_QT5_LIBRARIES}
    historyservice
    )

# Handle i18n in desktop files
set(desktop_FILES history-daemon.desktop)

foreach(DESKTOP_FILE ${desktop_FILES})
    file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE})
    file(STRINGS ${DESKTOP_FILE}.in DESKTOP_FILE_CONTENTS)
    foreach(LINE ${DESKTOP_FILE_CONTENTS})
        string(REGEX REPLACE "tr\\\(\"(.*)\"\\\)" "\\1" LINE "${LINE}")
        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE} "${LINE}\n")
    endforeach(LINE)
endforeach(DESKTOP_FILE)

configure_file(org.freedesktop.Telepathy.Client.HistoryDaemonObserver.service.in org.freedesktop.Telepathy.Client.HistoryDaemonObserver.service)
install(TARGETS history-daemon RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.HistoryDaemonObserver.service DESTINATION share/dbus-1/services)
install(FILES HistoryDaemonObserver.client DESTINATION share/telepathy/clients)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/history-daemon.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
