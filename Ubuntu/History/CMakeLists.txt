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
    )

set(plugin_HDRS
    historyeventmodel.h
    historygroupedeventsmodel.h
    historygroupedthreadsmodel.h
    historymodel.cpp
    historyqmlfilter.h
    historyqmlintersectionfilter.h
    historyqmlplugin.h
    historyqmlsort.h
    historyqmltexteventattachment.h
    historyqmlunionfilter.h
    historythreadmodel.h
)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/historyprivate
    )

add_library(history-qml SHARED ${plugin_SRCS} ${plugin_HDRS})

qt5_use_modules(history-qml Contacts Core Qml Quick)

target_link_libraries(history-qml
    historyservice
    historyprivate
    )

set(PLUGIN_DIR ${QT_INSTALL_QML}/Ubuntu/History)
install(TARGETS history-qml DESTINATION ${PLUGIN_DIR})
install(FILES qmldir DESTINATION ${PLUGIN_DIR})
