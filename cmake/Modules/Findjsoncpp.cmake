###############################################################################
#   
#   Pvtbox. Fast and secure file transfer & sync directly across your devices. 
#   Copyright © 2020  Pb Private Cloud Solutions Ltd. 
#   
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#   
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#   
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.
#   
###############################################################################
# Find jsoncpp
#
# Find the jsoncpp includes and library
# 
# if you nee to add a custom library search path, do it via via CMAKE_PREFIX_PATH 
# 
# This module defines
#  JSONCPP_INCLUDE_DIRS, where to find header, etc.
#  JSONCPP_LIBRARIES, the libraries needed to use jsoncpp.
#  JSONCPP_FOUND, If false, do not try to use jsoncpp.
#  JSONCPP_INCLUDE_PREFIX, include prefix for jsoncpp

# only look in default directories
find_path(
	JSONCPP_INCLUDE_DIR 
	NAMES jsoncpp/json/json.h json/json.h
	DOC "jsoncpp include dir"
)

find_library(
	JSONCPP_LIBRARY
	NAMES jsoncpp
	DOC "jsoncpp library"
)

set(JSONCPP_INCLUDE_DIRS ${JSONCPP_INCLUDE_DIR})
set(JSONCPP_LIBRARIES ${JSONCPP_LIBRARY})

# debug library on windows
# same naming convention as in qt (appending debug library with d)
# boost is using the same "hack" as us with "optimized" and "debug"
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	find_library(
		JSONCPP_LIBRARY_DEBUG
		NAMES jsoncppd
		DOC "jsoncpp debug library"
	)
	
	set(JSONCPP_LIBRARIES optimized ${JSONCPP_LIBRARIES} debug ${JSONCPP_LIBRARY_DEBUG})

endif()

# find JSONCPP_INCLUDE_PREFIX
find_path(
	JSONCPP_INCLUDE_PREFIX
	NAMES json.h
	PATH_SUFFIXES jsoncpp/json json
)

if (${JSONCPP_INCLUDE_PREFIX} MATCHES "jsoncpp")
	set(JSONCPP_INCLUDE_PREFIX "jsoncpp/json")
else()
	set(JSONCPP_INCLUDE_PREFIX "json")
endif()

# handle the QUIETLY and REQUIRED arguments and set JSONCPP_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(jsoncpp DEFAULT_MSG
	JSONCPP_INCLUDE_DIR JSONCPP_LIBRARY)
mark_as_advanced (JSONCPP_INCLUDE_DIR JSONCPP_LIBRARY)
