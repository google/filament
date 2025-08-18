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

# CMake < 3.15 sets /W3 in CMAKE_CXX_FLAGS. Remove it if it's there.
# See https://gitlab.kitware.com/cmake/cmake/-/issues/18317
if (CMAKE_VERSION VERSION_LESS 3.15.0)
  if (MSVC)
    if (CMAKE_CXX_FLAGS MATCHES "/W3")
      string(REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif ()
  endif ()
endif ()

if (TINT_CHECK_CHROMIUM_STYLE)
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -Xclang -add-plugin -Xclang find-bad-constructs")
endif ()

if ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    AND (CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
  set(COMPILER_IS_CLANG_CL TRUE)
endif ()

if ((CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang") OR
    ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") AND
     (NOT COMPILER_IS_CLANG_CL)))
  set(COMPILER_IS_CLANG TRUE)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(COMPILER_IS_GNU TRUE)
endif ()

if ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR COMPILER_IS_CLANG)
  set(COMPILER_IS_LIKE_GNU TRUE)
endif ()

# Enable msbuild multiprocessor builds
if (MSVC AND NOT COMPILER_IS_CLANG_CL)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif ()

if (TARGET_MACOS)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum macOS version")
endif ()