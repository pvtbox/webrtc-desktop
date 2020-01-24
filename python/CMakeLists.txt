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

Project(webrtc-python)

if(WIN32)
    set(MAKE_COMMAND nmake)
    set(WEBRTC_PYTHON_CONFIGURE configure-win.py)
else()
    set(MAKE_COMMAND make)
    if(APPLE)
        set(WEBRTC_PYTHON_CONFIGURE configure-osx.py)
    else()
        if(LINUX_X86)
            set(WEBRTC_PYTHON_CONFIGURE configure-linux_x86.py)
        else()
            set(WEBRTC_PYTHON_CONFIGURE configure-linux.py)
        endif()
    endif()
endif()

add_custom_target(
    webrtc-python-copy
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${WEBRTCAPI_SRC}/webrtc_api.h ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/webrtc.sip ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${WEBRTC_PYTHON_CONFIGURE} ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS webrtcapi)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dependencies.txt "${WEBRTCAPI_DEPENDENCIES}")
add_custom_target(
    webrtc-python-configure
    COMMAND ${CMAKE_PYTHON_EXECUTABLE} ${WEBRTC_PYTHON_CONFIGURE}
    DEPENDS webrtc-python-copy)

add_custom_target(
    webrtc-python
    COMMAND ${MAKE_COMMAND}
    DEPENDS webrtc-python-configure)



set(WEBRTC_PYTHON_ROOT ${CMAKE_CURRENT_BINARY_DIR})
set(WEBRTC_PYTHON_ROOT ${WEBRTC_PYTHON_ROOT} PARENT_SCOPE)
set(WEBRTC_PYTHON_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(WEBRTC_PYTHON_SOURCE ${WEBRTC_PYTHON_SOURCE} PARENT_SCOPE)

###################################################################################################
### ARTIFACTS SECTION
###################################################################################################
set(ARTIFACTS_PYWEBRTC_DIR ${ARTIFACTS_DIR}/pywebrtc)
add_custom_target(
    prep-artifacts-python
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ARTIFACTS_PYWEBRTC_DIR}
    DEPENDS prep-artifacts)

if(WIN32)
    add_custom_target(
        artifacts-python-win
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${WEBRTC_PYTHON_ROOT}/webrtc.pyd ${ARTIFACTS_DIR}/webrtc.pyd
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${WEBRTC_PYTHON_ROOT}/webrtc.pdb ${ARTIFACTS_DIR}/webrtc.pdb
        DEPENDS webrtc-python)
    add_dependencies(artifacts artifacts-python-win)
else()
    add_custom_target(
        artifacts-python-unix
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${WEBRTC_PYTHON_ROOT}/webrtc.so ${ARTIFACTS_DIR}/webrtc.so
        DEPENDS webrtc-python)
    add_dependencies(artifacts artifacts-python-unix)
endif()

