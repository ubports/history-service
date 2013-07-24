# QML plugin

set(plugin_SRCS
    historyeventmodel.cpp
    historyqmlfilter.cpp
    historyqmlintersectionfilter.cpp
    historyqmlplugin.cpp
    historyqmlsort.cpp
    historyqmlunionfilter.cpp
    historythreadmodel.cpp
    sortproxymodel.cpp
    )

set(plugin_HDRS
    historyeventmodel.h
    historyqmlfilter.h
    historyqmlintersectionfilter.h
    historyqmlplugin.h
    historyqmlsort.h
    historyqmlunionfilter.h
    historythreadmodel.h
    sortproxymodel.h
)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    )

add_library(history-qml SHARED ${plugin_SRCS} ${plugin_HDRS})

qt5_use_modules(history-qml Core Qml Quick)

target_link_libraries(history-qml
    historyservice
    )

set(PLUGIN_DIR ${QT_INSTALL_QML}/Ubuntu/History)
install(TARGETS history-qml DESTINATION ${PLUGIN_DIR})
install(FILES qmldir DESTINATION ${PLUGIN_DIR})
