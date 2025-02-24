# Copyright 2020 The SwiftShader Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(ROOT_PROJECT_COMPILE_OPTIONS
    ${SWIFTSHADER_COMPILE_OPTIONS}
    ${WARNINGS_AS_ERRORS}
)

set(SYSTEM_SRC_FILES
    Build.cpp
    Build.hpp
    Configurator.cpp
    Configurator.hpp
    CPUID.cpp
    CPUID.hpp
    Debug.cpp
    Debug.hpp
    Half.cpp
    Half.hpp
    LRUCache.hpp
    Math.cpp
    Math.hpp
    Memory.cpp
    Memory.hpp
    SharedLibrary.hpp
    Socket.cpp
    Socket.hpp
    Synchronization.hpp
    SwiftConfig.cpp
    SwiftConfig.hpp
    Timer.cpp
    Timer.hpp
    Types.hpp
)

if(LINUX OR ANDROID)
    list(APPEND SYSTEM_SRC_FILES
        Linux/MemFd.cpp
        Linux/MemFd.hpp
    )
endif()

set(SYSTEM_COMPILE_OPTIONS "")
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND SYSTEM_COMPILE_OPTIONS
        "-Wexit-time-destructors"  # declaration requires an exit-time destructor
    )
    # We use exit-time destructors for the global configuration.
    SET_SOURCE_FILES_PROPERTIES("SwiftConfig.cpp" PROPERTIES COMPILE_FLAGS "-Wno-exit-time-destructors")
endif()

add_library(vk_system EXCLUDE_FROM_ALL
    ${SYSTEM_SRC_FILES}
)

set_target_properties(vk_system PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    FOLDER "SwiftShader VK"
)

target_include_directories(vk_system
    PUBLIC
        ".."
)

target_compile_options(vk_system
    PRIVATE
        ${ROOT_PROJECT_COMPILE_OPTIONS}
        ${SYSTEM_COMPILE_OPTIONS}
)

target_link_libraries(vk_system
    PUBLIC
        marl
)

target_link_options(vk_system
    PUBLIC
        ${SWIFTSHADER_LINK_FLAGS}
)
