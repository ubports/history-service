include_directories(${TP_QT5_INCLUDE_DIRS})
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5DBus_INCLUDE_DIRS})

find_library(TELEPATHY_QT5_SERVICE_LIBRARIES telepathy-qt5-service)

add_executable(telepathy-mock main.cpp protocol.cpp connection.cpp textchannel.cpp callchannel.cpp)
qt5_use_modules(telepathy-mock Core DBus)
target_link_libraries(telepathy-mock ${TP_QT5_LIBRARIES} ${TELEPATHY_QT5_SERVICE_LIBRARIES} ${OFONO_QT_LIBRARIES} ${PULSEAUDIO_LIBRARIES})
