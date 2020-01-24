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
set (CMAKE_CXX_STANDARD 11)
Project(webrtc-cpp-tests)

###################################################################################################
### TESTS SOURES
################################################################################################### 
file(GLOB TESTS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB TESTS_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
set(SOURCES ${TESTS_SOURCES} ${TESTS_HEADERS}) 

###################################################################################################
### TESTS INCLUDE DIRECTORIES
###################################################################################################
include_directories(${WEBRTCAPI_INCLUDE})
include_directories(${WEBRTC_INCLUDE})                   
include_directories(${GTEST_INCLUDE})

###################################################################################################
### LIBRARY DEP TARGETS
###################################################################################################
set(DEP_TARGETS ${DEP_TARGETS} ${GTEST_TARGETS})
set(DEP_TARGETS ${DEP_TARGETS} webrtcapi)

###################################################################################################
### LIBRARY LINK DEPENDENCIES
###################################################################################################
set(DEPENDENCIES ${WEBRTCAPI_LIBRARIES} ${DEPENDENCIES} ${GTEST_LIBRARIES})
if(WIN32)
    set(DEPENDENCIES ${DEPENDENCIES} winmm.lib ws2_32.lib secur32.lib msdmo.lib dmoguids.lib wmcodecdspuuid.lib advapi32.lib)
elseif(APPLE)
    set(DEPENDENCIES ${DEPENDENCIES} "-framework AppKit" "-framework Cocoa" "-framework Security" "-framework IOKit" "-framework SystemConfiguration" "-framework ApplicationServices" "-framework CoreServices" "-framework CoreVideo" "-framework CoreAudio" "-framework AudioToolbox" "-framework Carbon" "-framework Foundation" "-framework AVFoundation" "-framework QTKit" "-framework QuartzCore" "-framework CoreMedia" "-framework OpenGL" "-framework CoreVideo" "-framework CoreGraphics" "-framework CFNetwork" "-licucore")
else()
    set(DEPENDENCIES "-Wl,--start-group" ${DEPENDENCIES} "-ldl" "-lpthread" "-lX11" "-lXcomposite" "-lXext" "-lXrender" "-Wl,--end-group")
endif()


###################################################################################################
### LIBRARY
###################################################################################################
add_executable(webrtcapi-tests ${SOURCES})
target_link_libraries(webrtcapi-tests ${DEPENDENCIES})
add_dependencies(webrtcapi-tests ${DEP_TARGETS})

###################################################################################################
### ARTIFACTS SECTION
###################################################################################################
add_custom_target(
    artifacts-cpp-tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/${CFG_INTDIR}/webrtcapi-tests${CMAKE_EXECUTABLE_SUFFIX} ${ARTIFACTS_DIR}/
    DEPENDS prep-artifacts webrtcapi-tests)

add_dependencies(artifacts artifacts-cpp-tests)

