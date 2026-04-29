# Copyright 2026 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Custom function for wrapping Meson based builds, i.e. mesa, in CMake
# rules so that they can be used as part of the Dawn builds. These
# rules invoke meson for actually running the build process, so behave
# like opaque external scripts.
#
# The meson setup stage will run when configuring the CMake
# build. This may take some time, on the order of seconds, and the
# output is buffered, so the CMake setup may look like it has stalled.
#
# The meson compile/install stages will be run with the rest of the
# CMake build commands.

function(meson_target TARGET_NAME)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "" "SOURCE_DIR" "OPTIONS")

    if (NOT ARG_SOURCE_DIR)
        message(FATAL_ERROR "SOURCE_DIR must be defined")
    endif()

    set(MESON_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}")
    set(MESON_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/install")

    set(MESON_ARGS
        "setup"
        "${MESON_BUILD_DIR}"
        "${ARG_SOURCE_DIR}"
        "--prefix=${MESON_INSTALL_DIR}"
    )

    if (ARG_OPTIONS)
        list(APPEND MESON_ARGS ${ARG_OPTIONS})
    endif()

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CC_PATH "${CMAKE_C_COMPILER}")
        set(CXX_PATH "${CMAKE_CXX_COMPILER}")

        # Find llvm-ar and llvm-strip relative to the compiler if possible,
        # otherwise rely on path.
        get_filename_component(COMPILER_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        find_program(LLVM_AR_PATH llvm-ar HINTS "${COMPILER_DIR}")
        find_program(LLVM_STRIP_PATH llvm-strip HINTS "${COMPILER_DIR}")

        if (NOT LLVM_AR_PATH)
            set(LLVM_AR_PATH "llvm-ar")
        endif()
        if (NOT LLVM_STRIP_PATH)
            set(LLVM_STRIP_PATH "llvm-strip")
        endif()

        set(NATIVE_FILE_CONTENT
            "[binaries]\n"
            "c = '${CC_PATH}'\n"
            "cpp = '${CXX_PATH}'\n"
            "ar = '${LLVM_AR_PATH}'\n"
            "strip = '${LLVM_STRIP_PATH}'\n"
            "pkg-config = 'pkg-config'\n"
            "python = ['vpython3', '-vpython-spec', '${PROJECT_SOURCE_DIR}/.vpython3']\n"
        )

        set(NATIVE_FILE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_native.ini")
        file(WRITE "${NATIVE_FILE_PATH}" "${NATIVE_FILE_CONTENT}")

        list(APPEND MESON_ARGS
            "--native-file"
            "${NATIVE_FILE_PATH}"
        )
    endif()

    # Run meson setup at configure time
    execute_process(
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/meson.py" ${MESON_ARGS}
        RESULT_VARIABLE MESON_SETUP_RESULT
    )

    if (NOT MESON_SETUP_RESULT EQUAL 0)
        message(FATAL_ERROR "Meson setup failed for ${TARGET_NAME}")
    endif()

    # Create a custom target for the build step
    add_custom_target(${TARGET_NAME} ALL
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/meson.py" "install" "-C" "${MESON_BUILD_DIR}"
        COMMENT "Building ${TARGET_NAME} with Meson"
    )
endfunction()
