set(library_SRCS
    historyfilter.cpp
    historyintersectionfilter.cpp
    historyitem.cpp
    historymanager.cpp
    historysort.cpp
    historythread.cpp
    historythreadview.cpp
    historyunionfilter.cpp
    pluginmanager.cpp
    textitem.cpp
    voiceitem.cpp
    )

set(library_HDRS
    HistoryFilter
    historyfilter.h
    HistoryIntersectionFilter
    historyintersectionfilter.h
    HistoryItem
    historyitem.h
    HistoryManager
    historymanager.h
    HistoryPlugin
    historyplugin.h
    HistoryReader
    historyreader.h
    HistorySort
    historysort.h
    HistoryThread
    historythread.h
    HistoryThreadView
    historythreadview.h
    HistoryUnionFilter
    historyunionfilter.h
    HistoryWriter
    historywriter.h
    PluginManager
    pluginmanager.h
    TextItem
    textitem.h
    Types
    types.h
    VoiceItem
    voiceitem.h
)

set(library_PRIV_HDRS
    historyfilter_p.h
    historyintersectionfilter_p.h
    historyitem_p.h
    historymanager_p.h
    historysort_p.h
    historythread_p.h
    historythreadview_p.h
    historyunionfilter_p.h
    textitem_p.h
    voiceitem_p.h
)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_library(commhistory SHARED ${library_SRCS} ${library_HDRS} ${library_PRIV_HDRS})

qt5_use_modules(commhistory Core Qml Quick)

install(TARGETS commhistory DESTINATION ${CMAKE_INSTALL_LIBDIR})
