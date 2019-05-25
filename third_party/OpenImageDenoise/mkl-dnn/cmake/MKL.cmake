#===============================================================================
# Copyright 2016-2018 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#===============================================================================

# Locate Intel(R) MKL installation using MKLROOT or look in
# ${CMAKE_CURRENT_SOURCE_DIR}/external
#===============================================================================

if(MKL_cmake_included)
    return()
endif()
set(MKL_cmake_included true)
include("cmake/utils.cmake")
include("cmake/options.cmake")

# set SKIP_THIS_MKL to true if given configuration is not supported
function(maybe_skip_this_mkl LIBNAME)
    # Optimism...
    set(SKIP_THIS_MKL False PARENT_SCOPE)

    # Both mklml_intel and mklml_gnu are OpenMP based.
    # So in case of TBB link with Intel MKL (RT library) and either set:
    #   MKL_THREADING_LAYER=tbb
    # to make Intel MKL use TBB threading as well, or
    #   MKL_THREADING_LAYER=sequential
    # to make Intel MKL be sequential.
    if (MKLDNN_THREADING STREQUAL "TBB" AND LIBNAME MATCHES "mklml")
        set(SKIP_THIS_MKL True PARENT_SCOPE)
    endif()

    # user doesn't want Intel MKL at all
    if (MKLDNN_USE_MKL STREQUAL "NONE")
        set(SKIP_THIS_MKL True PARENT_SCOPE)
    endif()

    # user specifies Intel MKL-ML should be used
    if (MKLDNN_USE_MKL STREQUAL "ML")
        if (LIBNAME STREQUAL "mkl_rt")
            set(SKIP_THIS_MKL True PARENT_SCOPE)
        endif()
    endif()

    # user specifies full Intel MKL should be used
    if (MKLDNN_USE_MKL MATCHES "FULL")
        if (LIBNAME MATCHES "mklml")
            set(SKIP_THIS_MKL True PARENT_SCOPE)
        endif()
    endif()

    # avoid using Intel MKL-ML that is not compatible with compiler's OpenMP RT
    if (MKLDNN_THREADING STREQUAL "OMP:COMP")
        if ((LIBNAME STREQUAL "mklml_intel" OR LIBNAME STREQUAL "mklml")
                AND (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Intel"))
            set(SKIP_THIS_MKL True PARENT_SCOPE)
        elseif (LIBNAME STREQUAL "mklml_gnu"
                AND (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
            set(SKIP_THIS_MKL True PARENT_SCOPE)
        endif()
    elseif (MKLDNN_THREADING STREQUAL "OMP:INTEL")
       if (LIBNAME STREQUAL "mklml_gnu")
           set(SKIP_THIS_MKL True PARENT_SCOPE)
       endif()
    endif()
endfunction()

function(detect_mkl LIBNAME)
    if(HAVE_MKL)
        return()
    endif()

    maybe_skip_this_mkl(${LIBNAME})
    set_if(SKIP_THIS_MKL MAYBE_SKIP_MSG "... skipped")
    message(STATUS "Detecting Intel(R) MKL: trying ${LIBNAME}${MAYBE_SKIP_MSG}")

    if (SKIP_THIS_MKL)
        return()
    endif()

    find_path(MKLINC mkl_cblas.h
        HINTS ${MKLROOT}/include $ENV{MKLROOT}/include)

    # skip full Intel MKL while looking for Intel MKL-ML
    if (MKLINC AND LIBNAME MATCHES "mklml")
        get_filename_component(__mklinc_root "${MKLINC}" PATH)
        find_library(tmp_MKLLIB NAMES "mkl_rt"
            HINTS ${__mklinc_root}/lib/intel64
            NO_DEFAULT_PATH)
        set_if(tmp_MKLLIB MKLINC "")
        unset(tmp_MKLLIB CACHE)
    endif()

    if(NOT MKLINC)
        file(GLOB_RECURSE MKLINC
                ${CMAKE_CURRENT_SOURCE_DIR}/external/*/mkl_cblas.h)
        if(MKLINC)
            # if user has multiple version under external/ then guess last
            # one alphabetically is "latest" and warn
            list(LENGTH MKLINC MKLINCLEN)
            if(MKLINCLEN GREATER 1)
                list(SORT MKLINC)
                list(REVERSE MKLINC)
                list(GET MKLINC 0 MKLINCLST)
                set(MKLINC "${MKLINCLST}")
            endif()
            get_filename_component(MKLINC ${MKLINC} PATH)
        endif()
    endif()
    if(NOT MKLINC)
        return()
    endif()

    get_filename_component(__mklinc_root "${MKLINC}" PATH)

    unset(MKLLIB CACHE) # make find_library to redo the search
    # At first, try to locate Intel MKL in the path where the header was found
    find_library(MKLLIB NAMES ${LIBNAME}
        PATHS ${__mklinc_root}/lib ${__mklinc_root}/lib/intel64
        NO_DEFAULT_PATH)
    # On failure, check the system paths
    find_library(MKLLIB NAMES ${LIBNAME})
    if(NOT MKLLIB)
        return()
    endif()

    if(WIN32)
        set(MKLREDIST ${MKLINC}/../../redist/)
        find_file(MKLDLL NAMES ${LIBNAME}.dll
            HINTS
                ${MKLREDIST}/mkl
                ${MKLREDIST}/intel64/mkl
                ${__mklinc_root}/lib)
        if(NOT MKLDLL)
            return()
        endif()
    endif()

    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        get_filename_component(MKLLIBPATH ${MKLLIB} PATH)
        find_library(MKLIOMP5LIB
            NAMES "iomp5" "iomp5md" "libiomp5" "libiomp5md"
            HINTS   ${MKLLIBPATH}
                    ${MKLLIBPATH}/../../lib
                    ${MKLLIBPATH}/../../../lib/intel64
                    ${MKLLIBPATH}/../../compiler/lib
                    ${MKLLIBPATH}/../../../compiler/lib/intel64)
        if(NOT MKLIOMP5LIB)
            return()
        endif()
        if(WIN32)
            find_file(MKLIOMP5DLL
                NAMES "libiomp5.dll" "libiomp5md.dll"
                HINTS ${MKLREDIST}/../compiler ${__mklinc_root}/lib)
            if(NOT MKLIOMP5DLL)
                return()
            endif()
        endif()
    else()
        set(MKLIOMP5LIB)
        set(MKLIOMP5DLL)
    endif()

    get_filename_component(MKLLIBPATH "${MKLLIB}" PATH)
    string(FIND "${MKLLIBPATH}" ${CMAKE_CURRENT_SOURCE_DIR}/external __idx)
    if(${__idx} EQUAL 0)
        if(WIN32)
            install(PROGRAMS ${MKLDLL} ${MKLIOMP5DLL}
                DESTINATION ${CMAKE_INSTALL_BINDIR})
        else()
            install(PROGRAMS ${MKLLIB} ${MKLIOMP5LIB}
                DESTINATION ${CMAKE_INSTALL_LIBDIR})
        endif()
    endif()

    if(WIN32)
        # Add paths to DLL to %PATH% on Windows
        get_filename_component(MKLDLLPATH "${MKLDLL}" PATH)
        append_to_windows_path_list(CTESTCONFIG_PATH "${MKLDLLPATH}")
        set(CTESTCONFIG_PATH "${CTESTCONFIG_PATH}" PARENT_SCOPE)
    endif()

    # TODO: cache the value
    set(HAVE_MKL TRUE PARENT_SCOPE)
    set(MKLINC ${MKLINC} PARENT_SCOPE)
    set(MKLLIB "${MKLLIB}" PARENT_SCOPE)
    set(MKLDLL "${MKLDLL}" PARENT_SCOPE)
    if(LIBNAME MATCHES "mklml")
        set(MKLDNN_USES_MKL "MKLML:SHARED" PARENT_SCOPE)
    else()
        set(MKLDNN_USES_MKL "FULL:SHARED" PARENT_SCOPE)
    endif()

    set(MKLIOMP5LIB "${MKLIOMP5LIB}" PARENT_SCOPE)
    set(MKLIOMP5DLL "${MKLIOMP5DLL}" PARENT_SCOPE)
endfunction()

function(set_static_mkl_libs libpath)
    set_ternary(lib WIN32 "" "lib")
    set_ternary(a WIN32 ".lib" ".a")

    if (MKLDNN_THREADING STREQUAL "TBB")
        set(thr_name "tbb_thread")
    elseif (MKLDNN_THREADING STREQUAL "OMP:COMP" AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(thr_name "gnu_thread")
    else()
        set(thr_name "intel_thread")
    endif()

    find_library(mkl_iface NAMES "${lib}mkl_intel_lp64${a}" HINTS ${libpath})
    find_library(mkl_thr   NAMES "${lib}mkl_${thr_name}${a}" HINTS ${libpath})
    find_library(mkl_core  NAMES "${lib}mkl_core${a}" HINTS ${libpath})

    set(MKLLIB "${mkl_iface};${mkl_thr};${mkl_core}")
    if (UNIX AND NOT APPLE)
        list(APPEND MKLLIB "${mkl_iface};${mkl_thr};${mkl_core}")
    endif()
    set_if(UNIX MKLLIB "${MKLLIB};m;dl")
    set(MKLLIB "${MKLLIB}" PARENT_SCOPE)
endfunction()

set(MKLDNN_USES_MKL "")
detect_mkl("mklml_intel")
detect_mkl("mklml_gnu")
detect_mkl("mklml")
detect_mkl("mkl_rt")

if(HAVE_MKL)
    if (MKLDNN_USE_MKL STREQUAL "FULL:STATIC")
        set(MKLDLL "")
        get_filename_component(MKLLIBPATH "${MKLLIB}" PATH)
        set_static_mkl_libs(${MKLLIBPATH})
        list(APPEND EXTRA_STATIC_LIBS ${MKLLIB})
        set(MKLDNN_USES_MKL "FULL:STATIC")
    else()
        list(APPEND EXTRA_SHARED_LIBS ${MKLLIB})
    endif()

    add_definitions(-DUSE_MKL -DUSE_CBLAS)
    include_directories(AFTER ${MKLINC})

    set(MSG "Intel(R) MKL:")
    message(STATUS "${MSG} include ${MKLINC}")
    message(STATUS "${MSG} lib ${MKLLIB}")
    if(WIN32 AND MKLDLL)
        message(STATUS "${MSG} dll ${MKLDLL}")
    endif()
else()
    if (MKLDNN_USE_MKL STREQUAL "NONE")
        return()
    endif()

    if (NOT MKLDNN_USE_MKL STREQUAL "DEF")
        set(FAIL_WITHOUT_MKL True)
    endif()

    if(DEFINED ENV{FAIL_WITHOUT_MKL} OR DEFINED FAIL_WITHOUT_MKL)
        set(SEVERITY "FATAL_ERROR")
    else()
        set(SEVERITY "WARNING")
    endif()
    message(${SEVERITY}
        "Intel(R) MKL not found. Some performance features may not be "
        "available. Please run scripts/prepare_mkl.sh to download a minimal "
        "set of libraries or get a full version from "
        "https://software.intel.com/en-us/intel-mkl")
endif()
