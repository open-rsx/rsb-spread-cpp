# To create a proper Debian/Ubuntu package, the following CMake
# options should be used:

#SET(CMAKE_BUILD_TYPE Release)
set(CPACK_STRIP_FILES "TRUE")

# Operating system checks

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "Cannot configure CPack to generate Debian packages on non-linux systems.")
endif()

include(CheckLSBTypes)
if((NOT LSB_DISTRIBUTOR_ID STREQUAL "ubuntu") AND (NOT LSB_DISTRIBUTOR_ID STREQUAL "debian"))
    message(FATAL_ERROR "Cannot configure CPack to generate Debian packages on something that is not Ubuntu or Debian.")
endif()

# Actual packaging options

set(CPACK_GENERATOR                   "DEB")

set(CPACK_DEBIAN_PACKAGE_NAME         "${PACKAGE_BASE_NAME}${VERSION_SUFFIX}")
set(CPACK_DEBIAN_PACKAGE_VERSION      "${CPACK_PACKAGE_VERSION}${CPACK_PACKAGE_REVISION}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER   "Jan Moringen <jmoringe@techfak.uni-bielefeld.de>")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION  "Spread Transport Plugin for RSB (C++ implementation)
 Shared library and plugin implementing a Spread-based transport for the
 Robotics Service Bus (RSB).")
set(CPACK_DEBIAN_PACKAGE_PRIORITY     "optional")
set(CPACK_DEBIAN_PACKAGE_SECTION      "libs")
#SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${CMAKE_SYSTEM_PROCESSOR}") Debian uses different names here
set(CPACK_DEBIAN_PACKAGE_DEPENDS      "libc6, librsc${VERSION_SUFFIX}, librsb${VERSION_SUFFIX}, spread (>= 4.0)")

set(CPACK_PACKAGE_FILE_NAME           "${CPACK_DEBIAN_PACKAGE_NAME}-${CPACK_DEBIAN_PACKAGE_VERSION}_${CMAKE_SYSTEM_PROCESSOR}")

# Write license file
file(WRITE "${CMAKE_BINARY_DIR}/copyright"
     "Copyright (C) 2010-2018 ${CPACK_DEBIAN_PACKAGE_MAINTAINER}

   This software may be licensed under the terms of the
   GNU Lesser General Public License Version 3 (the ``LGPL''),
   or (at your option) any later version.

   Software distributed under the License is distributed
   on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
   express or implied. See the LGPL for the specific language
   governing rights and limitations.

   You should have received a copy of the LGPL along with this
   program. If not, go to http://www.gnu.org/licenses/lgpl.html
   or write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

On Debian systems, the complete text of the GNU Lesser General Public
License can be found in `/usr/share/common-licenses/LGPL-3'.")
install(FILES "${CMAKE_BINARY_DIR}/copyright"
        DESTINATION "share/doc/${CPACK_DEBIAN_PACKAGE_NAME}")

# Generate required change log files
execute_process(COMMAND ${GIT_EXECUTABLE}
                        log "--format=%ad  %an  <%ae>%n%n%w(76,8,10)%s%w(76,8,8)%n%n%b%n"
                        --date=short
                COMMAND gzip -n -9
                OUTPUT_FILE "${CMAKE_BINARY_DIR}/changelog.gz")
execute_process(COMMAND sh -c "for c in \$(${GIT_EXECUTABLE} rev-list --all -- \"${CMAKE_CURRENT_LIST_FILE}\") ; do \\
                                 if tag=\$(${GIT_EXECUTABLE} describe --tags \$c 2> /dev/null) ; then \\
                                   replacement=\$(echo $tag \\
                                     | sed -re s/[^0-9]*\\([0-9]+\\)\\.\\([0-9]+\\)-\\([0-9]+\\)-.*/\\\\1.\\$\\(\\(\\\\2+1\\)\\).\\\\3/) ; \\
                                   echo -n \"-e \\\"s/\$c/\$replacement/\\\" \" ; \\
                                 fi ; \\
                               done"
    OUTPUT_VARIABLE RULES)
message(${RULES})
execute_process(COMMAND ${GIT_EXECUTABLE}
                        log "--format=${CPACK_DEBIAN_PACKAGE_NAME} (%H) ${LSB_CODENAME}; urgency=low%n%n%w(76,8,10)%s%w(76,8,8)%n%n%b%n%n%w(200,1,1)-- %an <%ae>  %ad%n"
                        --date=rfc
                        -- "${CMAKE_CURRENT_LIST_FILE}"
                COMMAND sh -c "sed ${RULES}"
                COMMAND gzip -n -9
                OUTPUT_FILE "${CMAKE_BINARY_DIR}/changelog.Debian.gz")
install(FILES "${CMAKE_BINARY_DIR}/changelog.gz"
              "${CMAKE_BINARY_DIR}/changelog.Debian.gz"
        DESTINATION "share/doc/${CPACK_DEBIAN_PACKAGE_NAME}")

message(STATUS "Debian Package: ${CPACK_DEBIAN_PACKAGE_NAME} (${CPACK_DEBIAN_PACKAGE_VERSION}) [${CPACK_PACKAGE_FILE_NAME}.deb]")
