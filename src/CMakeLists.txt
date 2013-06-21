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

configure_file(rsb/transport/spread/rsbspreadexports.h.in
               "${CMAKE_CURRENT_BINARY_DIR}/rsb/transport/spread/rsbspreadexports.h"
               @ONLY)

include_directories(BEFORE SYSTEM ${CMAKE_CURRENT_BINARY_DIR}
                                  ${RSC_INCLUDE_DIRS}
                                  ${RSB_INCLUDE_DIRS}
                                  ${Boost_INCLUDE_DIRS}
                                  ${PROTOBUF_INCLUDE_DIRS}
                                  ${SPREAD_INCLUDE_DIRS})

set(SOURCES rsb/Plugin.cpp

            rsb/transport/spread/SpreadMessage.cpp
            rsb/transport/spread/SpreadGroup.cpp
            rsb/transport/spread/SpreadConnection.cpp

            rsb/transport/spread/MembershipManager.cpp
            rsb/transport/spread/Assembly.cpp
            rsb/transport/spread/MessageHandler.cpp
            rsb/transport/spread/ReceiverTask.cpp

            rsb/transport/spread/SpreadConnector.cpp
            rsb/transport/spread/InPushConnector.cpp
            rsb/transport/spread/InPullConnector.cpp
            rsb/transport/spread/OutConnector.cpp

            rsb/transport/spread/registration.cpp)

set(HEADERS rsb/transport/spread/SpreadMessage.h
            rsb/transport/spread/SpreadGroup.h
            rsb/transport/spread/SpreadConnection.h

            rsb/transport/spread/MembershipManager.h
            rsb/transport/spread/Assembly.h
            rsb/transport/spread/MessageHandler.h
            rsb/transport/spread/ReceiverTask.h

            rsb/transport/spread/SpreadConnector.h
            rsb/transport/spread/InPushConnector.h
            rsb/transport/spread/InPullConnector.h
            rsb/transport/spread/OutConnector.h

            rsb/transport/spread/registration.h
            
            ${CMAKE_CURRENT_BINARY_DIR}/rsb/transport/spread/rsbspreadexports.h)

add_library(${RSBSPREAD_NAME} SHARED ${SOURCES} ${HEADERS})
target_link_libraries(${RSBSPREAD_NAME} ${RSC_LIBRARIES}
                                        ${RSB_LIBRARIES}
                                        ${SPREAD_LIBRARIES}
                                        ${Boost_LIBRARIES})

set_target_properties(${RSBSPREAD_NAME}
                      PROPERTIES
                      VERSION ${SO_VERSION})

install(TARGETS ${RSBSPREAD_NAME}
        EXPORT rsb-spread-depends
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib/rsb${VERSION_SUFFIX}/plugins NAMELINK_SKIP
        ARCHIVE DESTINATION lib)
install_files_recursive("include/${INSTALL_PATH_PREFIX}" HEADERS)
