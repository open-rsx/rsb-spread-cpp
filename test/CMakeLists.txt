# This file is part of the rsb-spread project.
#
# Copyright (C) 2013 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
#
# This file may be licensed under the terms of the
# GNU Lesser General Public License Version 3 (the ``LGPL''),
# or (at your option) any later version.
#
# Software distributed under the License is distributed
# on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
# express or implied. See the LGPL for the specific language
# governing rights and limitations.
#
# You should have received a copy of the LGPL along with this
# program. If not, go to http://www.gnu.org/licenses/lgpl.html
# or write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# The development of this software was supported by:
#   CoR-Lab, Research Institute for Cognition and Robotics
#     Bielefeld University

enable_testing()

include_directories(BEFORE ../src
                           ${CMAKE_CURRENT_BINARY_DIR}
                           ${CMAKE_CURRENT_BINARY_DIR}/rsb/protocol
                           ${CMAKE_CURRENT_BINARY_DIR}/../src
                           ${GMOCK_INCLUDE_DIRS}
                           ${CMAKE_CURRENT_BINARY_DIR}
                           ${CMAKE_CURRENT_SOURCE_DIR})

# Setup generic test infrastructure

set(TEST_RESULT_DIR ${CMAKE_BINARY_DIR}/testresults)
set(TEST_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(TEST_SPREAD_PORT 4816 CACHE STRING "the port of the spread daemon for tests")
set(TEST_SPREAD_CONFIG_SYMLINK "" CACHE STRING "A symlink pointing to the generated test spread config file. If not empty it will be created and used for the unit tests. This is required to prevent a size limitation bug for the argument in the spread daemon.")
set(SPREAD_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/spread.conf")

set(REAL_SPREAD_CONFIG_FILE ${SPREAD_CONFIG_FILE})
if(TEST_SPREAD_CONFIG_SYMLINK)
    set(REAL_SPREAD_CONFIG_FILE ${TEST_SPREAD_CONFIG_SYMLINK})
endif()

configure_file(spread.conf.in ${SPREAD_CONFIG_FILE} @ONLY)
configure_file(testconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h)

set(AVAILABLE_TESTS)

# Connector test library

set(CONNECTOR_TEST_NAME "connectortest")
add_library(${CONNECTOR_TEST_NAME} STATIC rsb/transport/ConnectorTest.cpp
                                          rsb/transport/ConnectorTest.h
                                          rsb/InformerTask.cpp
                                          rsb/InformerTask.h)
target_link_libraries(${CONNECTOR_TEST_NAME} ${RSB_NAME}
                                             ${GMOCK_LIBRARIES})

# Spread connector test

if(SPREAD_EXECUTABLE)
    set(SPREADCONNECTOR_TEST_SOURCES rsbtest_spread.cpp
                                     rsb/transport/spread/AssemblyTest.cpp
                                     rsb/transport/spread/SpreadConnectionTest.cpp
                                     rsb/transport/spread/SpreadConnectorTest.cpp
                                     rsb/transport/spread/SpreadMessageTest.cpp
                                     rsb/transport/spread/MembershipManagerTest.cpp)

    add_executable(${SPREADCONNECTOR_TEST_NAME} ${SPREADCONNECTOR_TEST_SOURCES})
    target_link_libraries(${SPREADCONNECTOR_TEST_NAME}
                          ${CONNECTOR_TEST_NAME})

    add_test(${SPREADCONNECTOR_TEST_NAME}
             ${EXECUTABLE_OUTPUT_PATH}/${SPREADCONNECTOR_TEST_NAME} "--gtest_output=xml:${TEST_RESULT_DIR}/")
    list(APPEND AVAILABLE_TESTS ${SPREADCONNECTOR_TEST_NAME})
endif()

# Spread symlink workaround

if(TEST_SPREAD_CONFIG_SYMLINK)

    add_custom_command(TARGET ${RSB_TEST_NAME} PRE_BUILD
                       COMMAND ${CMAKE_COMMAND} -E remove ${TEST_SPREAD_CONFIG_SYMLINK}
                       COMMAND ${CMAKE_COMMAND} -E create_symlink ${SPREAD_CONFIG_FILE} ${TEST_SPREAD_CONFIG_SYMLINK}
                       COMMENT "Creating spread test config symlink")
    add_custom_command(TARGET ${SPREADCONNECTOR_TEST_NAME} PRE_BUILD
                       COMMAND ${CMAKE_COMMAND} -E remove ${TEST_SPREAD_CONFIG_SYMLINK}
                       COMMAND ${CMAKE_COMMAND} -E create_symlink ${SPREAD_CONFIG_FILE} ${TEST_SPREAD_CONFIG_SYMLINK}
                       COMMENT "Creating spread test config symlink")

endif()
