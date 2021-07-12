# QML plugin

set(plugin_SRCS
    historyeventmodel.cpp
    historygroupedeventsmodel.cpp
    historygroupedthreadsmodel.cpp
    historymodel.cpp
    historyqmlfilter.cpp
    historyqmlintersectionfilter.cpp
    historyqmlplugin.cpp
    historyqmlsort.cpp
    historyqmltexteventattachment.cpp
    historyqmlunionfilter.cpp
    historythreadmodel.cpp
    historymanager.cpp
    )

set(plugin_HDRS
    historyeventmodel.h
    historygroupedeventsmodel.h
    historygroupedthreadsmodel.h
    historymodel.h
    historyqmlfilter.h
    historyqmlintersectionfilter.h
    historyqmlplugin.h
    historyqmlsort.h
    historyqmltexteventattachment.h
    historyqmlunionfilter.h
    historythreadmodel.h
    historymanager.h
)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    )

add_library(history-qml MODULE ${plugin_SRCS} ${plugin_HDRS})

qt5_use_modules(history-qml Contacts Core Qml Quick)

target_link_libraries(history-qml
    historyservice
    )

set(PLUGIN_DIR ${QT_INSTALL_QML}/Ubuntu/History)
install(TARGETS history-qml DESTINATION ${PLUGIN_DIR})
install(FILES qmldir DESTINATION ${PLUGIN_DIR})
