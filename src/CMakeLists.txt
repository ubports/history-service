set(library_SRCS
    historyitem.cpp
    textitem.cpp
    )

set(library_HDRS
    #historyfilter.h
    #historyintersectionfilter.h
    HistoryItem
    historyitem.h
    #historymanager.h
    #historyplugin.h
    #historyreader.h
    #historysort.h
    #historythread.h
    #historyunionfilter.h
    #historywriter.h
    TextItem
    textitem.h
    types.h
    #voiceitem.h
)

set(library_PRIV_HDRS
    historyitem_p.h
    textitem_p.h
)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_library(history STATIC ${library_SRCS} ${library_HDRS} ${library_PRIV_HDRS})

qt5_use_modules(history Core Qml Quick)

