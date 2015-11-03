project(history-service)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# Standard install paths
include(GNUInstallDirs)

# Check for include files
include(CheckIncludeFileCXX)
include(CheckIncludeFile)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Contacts)
find_package(Qt5DBus)
find_package(Qt5Qml)
find_package(Qt5Quick)
find_package(Qt5Test)
find_package(LibPhoneNumber REQUIRED)

include(qt5)

if(NOT CMAKE_CROSSCOMPILING)
    find_program(QMAKE_EXECUTABLE qmake)
    if(QMAKE_EXECUTABLE STREQUAL "QMAKE_EXECUTABLE-NOTFOUND")
        message(FATAL_ERROR "qmake not found")
    endif()
    execute_process(
        COMMAND ${QMAKE_EXECUTABLE} -query QT_INSTALL_QML
        RESULT_VARIABLE RESULT
        OUTPUT_VARIABLE QT_INSTALL_QML
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to determine QT_INSTALL_QML from qmake")
    endif()
else()
    # qmake isn't multi-arch aware as it installs arch-specific mkspec files
    # in to /usr/share, so we can't use it here (we'd need a qmake binary
    # for the host arch using data for the target arch)
    set(QT_INSTALL_QML "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(TP_QT5 REQUIRED TelepathyQt5)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)
pkg_check_modules(QTGLIB QtGLib-2.0)

find_program(DBUS_RUNNER dbus-test-runner)

add_definitions(-DQT_NO_KEYWORDS)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

set(HISTORY_PLUGIN_PATH ${CMAKE_INSTALL_LIBDIR}/history-service/plugins)
configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

# generate a macro to make it easier to enable coverage support on targets
function(ENABLE_COVERAGE)
    get_directory_property(COVERAGE_TARGETS DIRECTORY ${CMAKE_SOURCE_DIR} COVERAGE_TARGETS)
    list(APPEND COVERAGE_TARGETS ${ARGN})
    MESSAGE(STATUS "Enabling coverage report for target(s): ${ARGN}")
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY COVERAGE_TARGETS ${COVERAGE_TARGETS})
endfunction()

enable_testing()

add_definitions(-std=c++11)

# Define the version to be used in the library
set(HISTORY_VERSION_MAJOR 0)
set(HISTORY_VERSION_MINOR 0)
set(HISTORY_VERSION_PATCH 0)
set(PACKAGE_VERSION ${HISTORY_VERSION_MAJOR}.${HISTORY_VERSION_MINOR}.${HISTORY_VERSION_PATCH})

add_subdirectory(src)
add_subdirectory(daemon)
add_subdirectory(plugins)
add_subdirectory(tools)
add_subdirectory(Ubuntu)
add_subdirectory(tests)

include(EnableCoverageReport)
#####################################################################
# Enable code coverage calculation with gcov/gcovr/lcov
# Usage:
#  * Switch build type to coverage (use ccmake or cmake-gui)
#  * Invoke make, make test, make coverage
#  * Find html report in subdir coveragereport
#  * Find xml report feasible for jenkins in coverage.xml
#####################################################################
IF(CMAKE_BUILD_TYPE MATCHES [cC][oO][vV][eE][rR][aA][gG][eE])
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -coverage" )
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -coverage" )
  GET_DIRECTORY_PROPERTY(COVERAGE_TARGETS DIRECTORY ${CMAKE_SOURCE_DIR} COVERAGE_TARGETS)
  ENABLE_COVERAGE_REPORT(TARGETS ${COVERAGE_TARGETS})
ENDIF(CMAKE_BUILD_TYPE MATCHES [cC][oO][vV][eE][rR][aA][gG][eE])


