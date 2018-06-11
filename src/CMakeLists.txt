set(library_SRCS
    channelobserver.cpp
    contactmatcher.cpp
    event.cpp
    eventview.cpp
    filter.cpp
    intersectionfilter.cpp
    manager.cpp
    managerdbus.cpp
    participant.cpp
    phoneutils.cpp
    pluginthreadview.cpp
    plugineventview.cpp
    sort.cpp
    telepathyhelper.cpp
    textevent.cpp
    texteventattachment.cpp
    thread.cpp
    threadview.cpp
    unionfilter.cpp
    utils.cpp
    voiceevent.cpp
    )

set(library_HDRS
    Event
    event.h
    EventView
    eventview.h
    Filter
    filter.h
    IntersectionFilter
    intersectionfilter.h
    Manager
    manager.h
    Participant
    participant.h
    Plugin
    plugin.h
    PluginThreadView
    pluginthreadview.h
    PluginEventView
    plugineventview.h
    Sort
    sort.h
    TextEvent
    textevent.h
    TextEventAttachment
    texteventattachment.h
    Thread
    thread.h
    ThreadView
    threadview.h
    Types
    types.h
    UnionFilter
    unionfilter.h
    VoiceEvent
    voiceevent.h
)

set(library_PRIV_HDRS
    channelobserver_p.h
    contactmatcher_p.h
    event_p.h
    eventview_p.h
    filter_p.h
    intersectionfilter_p.h
    manager_p.h
    managerdbus_p.h
    participant_p.h
    phoneutils_p.h
    pluginthreadview_p.h
    plugineventview_p.h
    sort_p.h
    telepathyhelper_p.h
    textevent_p.h
    texteventattachment_p.h
    thread_p.h
    threadview_p.h
    unionfilter_p.h
    utils_p.h
    voiceevent_p.h
)

qt5_add_dbus_adaptor(library_SRCS PluginThreadView.xml pluginthreadview.h History::PluginThreadView)
qt5_add_dbus_adaptor(library_SRCS PluginEventView.xml plugineventview.h History::PluginEventView)

include_directories(${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${LibPhoneNumber_INCLUDE_DIRS}
                    ${TP_QT5_INCLUDE_DIRS})

add_library(historyservice SHARED ${library_SRCS} ${library_HDRS} ${library_PRIV_HDRS})

# Set the library version and the SOVERSION
set_target_properties(historyservice PROPERTIES
                      SOVERSION ${HISTORY_VERSION_MAJOR}
                      VERSION ${HISTORY_VERSION_MAJOR}.${HISTORY_VERSION_MINOR}.${HISTORY_VERSION_PATCH})

target_link_libraries(historyservice ${LibPhoneNumber_LIBRARIES} ${TP_QT5_LIBRARIES})
target_link_libraries(historyservice Qt5::Contacts Qt5::Core Qt5::DBus)

install(TARGETS historyservice DESTINATION ${CMAKE_INSTALL_LIBDIR})

set(INCLUDE_INSTALL_DIR include/history-service)
install(FILES ${library_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/History)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/history-service.pc.in ${CMAKE_CURRENT_BINARY_DIR}/history-service.pc)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/history-service.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig COMPONENT pkgconfig)
