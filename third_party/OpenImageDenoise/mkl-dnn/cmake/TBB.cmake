#===============================================================================
# Copyright 2009-2018 Intel Corporation
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

if(TBB_cmake_included)
    return()
endif()
set(TBB_cmake_included true)
#include("cmake/Threading.cmake")

if(NOT MKLDNN_THREADING STREQUAL "TBB")
    return()
endif()

if(NOT TBB_ROOT)
    set(TBB_ROOT $ENV{TBB_ROOT})
endif()
if(NOT TBB_ROOT)
    set(TBB_ROOT $ENV{TBBROOT})
endif()

if(WIN32)
    # workaround for parentheses in variable name / CMP0053
    set(PROGRAMFILESx86 "PROGRAMFILES(x86)")
    set(PROGRAMFILES32 "$ENV{${PROGRAMFILESx86}}")
    if(NOT PROGRAMFILES32)
        set(PROGRAMFILES32 "$ENV{PROGRAMFILES}")
    endif()
    if(NOT PROGRAMFILES32)
        set(PROGRAMFILES32 "C:/Program Files (x86)")
    endif()
    find_path(TBB_ROOT include/tbb/tbb.h
        DOC "Root of TBB installation"
        PATHS ${PROJECT_SOURCE_DIR}/tbb
        NO_DEFAULT_PATH
    )
    find_path(TBB_ROOT include/tbb/tbb.h
        HINTS ${TBB_ROOT}
        PATHS
            ${PROJECT_SOURCE_DIR}/../tbb
            "${PROGRAMFILES32}/IntelSWTools/compilers_and_libraries/windows/tbb"
            "${PROGRAMFILES32}/Intel/Composer XE/tbb"
            "${PROGRAMFILES32}/Intel/compilers_and_libraries/windows/tbb"
    )

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(TBB_ARCH intel64)
    else()
        set(TBB_ARCH ia32)
    endif()

    if(MSVC10)
        set(TBB_VCVER vc10)
    elseif(MSVC11)
        set(TBB_VCVER vc11)
    elseif(MSVC12)
        set(TBB_VCVER vc12)
    else()
        set(TBB_VCVER vc14)
    endif()

    if(TBB_ROOT STREQUAL "")
        find_path(TBB_INCLUDE_DIR tbb/task_scheduler_init.h)
        find_path(TBB_BIN_DIR tbb.dll)
        find_library(TBB_LIBRARY tbb)
        find_library(TBB_LIBRARY_MALLOC tbbmalloc)
    else()
        set(TBB_INCLUDE_DIR TBB_INCLUDE_DIR-NOTFOUND)
        set(TBB_BIN_DIR TBB_BIN_DIR-NOTFOUND)
        set(TBB_LIBRARY TBB_LIBRARY-NOTFOUND)
        set(TBB_LIBRARY_MALLOC TBB_LIBRARY_MALLOC-NOTFOUND)
        find_path(TBB_INCLUDE_DIR tbb/task_scheduler_init.h PATHS ${TBB_ROOT}/include NO_DEFAULT_PATH)
        find_path(TBB_BIN_DIR tbb.dll
            HINTS
                ${TBB_ROOT}/bin/${TBB_ARCH}/${TBB_VCVER}
                ${TBB_ROOT}/bin
                ${TBB_ROOT}/../redist/${TBB_ARCH}/tbb/${TBB_VCVER}
                ${TBB_ROOT}/../redist/${TBB_ARCH}_win/tbb/${TBB_VCVER}
            NO_DEFAULT_PATH
        )
        set(TBB_LIB_DIR ${TBB_ROOT}/lib/${TBB_ARCH}/${TBB_VCVER})
        find_library(TBB_LIBRARY tbb PATHS ${TBB_LIB_DIR} ${TBB_ROOT}/lib NO_DEFAULT_PATH)
        find_library(TBB_LIBRARY_MALLOC tbbmalloc PATHS ${TBB_LIB_DIR} ${TBB_ROOT}/lib NO_DEFAULT_PATH)
    endif()

else()

    find_path(TBB_ROOT include/tbb/tbb.h
        DOC "Root of TBB installation"
        PATHS ${PROJECT_SOURCE_DIR}/tbb
        NO_DEFAULT_PATH
    )
    find_path(TBB_ROOT include/tbb/tbb.h
        DOC "Root of TBB installation"
        HINTS ${TBB_ROOT}
        PATHS
            ${PROJECT_SOURCE_DIR}/tbb
            /opt/intel/composerxe/tbb
            /opt/intel/compilers_and_libraries/tbb
            /opt/intel/tbb
    )

    if(TBB_ROOT STREQUAL "")
        find_path(TBB_INCLUDE_DIR tbb/task_scheduler_init.h)
        find_library(TBB_LIBRARY tbb)
        find_library(TBB_LIBRARY_MALLOC tbbmalloc)
        
    elseif(EXISTS ${TBB_ROOT}/cmake/TBBBuild.cmake AND EXISTS ${TBB_ROOT}/src/tbb/tbb_version.h)
        option(TBB_STATIC_LIB "Build TBB as a static library (building TBB as a static library is NOT recommended)")
        if(TBB_STATIC_LIB)
            include(${TBB_ROOT}/cmake/TBBBuild.cmake)
            tbb_build(TBB_ROOT ${TBB_ROOT} CONFIG_DIR TBB_DIR MAKE_ARGS extra_inc=big_iron.inc)
            set(TBB_INCLUDE_DIR ${TBB_ROOT}/include)
            set(TBB_LIBRARY ${PROJECT_BINARY_DIR}/tbb_cmake_build/tbb_cmake_build_subdir_release/libtbb.a)
            set(TBB_LIBRARY_MALLOC ${PROJECT_BINARY_DIR}/tbb_cmake_build/tbb_cmake_build_subdir_release/libtbbmalloc.a)
        else()
            include(${TBB_ROOT}/cmake/TBBBuild.cmake)
            tbb_build(TBB_ROOT ${TBB_ROOT} CONFIG_DIR TBB_DIR)
            set(TBB_INCLUDE_DIR ${TBB_ROOT}/include)
            set(TBB_LIBRARY ${PROJECT_BINARY_DIR}/tbb_cmake_build/tbb_cmake_build_subdir_release/libtbb.so.2)
            set(TBB_LIBRARY_MALLOC ${PROJECT_BINARY_DIR}/tbb_cmake_build/tbb_cmake_build_subdir_release/libtbbmalloc.so.2)
        endif()
        
    else()
        set(TBB_INCLUDE_DIR TBB_INCLUDE_DIR-NOTFOUND)
        set(TBB_LIBRARY TBB_LIBRARY-NOTFOUND)
        set(TBB_LIBRARY_MALLOC TBB_LIBRARY_MALLOC-NOTFOUND)
        if(APPLE)
            find_path(TBB_INCLUDE_DIR tbb/task_scheduler_init.h PATHS ${TBB_ROOT}/include NO_DEFAULT_PATH)
            find_library(TBB_LIBRARY tbb PATHS ${TBB_ROOT}/lib NO_DEFAULT_PATH)
            find_library(TBB_LIBRARY_MALLOC tbbmalloc PATHS ${TBB_ROOT}/lib NO_DEFAULT_PATH)
        else()
            find_path(TBB_INCLUDE_DIR tbb/task_scheduler_init.h PATHS ${TBB_ROOT}/include NO_DEFAULT_PATH)
            set(TBB_HINTS HINTS ${TBB_ROOT}/lib/intel64/gcc4.4 ${TBB_ROOT}/lib ${TBB_ROOT}/lib64 PATHS /usr/libx86_64-linux-gnu/)
            find_library(TBB_LIBRARY tbb ${TBB_HINTS})
            find_library(TBB_LIBRARY_MALLOC tbbmalloc ${TBB_HINTS})
        endif()
    endif()

endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TBB DEFAULT_MSG TBB_INCLUDE_DIR TBB_LIBRARY TBB_LIBRARY_MALLOC)

if(TBB_FOUND)
    add_library(TBB::tbb SHARED IMPORTED)
    set_target_properties(TBB::tbb PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${TBB_INCLUDE_DIR}
        INTERFACE_COMPILE_DEFINITIONS "__TBB_NO_IMPLICIT_LINKAGE=1"
    )

    add_library(TBB::tbbmalloc SHARED IMPORTED)
    set_target_properties(TBB::tbbmalloc PROPERTIES
        INTERFACE_COMPILE_DEFINITIONS "__TBBMALLOC_NO_IMPLICIT_LINKAGE=1"
    )

    if(WIN32)
        set_target_properties(TBB::tbb PROPERTIES
            IMPORTED_IMPLIB ${TBB_LIBRARY}
        )

        set_target_properties(TBB::tbbmalloc PROPERTIES
            IMPORTED_IMPLIB ${TBB_LIBRARY_MALLOC}
        )
    else()
        set_target_properties(TBB::tbb PROPERTIES
            IMPORTED_LOCATION ${TBB_LIBRARY}
            IMPORTED_NO_SONAME TRUE
        )

        set_target_properties(TBB::tbbmalloc PROPERTIES
            IMPORTED_LOCATION ${TBB_LIBRARY_MALLOC}
            IMPORTED_NO_SONAME TRUE
        )
    endif()


    set(TBB_LIBRARIES TBB::tbb TBB::tbbmalloc)

    set_threading("TBB")
    list(APPEND EXTRA_SHARED_LIBS ${TBB_LIBRARIES})
endif()

mark_as_advanced(TBB_INCLUDE_DIR)
mark_as_advanced(TBB_LIBRARY)
mark_as_advanced(TBB_LIBRARY_MALLOC)

