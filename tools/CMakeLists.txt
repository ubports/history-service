add_subdirectory(reader)

if (${TPL_QT5_FOUND} AND ${QTGLIB_FOUND})
    add_subdirectory(tplogger-import)
endif (${TPL_QT5_FOUND} AND ${QTGLIB_FOUND})
