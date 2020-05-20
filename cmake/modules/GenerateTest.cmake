#
# Copyright (C) 2015 Canonical, Ltd.
#
# Authors:
#  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
#
# This file is part of history-service.
#
# history-service is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 3.
#
# history-service is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

include(CMakeParseArguments)
find_program(DBUS_RUNNER dbus-test-runner)

set(
  XVFB_RUN_BIN
  ${CMAKE_SOURCE_DIR}/tests/xvfb-run.sh
)
#find_program(XVFB_RUN_BIN
#    NAMES xvfb-run
#)

function(generate_test TESTNAME)
    set(options USE_DBUS USE_UI USE_XVFB)
    set(oneValueArgs TIMEOUT WORKING_DIRECTORY QML_TEST WAIT_FOR)
    set(multiValueArgs TASKS LIBRARIES QT5_MODULES SOURCES ENVIRONMENT)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    MESSAGE(STATUS "Adding test: ${TESTNAME}")

    # set reasonable defaults for the arguments
    if (NOT DEFINED ARG_WORKING_DIRECTORY)
        set(ARG_WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    endif ()

    if (NOT DEFINED ARG_TIMEOUT)
        set(ARG_TIMEOUT 60)
    endif ()

    if (NOT DEFINED ARG_QT5_MODULES)
        set(ARG_QT5_MODULES Core Test)
    endif ()

    if (${ARG_USE_UI})
        if (${ARG_USE_DBUS})
            set(PLATFORM -p -platform -p offscreen)
        else ()
            set(PLATFORM -platform offscreen)
        endif ()
    endif()

    # Generate QML tests
    if (DEFINED ARG_QML_TEST)
        add_test(NAME ${TESTNAME}
            WORKING_DIRECTORY ${ARG_WORKING_DIRECTORY}
            COMMAND qmltestrunner -platform offscreen -import ${CMAKE_BINARY_DIR} -input ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_QML_TEST})
        set_tests_properties(${TESTNAME} PROPERTIES ENVIRONMENT "QT_QPA_FONTDIR=${CMAKE_BINARY_DIR}")
    else ()
        # For sanity checking, make sure DBUS_RUNNER is available for DBUS tests
        if (${ARG_USE_DBUS} AND "${DBUS_RUNNER}" STREQUAL "")
            message(WARNING "Test ${TESTNAME} disabled because dbus-test-runner was not found.")
            return()
        endif ()

        # No QML test, regular binary compiled test.
        add_executable(${TESTNAME} ${ARG_SOURCES})
        qt5_use_modules(${TESTNAME} ${ARG_QT5_MODULES})

        if (${ARG_USE_DBUS})
            execute_process(COMMAND mktemp -d OUTPUT_VARIABLE TMPDIR)
            string(REPLACE "\n" "" TMPDIR ${TMPDIR})

            if (NOT DEFINED ARG_ENVIRONMENT)
                set(ARG_ENVIRONMENT HOME=${TMPDIR}
                                    HISTORY_PLUGIN_PATH=${CMAKE_BINARY_DIR}/plugins/sqlite
                                    HISTORY_SQLITE_DBPATH=:memory:
                                    HISTORY_LOCK_FILE=${TMPDIR}/history-service.lock
                                    MC_ACCOUNT_DIR=${TMPDIR}
                                    MC_MANAGER_DIR=${TMPDIR})
            endif ()
            if (${ARG_USE_XVFB})
                SET(XVFB_RUN ${XVFB_RUN_BIN} -a -s "-screen 0 1024x768x24" -e "xvfberr.log")
            endif ()

            set(TEST_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME} ${PLATFORM} -p -o -p -,txt -p -o -p ${CMAKE_BINARY_DIR}/test_${TESTNAME}.xml,xunitxml)
            if (DEFINED ARG_WAIT_FOR)
                SET(TEST_COMMAND ${TEST_COMMAND} --wait-for ${ARG_WAIT_FOR})
            endif ()

            add_test(${TESTNAME} ${XVFB_RUN} ${DBUS_RUNNER} --keep-env --dbus-config=${CMAKE_BINARY_DIR}/tests/common/dbus-session.conf --max-wait=${ARG_TIMEOUT}
                                                ${ARG_TASKS} --task ${TEST_COMMAND} --task-name ${TESTNAME})
        else ()
            add_test(${TESTNAME} ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME} ${PLATFORM} -o -,txt -o ${CMAKE_BINARY_DIR}/test_${TESTNAME}.xml,xunitxml)
        endif()

        set_tests_properties(${TESTNAME} PROPERTIES
                             ENVIRONMENT "${ARG_ENVIRONMENT}"
                             TIMEOUT ${ARG_TIMEOUT})
        if (DEFINED ARG_LIBRARIES)
            target_link_libraries(${TESTNAME}
                                  ${ARG_LIBRARIES})
        endif ()

        enable_coverage(${TESTNAME})
    endif ()
endfunction(generate_test)

function(generate_telepathy_test TESTNAME)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs TASKS LIBRARIES QT5_MODULES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    set(TASKS --task gnome-keyring-daemon -p -r -p -d --task-name gnome-keyring --ignore-return
              --task /usr/lib/dconf/dconf-service --task-name dconf-service --ignore-return
              --task dconf -p write -p /org/gnome/empathy/use-conn -p false --task-name dconf-write --wait-for ca.desrt.dconf --ignore-return
              --task /usr/lib/telepathy/mission-control-5 --task-name mission-control --wait-for ca.desrt.dconf --ignore-return
              --task ${CMAKE_BINARY_DIR}/tests/common/mock/telepathy-mock --task-name telepathy-mock --wait-for org.freedesktop.Telepathy.MissionControl5 --ignore-return
              ${ARG_TASKS})

    if (NOT DEFINED ARG_LIBRARIES)
        set(ARG_LIBRARIES ${TP_QT5_LIBRARIES} historyservice mockcontroller telepathytest)
    endif(NOT DEFINED ARG_LIBRARIES)

    if (NOT DEFINED ARG_QT5_MODULES)
        set(ARG_QT5_MODULES Core DBus Test Qml)
    endif (NOT DEFINED ARG_QT5_MODULES)
    generate_test(${TESTNAME} ${ARGN}
                  TASKS ${TASKS}
                  LIBRARIES ${ARG_LIBRARIES} 
                  QT5_MODULES ${ARG_QT5_MODULES} 
                  USE_DBUS)
endfunction(generate_telepathy_test)
