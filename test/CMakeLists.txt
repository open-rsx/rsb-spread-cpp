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

# Setup generic test infrastructure

set(TEST_RESULT_DIR ${CMAKE_BINARY_DIR}/testresults)
set(TEST_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(TEST_SPREAD_PORT 4816 CACHE STRING
    "The port of the spread daemon for tests")
set(TEST_SPREAD_CONFIG_SYMLINK "" CACHE STRING
    "A symlink pointing to the generated test spread config file. If not empty it will be created and used for the unit tests. This is required to prevent a size limitation bug for the argument in the spread daemon.")
set(SPREAD_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/spread.conf")

set(REAL_SPREAD_CONFIG_FILE ${SPREAD_CONFIG_FILE})
if(TEST_SPREAD_CONFIG_SYMLINK)
    set(REAL_SPREAD_CONFIG_FILE ${TEST_SPREAD_CONFIG_SYMLINK})
endif()

configure_file(spread.conf.in
               ${SPREAD_CONFIG_FILE}
               @ONLY)
configure_file(testconfig.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h)

# Compilation settings for test code

include_directories(BEFORE "${CMAKE_SOURCE_DIR}/src"
                           "${CMAKE_CURRENT_BINARY_DIR}/../src"
                           ${GMOCK_INCLUDE_DIRS}
                           ${CMAKE_CURRENT_SOURCE_DIR}  # test support files
                           ${CMAKE_CURRENT_BINARY_DIR}) # generated files

# Spread connector test

if(SPREAD_EXECUTABLE)
    set(TEST_SOURCES main.cpp

                     rsb/InformerTask.cpp

                     rsb/transport/ConnectorTest.cpp

                     rsb/transport/spread/AssemblyTest.cpp
                     rsb/transport/spread/SpreadConnectionTest.cpp
                     rsb/transport/spread/SpreadConnectorTest.cpp
                     rsb/transport/spread/SpreadMessageTest.cpp
                     rsb/transport/spread/MembershipManagerTest.cpp)

    add_executable(${TEST_NAME} ${TEST_SOURCES})
    target_link_libraries(${TEST_NAME}
                          ${RSB_LIBRARIES}
                          ${RSBSPREAD_NAME}
                          ${GMOCK_LIBRARIES})

    add_test(${TEST_NAME}
             ${EXECUTABLE_OUTPUT_PATH}/${TEST_NAME}
             "--gtest_output=xml:${TEST_RESULT_DIR}/")
endif()

# Spread symlink workaround

if(TEST_SPREAD_CONFIG_SYMLINK)
    add_custom_command(TARGET ${TEST_NAME} PRE_BUILD
                       COMMAND ${CMAKE_COMMAND} -E remove ${TEST_SPREAD_CONFIG_SYMLINK}
                       COMMAND ${CMAKE_COMMAND} -E create_symlink ${SPREAD_CONFIG_FILE} ${TEST_SPREAD_CONFIG_SYMLINK}
                       COMMENT "Creating spread test config symlink")
endif()
