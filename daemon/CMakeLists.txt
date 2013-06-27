
set(qt_SRCS
    callchannelobserver.cpp
    channelobserver.cpp
    historydaemon.cpp
    telepathyhelper.cpp
    textchannelobserver.cpp
    )

set(daemon_SRCS main.cpp ${qt_SRCS})

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    )

add_executable(history-daemon ${daemon_SRCS} ${daemon_HDRS})
qt5_use_modules(history-daemon Contacts Core DBus Gui Multimedia Qml)

if (USE_UBUNTU_PLATFORM_API)
    set(UBUNTU_APP_LIB "-lubuntu_application_api")
endif (USE_UBUNTU_PLATFORM_API)

target_link_libraries(history-daemon
    ${TP_QT5_LIBRARIES}
    history
    )

#configure_file(org.freedesktop.Telepathy.Client.PhoneAppApprover.service.in org.freedesktop.Telepathy.Client.PhoneAppApprover.service)
install(TARGETS history-daemon RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.PhoneAppApprover.service DESTINATION share/dbus-1/services)
#install(FILES PhoneAppApprover.client DESTINATION share/telepathy/clients)
