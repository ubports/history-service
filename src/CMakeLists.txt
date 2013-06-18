set(library_SRCS
    )

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    )


add_library(history STATIC ${library_SRCS})

qt5_use_modules(history Core Qml Quick)

