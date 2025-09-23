# Copyright 2025 The Dawn & Tint Authors
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

set(HERMETICXCODE_DAWN_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../..")
set(HERMETICXCODE_LLVM      "${HERMETICXCODE_DAWN_ROOT}/third_party/llvm-build/Release+Asserts/bin")
set(HERMETICXCODE_DEVELOPER "${HERMETICXCODE_DAWN_ROOT}/build/mac_files/xcode_binaries/Contents/Developer")

set(CMAKE_C_COMPILER        "${HERMETICXCODE_LLVM}/clang")
set(CMAKE_CXX_COMPILER      "${HERMETICXCODE_LLVM}/clang++")
set(CMAKE_AR                "${HERMETICXCODE_LLVM}/llvm-ar")
set(CMAKE_INSTALL_NAME_TOOL "${HERMETICXCODE_LLVM}/llvm-install-name-tool")

# ranlib is just a symlink to libtool in system CommandLineTools, so we have our
# own correctly-name symlink to make this work
set(CMAKE_RANLIB            "${CMAKE_CURRENT_LIST_DIR}/ranlib")

set(CMAKE_LINKER            "${HERMETICXCODE_DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/ld")
set(CMAKE_NM                "${HERMETICXCODE_DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/llvm-nm")
set(CMAKE_OBJDUMP           "${HERMETICXCODE_DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/llvm-objdump")
set(CMAKE_STRIP             "${HERMETICXCODE_DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip")

set(CMAKE_OSX_SYSROOT       "${HERMETICXCODE_DEVELOPER}/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk")
