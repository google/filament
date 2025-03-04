# Copyright 2024 The Dawn & Tint Authors
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

if (${DAWN_ENABLE_TSAN})
  add_compile_options(-stdlib=libc++)
  add_link_options(-stdlib=libc++)
endif ()

################################################################################
# common_compile_options - sets compiler and linker options common for dawn
################################################################################
function(common_compile_options target)
  if (COMPILER_IS_LIKE_GNU)
    target_compile_options(${target}
      PRIVATE
        "-fno-exceptions"
        "-fno-rtti"

        "-Wno-deprecated-builtins"
        "-Wno-unknown-warning-option"
        "-Wno-switch-default"
    )
    if (${DAWN_WERROR})
      target_compile_options(${target}
        PRIVATE "-Werror")
    endif ()
  endif ()

  if (MSVC)
    target_compile_options(${target}
      PUBLIC "/utf-8")
  endif ()

  if (COMPILER_IS_LIKE_GNU)
    set(SANITIZER_OPTIONS "")
    if (${DAWN_ENABLE_MSAN})
      list(APPEND SANITIZER_OPTIONS "-fsanitize=memory")
    endif ()
    if (${DAWN_ENABLE_ASAN})
      list(APPEND SANITIZER_OPTIONS "-fsanitize=address")
    endif ()
    if (${DAWN_ENABLE_TSAN})
      list(APPEND SANITIZER_OPTIONS "-fsanitize=thread")
    endif ()
    if (${DAWN_ENABLE_UBSAN})
      list(APPEND SANITIZER_OPTIONS
        "-fsanitize=undefined"
        "-fsanitize=float-divide-by-zero")
    endif ()
    if (SANITIZER_OPTIONS)
      target_compile_options(${target}
        PUBLIC ${SANITIZER_OPTIONS})
      target_link_options(${target}
        PUBLIC ${SANITIZER_OPTIONS})
    endif ()
  endif ()

  option(DAWN_EMIT_COVERAGE "Emit code coverage information" OFF)
  if (DAWN_EMIT_COVERAGE)
    target_compile_definitions(${target}
      PRIVATE "DAWN_EMIT_COVERAGE")
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
      target_compile_options(${target}
        PRIVATE "--coverage")
      target_link_options(${target}
        PRIVATE "gcov")
    elseif (COMPILER_IS_CLANG OR COMPILER_IS_CLANG_CL)
      target_compile_options(${target}
        PRIVATE "-fprofile-instr-generate" "-fcoverage-mapping")
      target_link_options(${target}
        PRIVATE "-fprofile-instr-generate" "-fcoverage-mapping")
    else()
      message(FATAL_ERROR
        "Coverage generation not supported for the ${CMAKE_CXX_COMPILER_ID} toolchain")
    endif ()
  endif ()
endfunction()
