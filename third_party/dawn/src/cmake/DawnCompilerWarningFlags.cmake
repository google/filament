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

# This module requires CMake 3.19 features (the `CheckCompilerFlag`
# module). Just skip it for older CMake versions and let the warnings appear.
if (CMAKE_VERSION VERSION_LESS "3.19")
  return ()
endif ()

include(CheckCompilerFlag)

function (dawn_add_flag flag)
  foreach (lang IN LISTS ARGN)
    check_compiler_flag("${lang}" "${flag}" "dawn_have_compiler_flag-${lang}-${flag}")
    if (dawn_have_compiler_flag-${lang}-${flag})
      target_compile_options(dawn_internal_config
        INTERFACE
          "$<BUILD_INTERFACE:$<$<COMPILE_LANGUAGE:${lang}>:${flag}>>")
    endif ()
  endforeach ()
endfunction ()

set(langs C CXX)

dawn_add_flag("-Wconditional-uninitialized" ${langs})
dawn_add_flag("-Wcstring-format-directive" ${langs})
dawn_add_flag("-Wctad-maybe-unsupported" ${langs})
dawn_add_flag("-Wc++11-narrowing" ${langs})
dawn_add_flag("-Wdeprecated-copy" ${langs})
dawn_add_flag("-Wdeprecated-copy-dtor" ${langs})
dawn_add_flag("-Wduplicate-enum" ${langs})
dawn_add_flag("-Wextra-semi" ${langs})
dawn_add_flag("-Wextra-semi-stmt" ${langs})
dawn_add_flag("-Wimplicit-fallthrough" ${langs})
dawn_add_flag("-Winconsistent-missing-destructor-override" ${langs})
dawn_add_flag("-Winvalid-offsetof" ${langs})
dawn_add_flag("-Wmissing-field-initializers" ${langs})
dawn_add_flag("-Wnon-c-typedef-for-linkage" ${langs})
dawn_add_flag("-Wpessimizing-move" ${langs})
dawn_add_flag("-Wrange-loop-analysis" ${langs})
dawn_add_flag("-Wredundant-move" ${langs})
dawn_add_flag("-Wshadow-field" ${langs})
dawn_add_flag("-Wstrict-prototypes" ${langs})
dawn_add_flag("-Wsuggest-destructor-override" ${langs})
dawn_add_flag("-Wsuggest-override" ${langs})
dawn_add_flag("-Wtautological-unsigned-zero-compare" ${langs})
dawn_add_flag("-Wunreachable-code-aggressive" ${langs})
dawn_add_flag("-Wunused-but-set-variable" ${langs})
dawn_add_flag("-Wunused-macros" ${langs})

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if (CMAKE_CXX_COMPILER_VERSION_MAJOR VERSION_LESS_EQUAL 18)
    dawn_add_flag("-Wno-gcc-compat" ${langs})
  endif ()
  if (WIN32)
    dawn_add_flag("/clang:-pedantic" ${langs})
    # Allow the use of __uuidof()
    dawn_add_flag("-Wno-language-extension-token" ${langs})
  else ()
    dawn_add_flag("-pedantic" ${langs})
  endif ()
endif ()

if (MSVC)
  # Dawn extends wgpu enums with internal enums.
  # MSVC considers these invalid switch values. crbug.com/dawn/397.
  dawn_add_flag("/wd4063" CXX)
  # MSVC things that a switch over all the enum values of an enum class is
  # not sufficient to cover all control paths. Turn off this warning so that
  # the respective clang warning tells us where to add switch cases
  # (otherwise we have to add default: DAWN_UNREACHABLE() that silences clang too)
  dawn_add_flag("/wd4715" CXX)
  # /ZW makes sure we don't add calls that are forbidden in UWP.
  # and /EHsc is required to be used in combination with it,
  # even if it is already added by the windows GN defaults,
  # we still add it to make every /ZW paired with a /EHsc
  dawn_add_flag("/ZW:nostdlib" CXX)
  dawn_add_flag("/EHsc" CXX)
endif ()
