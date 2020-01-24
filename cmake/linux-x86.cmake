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
# the name of the target operating system
set(CMAKE_SYSTEM_NAME Linux)
set(LINUX_X86 ON)

# which compilers to use for C and C++
set(CMAKE_C_COMPILER "gcc" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS "-m32" CACHE STRING "" FORCE)
set(CMAKE_CXX_COMPILER "g++" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "-m32" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "-m32" CACHE STRING "" FORCE)
set(CMAKE_MODULE_LINKER_FLAGS "-m32" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS "-m32" CACHE STRING "" FORCE)

# here is the target environment located
set(CMAKE_FIND_ROOT_PATH "/usr/i486-linux-gnu" CACHE STRING "" FORCE)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)