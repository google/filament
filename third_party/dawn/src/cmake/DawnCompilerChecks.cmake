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

# Preserved for the c++ module interface to match the compile config
# of the consuming project
if (DEFINED CMAKE_CXX_STANDARD)
  set(DAWN_TOPLEVEL_CXX_STANDARD ${CMAKE_CXX_STANDARD})
else ()
  set(DAWN_TOPLEVEL_CXX_STANDARD 20)
endif ()
if (DEFINED CMAKE_CXX_EXTENSIONS)
  set(DAWN_TOPLEVEL_CXX_EXTENSIONS ${CMAKE_CXX_EXTENSIONS})
else ()
  set(DAWN_TOPLEVEL_CXX_EXTENSIONS ${CMAKE_CXX_EXTENSIONS_DEFAULT})
endif ()


# Make sure we have C++20 enabled.
# Needed to make sure libraries and executables not built by the
# dawn_add_library still have the C++20 compiler flags enabled
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

# Prevent scanning headers for module dependencies
# Also needed for the module compile check below
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Check C++20 module support
# Ref: https://cmake.org/cmake/help/latest/manual/cmake-cxxmodules.7.html
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.28
  # Supported generators
  AND
  ((CMAKE_GENERATOR MATCHES "Ninja") OR
   (CMAKE_GENERATOR MATCHES "^Visual Studio ([0-9]+) [0-9]+$"
    AND CMAKE_MATCH_1 GREATER_EQUAL 17))
  # AppleClang, VisualStudio and certain Linux distros
  # don't bundle clang-scan-deps
  AND
  ((CMAKE_CXX_COMPILER_ID MATCHES Clang
   AND CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS)
   OR NOT CMAKE_CXX_COMPILER_ID MATCHES Clang))

  include(CheckCXXSourceCompiles)
  if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -fmodules-ts")
  endif()
  check_cxx_source_compiles([[
    module;
    export module test;
    extern "C++" int main() {}
  ]] DAWN_SUPPORTS_CXX_MODULES)
else()
  set(DAWN_SUPPORTS_CXX_MODULES False)
endif()
