# Copyright (C) 2020 The Khronos Group Inc.
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#    Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
#    Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#
#    Neither the name of The Khronos Group Inc. nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

add_library(glslang-default-resource-limits
            ${CMAKE_CURRENT_SOURCE_DIR}/ResourceLimits.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/resource_limits_c.cpp)
set_property(TARGET glslang-default-resource-limits PROPERTY FOLDER glslang)
set_property(TARGET glslang-default-resource-limits PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(glslang-default-resource-limits
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>)


set(SOURCES StandAlone.cpp DirStackFileIncluder.h)

add_executable(glslangValidator ${SOURCES})
set_property(TARGET glslangValidator PROPERTY FOLDER tools)
glslang_set_link_args(glslangValidator)

set(LIBRARIES
    glslang
    SPIRV
    glslang-default-resource-limits)

if(ENABLE_SPVREMAPPER)
    set(LIBRARIES ${LIBRARIES} SPVRemapper)
endif()

if(WIN32)
    set(LIBRARIES ${LIBRARIES} psapi)
elseif(UNIX)
    if(NOT ANDROID)
        set(LIBRARIES ${LIBRARIES} pthread)
    endif()
endif(WIN32)

target_link_libraries(glslangValidator ${LIBRARIES})
target_include_directories(glslangValidator PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../External>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/External>)

if(ENABLE_SPVREMAPPER)
    set(REMAPPER_SOURCES spirv-remap.cpp)
    add_executable(spirv-remap ${REMAPPER_SOURCES})
    set_property(TARGET spirv-remap PROPERTY FOLDER tools)
    glslang_set_link_args(spirv-remap)
    target_link_libraries(spirv-remap ${LIBRARIES})
endif()

if(WIN32)
    source_group("Source" FILES ${SOURCES})
endif(WIN32)

if(ENABLE_GLSLANG_INSTALL)
    install(TARGETS glslangValidator EXPORT glslangValidatorTargets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(EXPORT glslangValidatorTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake)

    if(ENABLE_SPVREMAPPER)
        install(TARGETS spirv-remap EXPORT spirv-remapTargets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
        install(EXPORT spirv-remapTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake)
    endif()

    if(BUILD_SHARED_LIBS)
        install(TARGETS glslang-default-resource-limits EXPORT glslang-default-resource-limitsTargets
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
        install(EXPORT glslang-default-resource-limitsTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake)
    endif()
endif(ENABLE_GLSLANG_INSTALL)
