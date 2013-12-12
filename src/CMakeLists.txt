set(library_SRCS
    event.cpp
    eventview.cpp
    filter.cpp
    intersectionfilter.cpp
    manager.cpp
    managerdbus.cpp
    phoneutils.cpp
    pluginthreadview.cpp
    plugineventview.cpp
    sort.cpp
    textevent.cpp
    texteventattachment.cpp
    thread.cpp
    threadview.cpp
    unionfilter.cpp
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
    event_p.h
    eventview_p.h
    filter_p.h
    intersectionfilter_p.h
    manager_p.h
    managerdbus_p.h
    phonenumberutils_p.h
    phoneutils_p.h
    pluginthreadview_p.h
    plugineventview_p.h
    sort_p.h
    textevent_p.h
    texteventattachment_p.h
    thread_p.h
    threadview_p.h
    unionfilter_p.h
    voiceevent_p.h
)

qt5_add_dbus_adaptor(library_SRCS PluginThreadView.xml pluginthreadview.h History::PluginThreadView)
qt5_add_dbus_adaptor(library_SRCS PluginEventView.xml plugineventview.h History::PluginEventView)

include_directories(${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_CURRENT_BINARY_DIR})

add_library(historyservice SHARED ${library_SRCS} ${library_HDRS} ${library_PRIV_HDRS})

# Set the library version and the SOVERSION
set_target_properties(historyservice PROPERTIES
                      SOVERSION ${HISTORY_VERSION_MAJOR}
                      VERSION ${HISTORY_VERSION_MAJOR}.${HISTORY_VERSION_MINOR}.${HISTORY_VERSION_PATCH})

qt5_use_modules(historyservice Core DBus)

install(TARGETS historyservice DESTINATION ${CMAKE_INSTALL_LIBDIR})

set(INCLUDE_INSTALL_DIR include/history-service)
install(FILES ${library_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/History)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/history-service.pc.in ${CMAKE_CURRENT_BINARY_DIR}/history-service.pc)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/history-service.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig COMPONENT pkgconfig)

#add_subdirectory(tests)
