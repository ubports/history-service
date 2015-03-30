set(library_SRCS
    channelobserver.cpp
    telepathyhelper.cpp
    )

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
    )

add_library(historyprivate STATIC ${library_SRCS} ${library_HDRS})
target_link_libraries(historyprivate ${TP_QT5_LIBRARIES})
qt5_use_modules(historyprivate Contacts Core DBus)
