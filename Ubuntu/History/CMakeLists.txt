# QML plugin

set(plugin_SRCS
    historyqmlfilter.cpp
    historyqmlintersectionfilter.cpp
    historyqmlplugin.cpp
    historyqmlunionfilter.cpp
    historythreadmodel.cpp
    )

set(plugin_HDRS
    historyqmlfilter.h
    historyqmlintersectionfilter.h
    historyqmlplugin.h
    historyqmlunionfilter.h
    historythreadmodel.h
)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    )

add_library(history-qml SHARED ${plugin_SRCS} ${plugin_HDRS})

qt5_use_modules(history-qml Core Qml Quick)

target_link_libraries(history-qml
    commhistory
    )

set(PLUGIN_DIR ${QT_INSTALL_QML}/Ubuntu/History)
install(TARGETS history-qml DESTINATION ${PLUGIN_DIR})
install(FILES qmldir DESTINATION ${PLUGIN_DIR})
