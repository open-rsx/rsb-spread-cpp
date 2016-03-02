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

# Global CMake Settings

cmake_minimum_required(VERSION 2.6)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

# Global Project Settings

project(rsb-spread)

set(VERSION_MAJOR "0" CACHE STRING "Major project version part")
set(VERSION_MINOR "14" CACHE STRING "Minor project version part")
set(VERSION_SUFFIX "${VERSION_MAJOR}.${VERSION_MINOR}")
set(SO_VERSION     "${VERSION_MAJOR}.${VERSION_MINOR}")

set(RSBSPREAD_NAME rsbspread) # stem of shared object name

set(TEST_NAME rsbspread-test)

set(OUTPUT_PATH            "${CMAKE_BINARY_DIR}/build")
set(ARCHIVE_OUTPUT_PATH    ${OUTPUT_PATH})
set(LIBRARY_OUTPUT_PATH    ${OUTPUT_PATH})
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_PATH})

# Configuration options

option(BUILD_TESTS "Build tests?" ON)

# Dependencies

find_package(RSC "${VERSION_SUFFIX}" REQUIRED)
message(STATUS "RSC version:              ${RSC_VERSION}")
message(STATUS "RSC libraries:            ${RSC_LIBRARIES}")
message(STATUS "RSC runtime library dirs: ${RSC_RUNTIME_LIBRARY_DIRS}")
message(STATUS "RSC CMake module path:    ${RSC_CMAKE_MODULE_PATH}")

list(INSERT CMAKE_MODULE_PATH 0 ${RSC_CMAKE_MODULE_PATH})

include(DefineProjectVersion)
include(CurrentDate)
include(PedanticCompilerWarnings)
include(InstallFilesRecursive)

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS OFF)
add_definitions(-DBOOST_ALL_DYN_LINK)
find_package(Boost 1.38 REQUIRED regex date_time program_options system)
if(NOT RSC_INTERNAL_BOOST_UUID)
    find_package(BoostUUID REQUIRED)
endif()

find_package(RSB "${VERSION_SUFFIX}" REQUIRED)
message(STATUS "RSB version:              ${RSB_VERSION}")
message(STATUS "RSB libraries:            ${RSB_LIBRARIES}")
message(STATUS "RSB runtime library dirs: ${RSB_RUNTIME_LIBRARY_DIRS}")

# Determine real project version using RSC macros

define_project_version(RSBSPREAD_ ${VERSION_MAJOR} ${VERSION_MINOR} 0 "archive")
current_date(RSBSPREAD_BUILD_DATE)
set(INSTALL_PATH_PREFIX "${RSBSPREAD_NAME}${VERSION_SUFFIX}"
    CACHE STRING "Prefix path applied to all non-versioned installed files in order to prevent version clashes.")

# Spread

find_package(Spread 4.0 REQUIRED)
if(NOT SPREAD_EXECUTABLE)
    message(STATUS "Spread executable not found. Unit tests will not be built!")
endif()

# Protobuf
# This is required because we are now reusing the protobuf-generated protocol

find_package(ProtocolBuffers REQUIRED)

# Compilation settings

add_definitions(${RSB_PROTOCOL_CFLAGS})

include_directories(BEFORE SYSTEM ${RSB_INCLUDE_DIRS}
                                  ${RSC_INCLUDE_DIRS}
                                  ${PROTOBUF_INCLUDE_DIRS}
                                  ${Boost_INCLUDE_DIRS})
link_directories(${RSC_LIBRARY_DIRS}
                 ${RSB_LIBRARY_DIRS})

set(CMAKE_INSTALL_RPATH "\$ORIGIN/../..:\$ORIGIN/"
    CACHE STRING "Default install RPATH.")

# Sub-directories

add_subdirectory(src)

# Tests

enable_testing()
if(BUILD_TESTS)
    include(ProvideGoogleMock)
    if(GMOCK_AVAILABLE)
        add_subdirectory(test)
    else()
        message(WARNING "Could not build unit tests (as requested) because Google Mock could not be installed.")
    endif()
endif()

# pkgconfig file

configure_file(rsbspread.pc.in
               "rsbspread${VERSION_SUFFIX}.pc"
               @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/rsbspread${VERSION_SUFFIX}.pc"
        DESTINATION lib/pkgconfig)

# CMake config files

configure_file(rsb-spread-buildtree-settings.cmake.in
               "${CMAKE_BINARY_DIR}/rsb-spread-buildtree-settings.cmake"
               @ONLY)

configure_file(rsb-spread-config.cmake.in
               ${CMAKE_BINARY_DIR}/rsb-spread-config.cmake
               @ONLY)
configure_file(rsb-spread-config-version.cmake.in
               ${CMAKE_BINARY_DIR}/rsb-spread-config-version.cmake
               @ONLY)
install(FILES "${CMAKE_BINARY_DIR}/rsb-spread-config.cmake"
              "${CMAKE_BINARY_DIR}/rsb-spread-config-version.cmake"
        DESTINATION "share/${INSTALL_PATH_PREFIX}")

export(TARGETS ${RSBSPREAD_NAME}
       FILE "${CMAKE_BINARY_DIR}/rsb-spread-depends.cmake")
install(EXPORT rsb-spread-depends
        DESTINATION "share/${INSTALL_PATH_PREFIX}")

# Packaging

set(PACKAGE_BASE_NAME           "librsbspread")
set(CPACK_PACKAGE_VERSION_MAJOR ${RSBSPREAD_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${RSBSPREAD_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${RSBSPREAD_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION       "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_VENDOR        "CoR-Lab Bielefeld University")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.txt")

include(ProvideFlexibleCPack)
