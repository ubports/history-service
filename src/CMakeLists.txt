set(library_SRCS
    event.cpp
    filter.cpp
    intersectionfilter.cpp
    manager.cpp
    pluginmanager.cpp
    sort.cpp
    textevent.cpp
    thread.cpp
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
    PluginManager
    pluginmanager.h
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
    filter_p.h
    intersectionfilter_p.h
    manager_p.h
    sort_p.h
    textevent_p.h
    thread_p.h
    unionfilter_p.h
    voiceevent_p.h
)

include_directories(${CMAKE_SOURCE_DIR}/src)

# FIXME: set the library version and SONAME
add_library(historyservice SHARED ${library_SRCS} ${library_HDRS} ${library_PRIV_HDRS})

qt5_use_modules(historyservice Core)

install(TARGETS historyservice DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${library_HDRS} DESTINATION include/history-service/History)
