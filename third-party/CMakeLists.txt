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

Project(webrtc-tp)

###################################################################################################
### depot_tools
##################################################################################################
message(STATUS "depot_tools ")
set(OLD_PATH $ENV{PATH})

if(NOT DEFINED DEPOT_TOOLS_ROOT)
    set(DEPOT_TOOLS_ROOT ${CMAKE_CURRENT_BINARY_DIR}/depot_tools)
endif()

if(WIN32)
    #message(WARNING "  You should have depot_tools installed! ")
    #message(WARNING "  Instruction can be found at http://dev.chromium.org/developers/how-tos/install-depot-tools ")
    #message(STATUS "  Setting env variables for webrtc ")
    set(ENV{DEPOT_TOOLS_WIN_TOOLCHAIN} "0")
    if(NOT EXISTS ${DEPOT_TOOLS_ROOT})
        message( STATUS "  Download Chromium depot tools ..." )
        file( DOWNLOAD https://storage.googleapis.com/chrome-infra/depot_tools.zip ${CMAKE_CURRENT_BINARY_DIR}/depot_tools.zip )
        file( MAKE_DIRECTORY ${DEPOT_TOOLS_ROOT} )
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar -x ${CMAKE_CURRENT_BINARY_DIR}/depot_tools.zip
            WORKING_DIRECTORY ${DEPOT_TOOLS_ROOT}
            OUTPUT_QUIET )
        message( STATUS "  Download Chromium depot tools -- done" )
        message( STATUS "  install depot tools ..." )
        execute_process(
            COMMAND gclient.bat
            WORKING_DIRECTORY ${DEPOT_TOOLS_ROOT}
            OUTPUT_QUIET )
        message(STATUS "  install depot tools -- done" )
    endif()
    message(STATUS "  Setting depot_tools into PATH ")
    set(ENV{PATH} "${DEPOT_TOOLS_ROOT}\;${OLD_PATH}")
    message(STATUS "  Setting depot_tools into PATH -- done")
else()
    if(NOT EXISTS ${DEPOT_TOOLS_ROOT})
        message(STATUS "  Cloning depot tools ")
        execute_process(
            COMMAND git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git ${DEPOT_TOOLS_ROOT}
            RESULT_VARIABLE err
            OUTPUT_QUIET)
        if(NOT ${err} EQUAL 0)
            message(FATAL_ERROR "execute_process failed with ${err}" )
        endif()
        message(STATUS "  Cloning depot tools -- done")
    endif()
    message(STATUS "  Setting depot_tools into PATH ")
    set(ENV{PATH} "${DEPOT_TOOLS_ROOT}:${OLD_PATH}")
    message(STATUS "  Setting depot_tools into PATH -- done")
endif()
message(STATUS "depot_tools -- done")


###################################################################################################
### WEBRTC
###################################################################################################
message(STATUS "webrtc ")

if(NOT DEFINED WEBRTC_ROOT)
    set(WEBRTC_ROOT ${CMAKE_CURRENT_BINARY_DIR}/webrtc)
endif()
set(WEBRTC_SRC ${WEBRTC_ROOT}/src)
set(WEBRTC_OUT ${WEBRTC_SRC}/out)

if(APPLE)
    set(NINJA_BUILD_TARGET "webrtc/examples:AppRTCMobile_executable")
elseif(ANDROID)
    set(NINJA_BUILD_TARGET "AppRTCMobile")
else()
    #set(NINJA_BUILD_TARGET "webrtc/examples:peerconnection_client")
endif()

IF(NOT EXISTS ${WEBRTC_ROOT})
    message(STATUS "  Creating webrtc directory ")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory ${WEBRTC_ROOT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RESULT_VARIABLE err
        OUTPUT_QUIET)
    if(NOT ${err} EQUAL 0)
        message(FATAL_ERROR "execute_process failed with ${err}" )
    endif()
    message(STATUS "  Creating webrtc directory -- done")
    message(STATUS "  Fetching webrtc ")
    if(IOS)
        set(WEBRTC_FETCH_TARGET webrtc_ios)
    elseif(ANDROID)
        set(WEBRTC_FETCH_TARGET webrtc_android)
    else()
        set(WEBRTC_FETCH_TARGET webrtc)
    endif()
    execute_process(
        COMMAND bash -c "fetch --nohooks ${WEBRTC_FETCH_TARGET}"
        WORKING_DIRECTORY ${WEBRTC_ROOT}
        RESULT_VARIABLE err
        OUTPUT_QUIET)
    if(NOT ${err} EQUAL 0)
        message(FATAL_ERROR "execute_process failed with ${err}" )
    endif()
    message(STATUS "  Fetching webrtc -- done")
    execute_process(
        COMMAND git checkout branch-heads/${WEBRTC_BRANCH_HEAD}
        WORKING_DIRECTORY ${WEBRTC_SRC}
        RESULT_VARIABLE err)
    if(NOT ${err} EQUAL 0)
        message(FATAL_ERROR "execute_process failed with ${err}" )
    endif()

    if(WIN32)
        execute_process(
            COMMAND git config --system core.longpaths true
            WORKING_DIRECTORY ${WEBRTC_SRC}
            OUTPUT_QUIET)
        execute_process(
            COMMAND git config --system core.longpaths true
            WORKING_DIRECTORY ${WEBRTC_SRC}/third_party
            OUTPUT_QUIET)
    endif()
        
    message(STATUS "  Cleaning repository")
    execute_process(
        COMMAND git reset --hard
	WORKING_DIRECTORY ${WEBRTC_SRC}/build
        OUTPUT_QUIET)
    execute_process(
        COMMAND git reset --hard
        WORKING_DIRECTORY ${WEBRTC_SRC}
        OUTPUT_QUIET)
    execute_process(
        COMMAND git reset --hard
        WORKING_DIRECTORY ${WEBRTC_SRC}/third_party
        OUTPUT_QUIET)

    message(STATUS "  Cleaning repository -- done")

    message(STATUS "  Running glient sync ")
    if(WIN32)
        execute_process(
            COMMAND cmd /C "gclient sync"
            WORKING_DIRECTORY ${WEBRTC_ROOT}
            RESULT_VARIABLE err
            OUTPUT_QUIET)
    else()
        execute_process(
            COMMAND bash -c "yes"
            COMMAND bash -c "gclient sync"
            WORKING_DIRECTORY ${WEBRTC_ROOT}
            RESULT_VARIABLE err
            OUTPUT_QUIET)
    endif()
    if(NOT ${err} EQUAL 0)
        message(FATAL_ERROR "execute_process failed with ${err}" )
    endif()
    message(STATUS "  Running glient sync -- done")
    # install-build-deps should be run after glient sync,
    # because he will can install some libraries from webrtc/src/chromium.
    if(NOT WIN32 AND NOT APPLE AND NOT ANDROID)
        message(STATUS "  Install build dependencies ")
        set(ENV{CHROMIUM_ROOT} ${WEBRTC_ROOT})
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${WEBRTC_SRC}/build/install-build-deps.sh ${CMAKE_CURRENT_BINARY_DIR}
            RESULT_VARIABLE err
            OUTPUT_QUIET) 
        execute_process( 
            COMMAND chmod +x ${CMAKE_CURRENT_BINARY_DIR}/install-build-deps.sh
            RESULT_VARIABLE err
            OUTPUT_QUIET) 
        execute_process( 
            COMMAND "${CMAKE_CURRENT_BINARY_DIR}/install-build-deps.sh" "--no-syms" "--no-arm" "--no-chromeos-fonts" "--no-nacl" "--no-prompt"
            RESULT_VARIABLE err
            OUTPUT_QUIET)
        message(STATUS "  Install build dependencies -- done")
    endif()
endif()


message(STATUS "  Running git pull ")
execute_process(
    COMMAND git checkout .
    WORKING_DIRECTORY ${WEBRTC_SRC})
execute_process(
    COMMAND git fetch
    WORKING_DIRECTORY ${WEBRTC_SRC})
execute_process(
    COMMAND git checkout branch-heads/${WEBRTC_BRANCH_HEAD}
    WORKING_DIRECTORY ${WEBRTC_SRC}
    RESULT_VARIABLE err)
if(NOT ${err} EQUAL 0)
    message(FATAL_ERROR "execute_process failed with ${err}" )
endif()
message(STATUS "  Running git pull -- done") 

if(WIN32)
    execute_process(
        COMMAND git config --system core.longpaths true
        WORKING_DIRECTORY ${WEBRTC_SRC}
        OUTPUT_QUIET)
    execute_process(
        COMMAND git config --system core.longpaths true
        WORKING_DIRECTORY ${WEBRTC_SRC}/third_party
        OUTPUT_QUIET)
endif()
        
message(STATUS "  Cleaning repository")
execute_process(
    COMMAND git reset --hard
    WORKING_DIRECTORY ${WEBRTC_SRC}/build
    OUTPUT_QUIET)
execute_process(
    COMMAND git reset --hard
    WORKING_DIRECTORY ${WEBRTC_SRC}
    OUTPUT_QUIET)
execute_process(
    COMMAND git reset --hard
    WORKING_DIRECTORY ${WEBRTC_SRC}/third_party
    OUTPUT_QUIET)

message(STATUS "  Cleaning repository -- done")

message(STATUS "  Running glient sync ")
if(WIN32)
    execute_process(
        COMMAND cmd /C "gclient sync --no-history"
        WORKING_DIRECTORY ${WEBRTC_ROOT}
        RESULT_VARIABLE err
        OUTPUT_QUIET)
else()
    execute_process(
        COMMAND bash -c "yes"
        COMMAND bash -c "gclient sync --no-history"
        WORKING_DIRECTORY ${WEBRTC_ROOT}
        RESULT_VARIABLE err
        OUTPUT_QUIET)
endif()
if(NOT ${err} EQUAL 0)
    message(FATAL_ERROR "execute_process failed with ${err}" )
endif()
message(STATUS "  Running glient sync -- done") 

if(WIN64)
   set(ENV{GYP_DEFINES} "target_arch=x64" $ENV{GYP_DEFINES})
endif()

if(NOT WIN32 AND NOT APPLE AND NOT ANDROID)
    execute_process(
        COMMAND "build/linux/sysroot_scripts/install-sysroot.py" "--arch=amd64"
        WORKING_DIRECTORY ${WEBRTC_SRC}
        OUTPUT_QUIET)
endif()


if(IOS)
    set(WEBRTC_BUILD_TYPE ${IOS_WEBRTC_ARCH})
elseif(ANDROID)
    if(${ANDROID_ABI} STREQUAL "arm64-v8a")
        set(WEBRTC_BUILD_TYPE "arm64")
        set(WEBRTC_ARM_NEON "false")
    elseif(${ANDROID_ABI} STREQUAL "armeabi")
        set(WEBRTC_BUILD_TYPE "arm")
        set(WEBRTC_ARM_VER "6")
        set(WEBRTC_ARM_NEON "false")
    elseif(${ANDROID_ABI} STREQUAL "armeabi-v7a")
        set(WEBRTC_BUILD_TYPE "arm")
        set(WEBRTC_ARM_VER "7")
        set(WEBRTC_ARM_NEON "false")
    elseif(${ANDROID_ABI} STREQUAL "x86_64")
        set(WEBRTC_BUILD_TYPE "x64")
    elseif(${ANDROID_ABI} STREQUAL "mips")
        set(WEBRTC_BUILD_TYPE "mipsel")
    elseif(${ANDROID_ABI} STREQUAL "mips64")
        set(WEBRTC_BUILD_TYPE "mips64el")
    else()
        set(WEBRTC_BUILD_TYPE ${ANDROID_ABI})
    endif()
else()
    set(WEBRTC_BUILD_TYPE default)
endif()

foreach(build_type ${WEBRTC_BUILD_TYPE})
    message(STATUS "  Building webrtc ${build_type}${WEBRTC_ARM_VER}${WEBRTC_ARM_NEON} ")
    message(STATUS "  Cleaning repository")
    execute_process(
        COMMAND git reset --hard
        WORKING_DIRECTORY ${WEBRTC_SRC}/build
        OUTPUT_QUIET)
    execute_process(
        COMMAND git reset --hard
        WORKING_DIRECTORY ${WEBRTC_SRC}
        OUTPUT_QUIET)
    execute_process(
        COMMAND git reset --hard
        WORKING_DIRECTORY ${WEBRTC_SRC}/third_party
        OUTPUT_QUIET)
    message(STATUS "  Cleaning repository -- done")

	
    if(NOT WIN32 AND NOT APPLE AND NOT ANDROID)
        message(STATUS "    Applying linux patch. Setting -Wno-unused-result flag")
        execute_process( 
            COMMAND chmod +x ${CMAKE_CURRENT_SOURCE_DIR}/patch_linux_ur.sh
            RESULT_VARIABLE err
            OUTPUT_QUIET) 
        if(NOT ${err} EQUAL 0)
            message(FATAL_ERROR "execute_process failed with ${err}" )
        endif()		
        execute_process(
            COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/patch_linux_ur.sh"
            WORKING_DIRECTORY ${WEBRTC_SRC}
            RESULT_VARIABLE err
            OUTPUT_QUIET)
        if(NOT ${err} EQUAL 0)
            message(FATAL_ERROR "execute_process failed with ${err}" )
        endif()		
        message(STATUS "    Applying linux patch. Setting -Wno-unused-result flag  --  done")
    endif()

    if(ANDROID)	
        # apply android or apple patch
        message(STATUS "    Applying android(apple) patch. Making ninja build json, jsoncpp")
        if (ANDROID)
            set (JSON_PATCH_PATH ${CMAKE_CURRENT_SOURCE_DIR}/patch_android_json.sh)
        else()	# OS X and IOS
            set (JSON_PATCH_PATH ${CMAKE_CURRENT_SOURCE_DIR}/patch_osx_json.sh)
        endif()
        execute_process( 
            COMMAND chmod +x ${JSON_PATCH_PATH}
            RESULT_VARIABLE err
            OUTPUT_QUIET) 
        if(NOT ${err} EQUAL 0)
            message(FATAL_ERROR "execute_process failed with ${err}" )
        endif()		
        execute_process(
            COMMAND ${JSON_PATCH_PATH}
            WORKING_DIRECTORY ${WEBRTC_SRC}/webrtc
            RESULT_VARIABLE err
            OUTPUT_QUIET)
        if(NOT ${err} EQUAL 0)
            message(FATAL_ERROR "execute_process failed with ${err}" )
        endif()
        message(STATUS "    Applying android(apple) patch. Making ninja build json, jsoncpp. --  done")
    endif()


    message(STATUS "    Running gn gen out/${build_type}${WEBRTC_ARM_VER}${WEBRTC_ARM_NEON} ")
    if(${build_type} STREQUAL "default")
        if(LINUX_X86 OR (WIN32 AND NOT WIN64))
            set(ADDITIONAL_PARAMS "target_cpu=\\\"x86\\\"")
        endif()
        if(WIN32 OR (UNIX AND NOT APPLE))
            set(ADDITIONAL_PARAMS "${ADDITIONAL_PARAMS} is_clang=false")
        endif()
        if (UNIX AND NOT APPLE)
            execute_process(COMMAND bash -c "grep -Po '^ID=\"?\\K(\\w+)' /etc/os-release" OUTPUT_VARIABLE DISTRO_ID)
            string(REGEX REPLACE "\n$" "" DISTRO_ID ${DISTRO_ID})
            message(STATUS "    ** distro_id = ${DISTRO_ID} ")
            if("${DISTRO_ID}" STREQUAL "centos")
            message(STATUS "    ** distro_id = ${DISTRO_ID} ")
            set(ADDITIONAL_PARAMS "${ADDITIONAL_PARAMS} is_cfi=false treat_warnings_as_errors=false use_custom_libcxx=false use_allocator=\\\"none\\\" use_gold=false use_sysroot=false target_cpu=\\\"x64\\\" rtc_include_tests=false")
            elseif("${DISTRO_ID}" STREQUAL "opensuse")
            message(STATUS "    ** distro_id = ${DISTRO_ID} ")
            set(ADDITIONAL_PARAMS "${ADDITIONAL_PARAMS} is_cfi=false treat_warnings_as_errors=false use_custom_libcxx=false use_allocator=\\\"none\\\" use_gold=false use_sysroot=false target_cpu=\\\"x64\\\" rtc_include_tests=false")
            else()
            set(ADDITIONAL_PARAMS "${ADDITIONAL_PARAMS} is_cfi=false treat_warnings_as_errors=false use_custom_libcxx=false use_allocator=\\\"none\\\" use_gold=false")
            endif()
        endif()
    else()
        if(IOS)
            set(ADDITIONAL_PARAMS "target_os=\\\"ios\\\" ios_enable_code_signing=false")
        elseif(ANDROID)
            set(ADDITIONAL_PARAMS "target_os=\\\"android\\\" is_clang=true")
        endif()
        set(ADDITIONAL_PARAMS "${ADDITIONAL_PARAMS} target_cpu=\\\"${build_type}\\\"")
        if(DEFINED WEBRTC_ARM_VER)
            set(ADDITIONAL_PARAMS "${ADDITIONAL_PARAMS} arm_version=${WEBRTC_ARM_VER}")
        endif()
        if(DEFINED WEBRTC_ARM_NEON)
            #set(ADDITIONAL_PARAMS "${ADDITIONAL_PARAMS} arm_neon=${WEBRTC_ARM_NEON}")
        endif()
    endif()
 
    set(build_type "${build_type}${WEBRTC_ARM_VER}${WEBRTC_ARM_NEON}")

    execute_process(
        COMMAND bash -c "gn gen out/${build_type} --args=\"is_official_build=true is_debug=false rtc_include_tests=false rtc_build_examples=false ${ADDITIONAL_PARAMS}\""
        WORKING_DIRECTORY ${WEBRTC_SRC}
        RESULT_VARIABLE err
        )
    if(NOT ${err} EQUAL 0)
        message(FATAL_ERROR "execute_process failed with ${err}" )
    endif()

    message(STATUS "    Running gn gen out/${build_type} -- done")


    message(STATUS "    Running ninja build ")
    execute_process(
        COMMAND ninja -C out/${build_type}
        WORKING_DIRECTORY ${WEBRTC_SRC}
        OUTPUT_VARIABLE NINJA_LOG
        RESULT_VARIABLE err)
    if(NOT ${err} EQUAL 0)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${WEBRTC_SRC}/out/${build_type}
            OUTPUT_QUIET)
        message(STATUS "    Running gn gen out/${build_type} ")
        execute_process(
            COMMAND bash -c "gn gen out/${build_type} --args=\"is_official_build=true is_debug=false ${ADDITIONAL_PARAMS}\""
            WORKING_DIRECTORY ${WEBRTC_SRC}
            RESULT_VARIABLE err
            OUTPUT_QUIET)
        if(NOT ${err} EQUAL 0)
            message(FATAL_ERROR "execute_process failed with ${err}" )
        endif()

        message(STATUS "    Running gn gen out/${build_type} -- done")

        message(STATUS "    Running ninja build ")
        execute_process(
            COMMAND ninja -C out/${build_type} 
            WORKING_DIRECTORY ${WEBRTC_SRC}
            OUTPUT_VARIABLE NINJA_LOG
            RESULT_VARIABLE err)
        if(NOT ${err} EQUAL 0)
            message("${NINJA_LOG}")
            message(FATAL_ERROR "execute_process failed with ${err}" )
        endif()
    endif()
    message(STATUS "    Running ninja build -- done")

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory ${WEBRTC_ROOT}/${build_type})


    if (NOT EXISTS ${WEBRTC_ROOT}/${build_type}/${CMAKE_STATIC_LIBRARY_PREFIX}webrtc_full${CMAKE_STATIC_LIBRARY_SUFFIX})
        set(NEED_TO_REBUILD_WEBRTC_FULL ON)
    endif()
 
    if(NOT "${NINJA_LOG}" MATCHES "no work to do\.")
        set(NEED_TO_REBUILD_WEBRTC_FULL ON)
    endif()

    if(${NEED_TO_REBUILD_WEBRTC_FULL}) 
        message(STATUS "    Linking fat library from objects ")
        execute_process(
            COMMAND bash -c "rm -f ${build_type}/${CMAKE_STATIC_LIBRARY_PREFIX}webrtc_full${CMAKE_STATIC_LIBRARY_SUFFIX}"
            WORKING_DIRECTORY ${WEBRTC_ROOT}
            RESULT_VARIABLE err
            OUTPUT_QUIET)
        if(NOT ${err} EQUAL 0)
            message(FATAL_ERROR "execute_process failed with ${err}" )
        endif()

        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/build_webrtc_full.py ${WEBRTC_ROOT}/
            OUTPUT_QUIET)

        set(OUT_DIR ${WEBRTC_ROOT}/${build_type}/)
        set(OUT "${CMAKE_STATIC_LIBRARY_PREFIX}webrtc_full${CMAKE_STATIC_LIBRARY_SUFFIX}")
        set(NINJA_FILE_PATH "obj/webrtc/examples/")
        if(APPLE)
            if(IOS)
                set(NINJA_FILE AppRTCMobile_arch_executable.ninja)
                set(NINJA_TARGET "${build_type}/AppRTCMobile")
                set(CMAKE_AR ar)
            else()
                set(NINJA_FILE AppRTCMobile_executable.ninja)
                set(NINJA_TARGET "AppRTCMobile_executable/AppRTCMobile")
            endif()
        elseif(ANDROID)
            set(NINJA_FILE_PATH "obj/webrtc/sdk/android/")
            set(NINJA_FILE libjingle_peerconnection_so.ninja)
            set(NINJA_TARGET "libjingle_peerconnection_so.so")
        else()
            set(NINJA_FILE peerconnection_client.ninja)
            set(NINJA_TARGET peerconnection_client)
        endif()

        execute_process(
            COMMAND ${CMAKE_PYTHON_EXECUTABLE} ${WEBRTC_ROOT}/build_webrtc_full.py
                "${WEBRTC_SRC}/out/${build_type}"
                "${CMAKE_CXX_OUTPUT_EXTENSION}"
                "${CMAKE_STATIC_LIBRARY_SUFFIX}"
                "${OUT_DIR}"
                "${OUT}"
                "${NINJA_FILE_PATH}"
                "${NINJA_FILE}"
                "${NINJA_TARGET}"
                "${CMAKE_AR}"
            ERROR_FILE ${WEBRTC_ROOT}/err.log
            OUTPUT_FILE ${WEBRTC_ROOT}/out.log
            WORKING_DIRECTORY ${WEBRTC_ROOT})

        message(STATUS "    Linking fat library from objects -- done")
    endif()
    message(STATUS "  Building webrtc ${build_type} -- done")

    file(GLOB WEBRTC_${build_type}_LIBRARIES ${WEBRTC_ROOT}/${build_type}/*${CMAKE_STATIC_LIBRARY_SUFFIX})
    list(SORT WEBRTC_${build_type}_LIBRARIES)
    set(WEBRTC_default_LIBRARIES ${WEBRTC_${build_type}_LIBRARIES})
    if(ANDROID)
        list(REMOVE_ITEM WEBRTC_default_LIBRARIES "${WEBRTC_ROOT}/${build_type}/libgcc.a")
    endif()
	
endforeach()

if(IOS)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${WEBRTC_ROOT}/default)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory ${WEBRTC_ROOT}/default)

    foreach(build_type ${WEBRTC_BUILD_TYPE})
        foreach(lib ${WEBRTC_${build_type}_LIBRARIES})
            get_filename_component(lib_name ${lib} NAME)
            if(EXISTS ${WEBRTC_ROOT}/default/${lib_name})
                execute_process(
                    COMMAND lipo -create ${lib} ${WEBRTC_ROOT}/default/${lib_name} -output ${WEBRTC_ROOT}/default/${lib_name})
            else()
                execute_process(
                    COMMAND lipo -create ${lib} -output ${WEBRTC_ROOT}/default/${lib_name})
            endif()
        endforeach()

        execute_process(
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${WEBRTC_ROOT}/${build_type})
    endforeach()
endif()
if(NOT ANDROID)
    file(GLOB WEBRTC_default_LIBRARIES ${WEBRTC_ROOT}/default/*${CMAKE_STATIC_LIBRARY_SUFFIX})
    list(SORT WEBRTC_default_LIBRARIES)
endif()


set(WEBRTC_LIBRARIES ${WEBRTC_SRC}/out/default/obj/${CMAKE_STATIC_LIBRARY_PREFIX}webrtc${CMAKE_STATIC_LIBRARY_SUFFIX})
#${WEBRTC_default_LIBRARIES}) 
set(WEBRTC_LIBRARIES ${WEBRTC_LIBRARIES} PARENT_SCOPE)
set(WEBRTC_INCLUDE ${WEBRTC_SRC} ${WEBRTC_SRC}/third_party/abseil-cpp)
set(WEBRTC_INCLUDE ${WEBRTC_INCLUDE} ${WEBRTC_SRC}/third_party/jsoncpp/overrides/include ${WEBRTC_SRC}/third_party/jsoncpp/source/include)
set(WEBRTC_INCLUDE ${WEBRTC_INCLUDE} PARENT_SCOPE)
set(WEBRTC_SRC ${WEBRTC_SRC} PARENT_SCOPE)
message(STATUS "webrtc -- done")
set(ENV{PATH} "${OLD_PATH}")
                                                                        
###################################################################################################
### sip
##################################################################################################
if(NOT IOS AND NOT ANDROID)
    message(STATUS "sip ")
    set(SIP_ROOT ${CMAKE_CURRENT_BINARY_DIR}/sip-4.19.3)

    IF(NOT EXISTS ${SIP_ROOT}) 
        message(STATUS "  Unpacking sip ")
        set(SIP_ARCHIVE ${CMAKE_CURRENT_SOURCE_DIR}/sip-4.19.3.tar.gz)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xzf ${SIP_ARCHIVE}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            RESULT_VARIABLE err
            OUTPUT_QUIET)
        if(NOT ${err} EQUAL 0)
            message(FATAL_ERROR "execute_process failed with ${err}" )
        endif()
        message(STATUS "  Unpacking sip -- done")

        if(LINUX_X86)
            set(SIP_PLATFORM "--platform" "linux-g++-32")
        endif()

        message(STATUS "  Configuring sip ")
        execute_process(
            COMMAND ${CMAKE_PYTHON_EXECUTABLE} configure.py ${SIP_PLATFORM}
            WORKING_DIRECTORY ${SIP_ROOT}
            RESULT_VARIABLE err
            OUTPUT_QUIET)
        if(NOT ${err} EQUAL 0)
            message(FATAL_ERROR "execute_process failed with ${err}" )
        endif()
        message(STATUS "  Configuring sip -- done")

        if(WIN32)
            set(MAKE nmake)
        else()
            set(MAKE make)
        endif()

        message(STATUS "  Building sip ")
        execute_process(
            COMMAND ${MAKE}
            WORKING_DIRECTORY ${SIP_ROOT}
            RESULT_VARIABLE err
            OUTPUT_QUIET)
        if(NOT ${err} EQUAL 0)
            message(FATAL_ERROR "execute_process failed with ${err}" )
        endif()
        message(STATUS "  Building sip -- done")

        message(STATUS "  Installing sip ")
        execute_process(
            COMMAND ${MAKE} install
            WORKING_DIRECTORY ${SIP_ROOT}
            RESULT_VARIABLE err
            OUTPUT_QUIET)
        message(STATUS "  Installing sip -- done")
    endif()

    if(WIN32)
        set(SIP_LIBRARY ${SIP_ROOT}/siplib/sip.pyd)
    else()
        set(SIP_LIBRARY ${SIP_ROOT}/siplib/sip.so)
    endif()
    set(SIP_LIBRARY ${SIP_LIBRARY} PARENT_SCOPE)
    message(STATUS "sip -- done")

    add_custom_target(
        artifacts-sip
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SIP_LIBRARY} ${ARTIFACTS_DIR}/
        DEPENDS prep-artifacts)
    add_dependencies(artifacts artifacts-sip)
endif()

###################################################################################################
### gtest
##################################################################################################
if(NOT IOS AND NOT ANDROID AND NOT DISABLE_TESTS)
    message(STATUS "gtest ")
    set(GTEST_ROOT ${CMAKE_CURRENT_BINARY_DIR}/googletest)

    IF(NOT EXISTS ${GTEST_ROOT})
        message(STATUS "  Cloning googletest ")
        execute_process(
            COMMAND git clone https://github.com/google/googletest.git
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        message(STATUS "  Cloning googletest -- done")
    endif()

    add_subdirectory(${GTEST_ROOT} ${GTEST_ROOT})

    set(GTEST_TARGETS gmock gmock_main gtest gtest_main PARENT_SCOPE)
    set(GTEST_INCLUDE ${GTEST_ROOT}/googlemock/include/ ${GTEST_ROOT}/googletest/include/ PARENT_SCOPE)
    set(GTEST_LIBRARIES ${CMAKE_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gmock${CMAKE_STATIC_LIBRARY_SUFFIX}
                        ${CMAKE_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gmock_main${CMAKE_STATIC_LIBRARY_SUFFIX}
                        ${CMAKE_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX}
                        ${CMAKE_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest_main${CMAKE_STATIC_LIBRARY_SUFFIX} PARENT_SCOPE)
    message(STATUS "gtest -- done")
endif()

