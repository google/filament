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

set(SPIRV_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/GlslangToSpv.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InReadableOrder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SpvBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SpvPostProcess.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/doc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SpvTools.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/disassemble.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CInterface/spirv_c_interface.cpp
    PARENT_SCOPE)

set(SPIRV_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/bitutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv.hpp11
    ${CMAKE_CURRENT_SOURCE_DIR}/GLSL.std.450.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GLSL.ext.EXT.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GLSL.ext.KHR.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GlslangToSpv.h
    ${CMAKE_CURRENT_SOURCE_DIR}/hex_float.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SpvBuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/spvIR.h
    ${CMAKE_CURRENT_SOURCE_DIR}/spvUtil.h
    ${CMAKE_CURRENT_SOURCE_DIR}/doc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SpvTools.h
    ${CMAKE_CURRENT_SOURCE_DIR}/disassemble.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GLSL.ext.AMD.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GLSL.ext.NV.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GLSL.ext.ARM.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GLSL.ext.QCOM.h
    ${CMAKE_CURRENT_SOURCE_DIR}/NonSemanticDebugPrintf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/NonSemanticShaderDebugInfo100.h
    PARENT_SCOPE)

set(PUBLIC_HEADERS
    GlslangToSpv.h
    disassemble.h
    Logger.h
    spirv.hpp11
    SpvTools.h)

add_library(SPIRV ${LIB_TYPE} ${CMAKE_CURRENT_SOURCE_DIR}/../glslang/stub.cpp)
add_library(glslang::SPIRV ALIAS SPIRV)
set_target_properties(SPIRV PROPERTIES
    FOLDER glslang
    POSITION_INDEPENDENT_CODE ON
    VERSION   "${GLSLANG_VERSION}"
    SOVERSION "${GLSLANG_VERSION_MAJOR}")
target_include_directories(SPIRV PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
glslang_only_export_explicit_symbols(SPIRV)

if(WIN32 AND BUILD_SHARED_LIBS)
    set_target_properties(SPIRV PROPERTIES PREFIX "")
endif()

if(ENABLE_OPT)
    target_link_libraries(SPIRV INTERFACE glslang PUBLIC SPIRV-Tools-opt)
    target_include_directories(SPIRV PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../External>)
else()
    target_link_libraries(SPIRV INTERFACE glslang)
endif()

if(WIN32)
    source_group("Source" FILES ${SOURCES} ${HEADERS})
endif()

if(GLSLANG_ENABLE_INSTALL)
    install(TARGETS SPIRV EXPORT glslang-targets)

    install(FILES ${PUBLIC_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/glslang/SPIRV/)
endif()
