set(library_SRCS
    event.cpp
    eventview.cpp
    filter.cpp
    intersectionfilter.cpp
    manager.cpp
    managerdbus.cpp
    pluginmanager.cpp
    sort.cpp
    textevent.cpp
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
    Reader
    reader.h
    Sort
    sort.h
    TextEvent
    textevent.h
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
    Writer
    writer.h
)

set(library_PRIV_HDRS
    event_p.h
    eventview_p.h
    filter_p.h
    intersectionfilter_p.h
    manager_p.h
    managerdbus_p.h
    pluginmanager_p.h
    sort_p.h
    textevent_p.h
    thread_p.h
    threadview_p.h
    unionfilter_p.h
    voiceevent_p.h
)

include_directories(${CMAKE_SOURCE_DIR}/src)

qt5_add_dbus_adaptor(library_SRCS HistoryService.xml managerdbus_p.h History::ManagerDBus)

add_library(historyservice SHARED ${library_SRCS} ${library_HDRS} ${library_PRIV_HDRS})

# Set the library version and the SOVERSION
set_target_properties(historyservice PROPERTIES
                      SOVERSION ${HISTORY_VERSION_MAJOR}
                      VERSION ${HISTORY_VERSION_MAJOR}.${HISTORY_VERSION_MINOR}.${HISTORY_VERSION_PATCH})

qt5_use_modules(historyservice Core DBus)

install(TARGETS historyservice DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${library_HDRS} DESTINATION include/history-service/History)
