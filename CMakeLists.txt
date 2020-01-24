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
cmake_minimum_required (VERSION 2.10)

Project(webrtc)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
if(WIN32 OR APPLE)
    set(CFG_INTDIR ${CMAKE_BUILD_TYPE}${CMAKE_XCODE_EFFECTIVE_PLATFORMS} CACHE STRING "cmake_cfg_intdir" FORCE)
endif()

option(PYTHON3 "Build webrtc module for python3 instead of python2" OFF)
if (PYTHON3)
    set(CMAKE_PYTHON_EXECUTABLE python3)
else()
    set(CMAKE_PYTHON_EXECUTABLE python)
endif()

###################################################################################################
### PLATFORM DEPENDENT DEFINITIONS
###################################################################################################
if (WIN32)
    foreach (flag_var
             CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
             CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REPLACE "/MD" "-MT" ${flag_var} "${${flag_var}}")
    endforeach()
    set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELWITHDEBINFO} CACHE STRING "flags" FORCE)
    foreach (flag_var
             CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
             CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
        string(REPLACE "/MD" "-MT" ${flag_var} "${${flag_var}}")
    endforeach()
    set(CMAKE_C_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELWITHDEBINFO} CACHE STRING "flags" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} CACHE STRING "flags" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:LIBCMT" CACHE STRING "flags" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} CACHE STRING "flags" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:LIBCMT" CACHE STRING "flags" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP" CACHE STRING "flags" FORCE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP" CACHE STRING "flags" FORCE)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -DNOMINMAX)
else ()
    if (APPLE)
        add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0 -std=c++11)
    else()
        add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1 -std=gnu++11)
    endif()
    add_definitions(-fPIC -fno-rtti)
endif()


set(GENERATOR ${CMAKE_GENERATOR})
SEPARATE_ARGUMENTS(GENERATOR)
list(REVERSE GENERATOR)
list(GET GENERATOR 0 arch)
if (${arch} MATCHES "Win64")
    set(WIN64 ON CACHE BOOL "win64")
endif()

if (${CMAKE_GENERATOR} STREQUAL "NMake Makefiles") 
	set(NMAKE_MAKEFILE_GENERATOR ON CACHE BOOL "nmake__makefile_generator")
endif()

if (APPLE)
    if (IOS)
        set (CMAKE_OSX_ARCHITECTURES ${IOS_ARCH})
    else()
        set (CMAKE_OSX_ARCHITECTURES "x86_64")
    endif()
endif(APPLE)

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    add_definitions(-DDEBUG=1)
endif(CMAKE_BUILD_TYPE MATCHES DEBUG)

###################################################################################################
### ARTIFACTS SECTION
###################################################################################################
set(ARTIFACTS_DIR ${CMAKE_CURRENT_BINARY_DIR}/artifacts)
add_custom_target(
    prep-artifacts
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ARTIFACTS_DIR})

add_custom_target(
    artifacts
    DEPENDS prep-artifacts) 

###################################################################################################
### MAIN SECTION
###################################################################################################
add_subdirectory(third-party)
add_subdirectory(cpp)
if(NOT IOS AND NOT ANDROID)
    add_subdirectory(python)
endif()

set(WEBRTC_INCLUDE ${WEBRTC_INCLUDE} PARENT_SCOPE)
set(WEBRTC_SRC ${WEBRTC_SRC} PARENT_SCOPE)
set(WEBRTCAPI_DEPENDENCIES ${WEBRTCAPI_DEPENDENCIES} PARENT_SCOPE)
set(WEBRTCAPI_SRC ${WEBRTCAPI_SRC} PARENT_SCOPE)
set(WEBRTCAPI_HEADERS ${WEBRTCAPI_HEADERS} PARENT_SCOPE)
set(WEBRTCAPI_TARGET ${WEBRTCAPI_TARGET} PARENT_SCOPE)
