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
    )

add_library(history-qml SHARED ${plugin_SRCS} ${plugin_HDRS})

target_link_libraries(history-qml Qt5::Contacts Qt5::Core Qt5::Qml Qt5::Quick)

target_link_libraries(history-qml
    historyservice
    )

set(PLUGIN_DIR ${QT_INSTALL_QML}/Ubuntu/History)
install(TARGETS history-qml DESTINATION ${PLUGIN_DIR})
install(FILES qmldir DESTINATION ${PLUGIN_DIR})
