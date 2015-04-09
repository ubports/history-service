set(library_SRCS
    channelobserver.cpp
    contactmatcher.cpp
    telepathyhelper.cpp
    utils.cpp
    )

set(library_HDRS
    channelobserver_p.h
    contactmatcher_p.h
    telepathyhelper_p.h
    utils_p.h
    )

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
    )

add_library(historyprivate STATIC ${library_SRCS} ${library_HDRS})
target_link_libraries(historyprivate ${TP_QT5_LIBRARIES})
qt5_use_modules(historyprivate Contacts Core DBus)
