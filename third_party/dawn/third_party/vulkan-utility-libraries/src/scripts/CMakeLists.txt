# ~~~
# Copyright (c) 2023 LunarG, Inc.
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
# ~~~

option(UPDATE_DEPS "Run update_deps.py for user")
if (UPDATE_DEPS)
    find_package(Python3 REQUIRED QUIET)

    set(update_dep_py "${CMAKE_CURRENT_LIST_DIR}/update_deps.py")
    set(known_good_json "${CMAKE_CURRENT_LIST_DIR}/known_good.json")

    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${update_dep_py} ${known_good_json})

    list(APPEND update_dep_command "${update_dep_py}")
    list(APPEND update_dep_command "--generator")
    list(APPEND update_dep_command "${CMAKE_GENERATOR}")

    if (CMAKE_GENERATOR_PLATFORM)
        list(APPEND update_dep_command "--arch")
        list(APPEND update_dep_command "${CMAKE_GENERATOR_PLATFORM}")
    endif()

    if (NOT CMAKE_BUILD_TYPE)
        message(WARNING "CMAKE_BUILD_TYPE not set. Using Debug for dependency build type")
        set(_build_type Debug)
    else()
        set(_build_type ${CMAKE_BUILD_TYPE})
    endif()
    list(APPEND update_dep_command "--config")
    list(APPEND update_dep_command "${_build_type}")
    list(APPEND update_dep_command "--api")
    list(APPEND update_dep_command "${API_TYPE}")

    set(UPDATE_DEPS_DIR_SUFFIX "${_build_type}")
    if (CMAKE_CROSSCOMPILING)
        set(UPDATE_DEPS_DIR_SUFFIX "${CMAKE_SYSTEM_NAME}/${UPDATE_DEPS_DIR_SUFFIX}/${CMAKE_SYSTEM_PROCESSOR}")
    else()
        math(EXPR bitness "8 * ${CMAKE_SIZEOF_VOID_P}")
        set(UPDATE_DEPS_DIR_SUFFIX "${UPDATE_DEPS_DIR_SUFFIX}/${bitness}")
    endif()    
    set(UPDATE_DEPS_DIR "${PROJECT_SOURCE_DIR}/external/${UPDATE_DEPS_DIR_SUFFIX}" CACHE PATH "Location where update_deps.py installs packages")
    list(APPEND update_dep_command "--dir" )
    list(APPEND update_dep_command "${UPDATE_DEPS_DIR}")

    if (NOT BUILD_TESTS)
        list(APPEND update_dep_command "--optional=tests")
    endif()

    if (UPDATE_DEPS_SKIP_EXISTING_INSTALL)
        list(APPEND update_dep_command "--skip-existing-install")
    endif()

    list(APPEND cmake_vars "CMAKE_TOOLCHAIN_FILE")

    # Avoids manually setting CMAKE_SYSTEM_NAME unless it's needed:
    # https://cmake.org/cmake/help/latest/variable/CMAKE_CROSSCOMPILING.html
    if (NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "${CMAKE_HOST_SYSTEM_NAME}")
        list(APPEND cmake_vars "CMAKE_SYSTEM_NAME")
    endif()
    if (APPLE)
        list(APPEND cmake_vars "CMAKE_OSX_ARCHITECTURES" "CMAKE_OSX_DEPLOYMENT_TARGET")
    endif()
    if (NOT MSVC_IDE)
        list(APPEND cmake_vars "CMAKE_CXX_COMPILER" "CMAKE_C_COMPILER" "CMAKE_ASM_COMPILER")
    endif()
    if (ANDROID)
        list(APPEND cmake_vars "ANDROID_PLATFORM" "CMAKE_ANDROID_ARCH_ABI" "CMAKE_ANDROID_STL_TYPE" "CMAKE_ANDROID_RTTI" "CMAKE_ANDROID_EXCEPTIONS" "ANDROID_USE_LEGACY_TOOLCHAIN_FILE")
    endif()

    set(cmake_var)
    foreach(var IN LISTS cmake_vars)
        if (DEFINED ${var})
            list(APPEND update_dep_command "--cmake_var")
            list(APPEND update_dep_command "${var}=${${var}}")
        endif()
    endforeach()

    if (cmake_var)
        list(APPEND update_dep_command "${cmake_var}")
    endif()

    file(TIMESTAMP ${update_dep_py} timestamp_1)
    file(TIMESTAMP ${known_good_json} timestamp_2)

    string("MD5" md5_hash "${timestamp_1}-${timestamp_2}-${update_dep_command}")

    set(UPDATE_DEPS_HASH "0" CACHE STRING "Default value until we run update_deps.py")
    mark_as_advanced(UPDATE_DEPS_HASH)

    if ("${UPDATE_DEPS_HASH}" STREQUAL "0")
        list(APPEND update_dep_command "--clean-build")
        list(APPEND update_dep_command "--clean-install")
    endif()

    if ("${md5_hash}" STREQUAL $CACHE{UPDATE_DEPS_HASH})
        message(DEBUG "update_deps.py: no work to do.")
    else()
        execute_process(
            COMMAND ${Python3_EXECUTABLE} ${update_dep_command}
            RESULT_VARIABLE _update_deps_result
        )
        if (NOT (${_update_deps_result} EQUAL 0))
            message(FATAL_ERROR "Could not run update_deps.py which is necessary to download dependencies.")
        endif()
        set(UPDATE_DEPS_HASH ${md5_hash} CACHE STRING "Ensure we only run update_deps.py when we need to." FORCE)
        include("${UPDATE_DEPS_DIR}/helper.cmake")
    endif()
endif()
if (VULKAN_HEADERS_INSTALL_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${VULKAN_HEADERS_INSTALL_DIR})
    set(CMAKE_REQUIRE_FIND_PACKAGE_VulkanHeaders TRUE PARENT_SCOPE)
endif()
if (GOOGLETEST_INSTALL_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${GOOGLETEST_INSTALL_DIR})
endif()
if (MAGIC_ENUM_INSTALL_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${MAGIC_ENUM_INSTALL_DIR})
endif()

if (CMAKE_CROSSCOMPILING)
    set(CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH} ${CMAKE_PREFIX_PATH} PARENT_SCOPE)
else()
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} PARENT_SCOPE)
endif()
