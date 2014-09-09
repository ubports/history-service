# QML plugin

set(plugin_SRCS
    contactmatcher.cpp
    historyeventmodel.cpp
    historygroupedeventsmodel.cpp
    historymodel.cpp
    historyqmlfilter.cpp
    historyqmlintersectionfilter.cpp
    historyqmlplugin.cpp
    historyqmlsort.cpp
    historyqmltexteventattachment.cpp
    historyqmlunionfilter.cpp
    historythreadgroupingproxymodel.cpp
    historythreadmodel.cpp
    sortproxymodel.cpp
    )

set(plugin_HDRS
    contactmatcher_p.h
    historyeventmodel.h
    historygroupedeventsmodel.h
    historymodel.cpp
    historyqmlfilter.h
    historyqmlintersectionfilter.h
    historyqmlplugin.h
    historyqmlsort.h
    historyqmltexteventattachment.h
    historyqmlunionfilter.h
    historythreadgroupingproxymodel.h
    historythreadmodel.h
    sortproxymodel.h
)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    )

add_library(history-qml SHARED ${plugin_SRCS} ${plugin_HDRS})

qt5_use_modules(history-qml Contacts Core Qml Quick)

target_link_libraries(history-qml
    historyservice
    )

set(PLUGIN_DIR ${QT_INSTALL_QML}/Ubuntu/History)
install(TARGETS history-qml DESTINATION ${PLUGIN_DIR})
install(FILES qmldir DESTINATION ${PLUGIN_DIR})
