# Copyright (C) 2020-2025 The Khronos Group Inc.
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

find_package(Python3 REQUIRED)

set(GLSLANG_INTRINSIC_H          "${GLSLANG_GENERATED_INCLUDEDIR}/glslang/glsl_intrinsic_header.h")
set(GLSLANG_INTRINSIC_PY         "${CMAKE_CURRENT_SOURCE_DIR}/../gen_extension_headers.py")
set(GLSLANG_INTRINSIC_HEADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../glslang/ExtensionHeaders")

add_custom_command(
    OUTPUT  ${GLSLANG_INTRINSIC_H}
    COMMAND Python3::Interpreter "${GLSLANG_INTRINSIC_PY}"
            "-i" ${GLSLANG_INTRINSIC_HEADER_DIR}
            "-o" ${GLSLANG_INTRINSIC_H}
    DEPENDS ${GLSLANG_INTRINSIC_PY}
    COMMENT "Generating ${GLSLANG_INTRINSIC_H}")

set(SOURCES StandAlone.cpp DirStackFileIncluder.h  ${GLSLANG_INTRINSIC_H})

add_executable(glslang-standalone ${SOURCES})
if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    target_compile_options(glslang-standalone PRIVATE -Wconversion)
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" AND NOT MSVC)
    target_compile_options(glslang-standalone PRIVATE -Wshorten-64-to-32)
endif()
set_property(TARGET glslang-standalone PROPERTY FOLDER tools)
set_property(TARGET glslang-standalone PROPERTY OUTPUT_NAME glslang)
glslang_set_link_args(glslang-standalone)

set(LIBRARIES
    glslang
    glslang-default-resource-limits
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)

if(WIN32)
    set(LIBRARIES ${LIBRARIES} psapi)
elseif(UNIX)
    if(NOT ANDROID AND NOT QNX)
        set(LIBRARIES ${LIBRARIES} pthread)
    endif()
endif()

target_link_libraries(glslang-standalone ${LIBRARIES})
target_include_directories(glslang-standalone PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../External>)

if(WIN32)
    source_group("Source" FILES ${SOURCES})
endif()

# Create a symbolic link to glslang named glslangValidator for backwards compatibility
set(legacy_glslang_name "glslangValidator${CMAKE_EXECUTABLE_SUFFIX}")
set(link_method create_symlink)
if(WIN32 OR MINGW)
  set(link_method copy_if_different)
endif()

add_custom_command(
    TARGET glslang-standalone POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E "${link_method}" "\$<TARGET_FILE_NAME:glslang-standalone>" "${legacy_glslang_name}"
    WORKING_DIRECTORY "\$<TARGET_FILE_DIR:glslang-standalone>"
    VERBATIM
)

if(GLSLANG_ENABLE_INSTALL)
    install(TARGETS glslang-standalone EXPORT glslang-targets)

    # Create the same symlink at install time
    install(CODE "\
        message(STATUS \"Installing (${link_method}): \$<TARGET_FILE_NAME:glslang-standalone> -> \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\")
        execute_process(
            COMMAND \"\${CMAKE_COMMAND}\" -E ${link_method} [=[\$<TARGET_FILE_NAME:glslang-standalone>]=] [=[${legacy_glslang_name}]=]
            WORKING_DIRECTORY \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\"
        )
    ")

endif()
