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
cmake_minimum_required (VERSION 3.1)
set(CMAKE_BACKWARDS_COMPATIBILITY 3.1)
Project(webrtc-cpp)

###################################################################################################
### PLATFORM DEPENDENT DEFINITIONS
###################################################################################################
if (WIN32)
    add_definitions(-DWEBRTC_WIN=1)
else ()
    add_definitions(-DWEBRTC_POSIX=1)
    if (APPLE)
        add_definitions(-DWEBRTC_MAC=1)
        if(IOS)
            add_definitions(-DWEBRTC_IOS=1)
        endif()
    else()
        add_definitions(-DWEBRTC_LINUX=1)
        if(ANDROID)
            add_definitions(-DWEBRTC_ANDROID=1)
        endif()
    endif()
endif()

###################################################################################################
### LIBRARY SOURES
################################################################################################### 
file(GLOB WEBRTC_API_CC ${CMAKE_CURRENT_SOURCE_DIR}/webrtcapi/*.cc)
file(GLOB WEBRTC_API_CPP ${CMAKE_CURRENT_SOURCE_DIR}/webrtcapi/*.cpp)
set(WEBRTC_SOURCES ${WEBRTC_CLIENT_CC} ${WEBRTC_CLIENT_CPP} ${WEBRTC_API_CC} ${WEBRTC_API_CPP})

file(GLOB WEBRTC_API_H ${CMAKE_CURRENT_SOURCE_DIR}/webrtcapi/*.h)
file(GLOB WEBRTC_API_HPP ${CMAKE_CURRENT_SOURCE_DIR}/webrtcapi/*.hpp)
set(WEBRTC_HEADERS ${WEBRTC_CLIENT_H} ${WEBRTC_CLIENT_HPP} ${WEBRTC_API_H} ${WEBRTC_API_HPP})

set(SOURCES ${WEBRTC_SOURCES} ${WEBRTC_HEADERS})

###################################################################################################
### LIBRARY INCLUDE DIRECTORIES
###################################################################################################
set(WEBRTCAPI_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/webrtcapi)
include_directories(${WEBRTCAPI_INCLUDE})
include_directories(${WEBRTC_INCLUDE})


###################################################################################################
### LIBRARY DEP TARGETS
###################################################################################################
	

###################################################################################################
### LIBRARY LINK DEPENDENCIES
###################################################################################################
set(DEPENDENCIES ${DEPENDENCIES} ${WEBRTC_LIBRARIES})


###################################################################################################
### LIBRARY
###################################################################################################
add_library(webrtcapi STATIC ${SOURCES})

target_link_libraries(webrtcapi ${DEPENDENCIES})
set(WEBRTCAPI_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/${CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}webrtcapi${CMAKE_STATIC_LIBRARY_SUFFIX})

###################################################################################################
### TESTS
###################################################################################################
if(NOT ANDROID AND NOT IOS AND NOT DISABLE_TESTS)
    add_subdirectory(tests)
endif()

###################################################################################################
### PARENT LEVEL DEFINITIONS
###################################################################################################
set(WEBRTCAPI_DEPENDENCIES ${WEBRTCAPI_LIBRARIES} ${DEPENDENCIES})
set(WEBRTCAPI_DEPENDENCIES ${WEBRTCAPI_DEPENDENCIES} PARENT_SCOPE)
set(WEBRTCAPI_DEP_TARGETS ${DEP_TARGETS} PARENT_SCOPE)
set(WEBRTCAPI_SRC ${CMAKE_CURRENT_SOURCE_DIR}/webrtcapi PARENT_SCOPE)
set(WEBRTCAPI_ROOT ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
set(WEBRTCAPI_HEADERS ${WEBRTCAPI_INCLUDE}/webrtc_api.h PARENT_SCOPE)
set(WEBRTCAPI_TARGET webrtcapi PARENT_SCOPE)

###################################################################################################
### ARTIFACTS SECTION
###################################################################################################
if(IOS)
    add_custom_target(
        artifacts-cpp
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${WEBRTCAPI_DEPENDENCIES} ${ARTIFACTS_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/webrtcapi/webrtc_api.h ${ARTIFACTS_DIR}/
        DEPENDS webrtcapi prep-artifacts)

    add_dependencies(artifacts artifacts-cpp)
endif()
