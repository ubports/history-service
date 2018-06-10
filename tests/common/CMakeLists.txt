include_directories(${TP_QT5_INCLUDE_DIRS}
                    ${CMAKE_SOURCE_DIR}/src)

configure_file(dbus-session.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dbus-session.conf)

add_library(mockcontroller STATIC mockcontroller.cpp mockcontroller.h)
target_link_libraries(mockcontroller Qt5::Core Qt5::DBus)

add_library(telepathytest STATIC telepathytest.cpp telepathytest.h)
target_link_libraries(telepathytest Qt5::Core Qt5::DBus)
target_link_libraries(telepathytest ${TP_QT5_LIBRARIES})

add_subdirectory(mock)
add_subdirectory(dbus-services)
