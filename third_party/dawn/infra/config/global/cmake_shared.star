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

"""Code shared by both CI and try CMake builders."""

load("@chromium-luci//builders.star", "cpu", "os")

def _apply_linux_cmake_builder_defaults(kwargs):
    kwargs.setdefault("cpu", cpu.X86_64)

    # n2-standard-8 is specifically targeted for Linux/CMake instead of the more
    # common e2-standard-8 because RBE is not currently supported for CMake
    # builds. The newer CPUs used by n2-standard-8 GCE instances result in
    # significantly faster local compile times. e4-standard-8 will be the
    # standard going forward for all builders and appears to have equivalent
    # CMake build times to n2-standard-8, so allow that as well until
    # all n2-standard-8 machines are phased out.
    kwargs.setdefault("machine_type", "n2-standard-8|e4-standard-8")
    kwargs.setdefault("os", os.LINUX_DEFAULT)
    kwargs.setdefault("ssd", None)
    return kwargs

def _apply_mac_cmake_builder_defaults(kwargs):
    kwargs.setdefault("caches", [swarming.cache(name = "osx_sdk", path = "cache/osx_sdk")])

    # x64 used for the builders since historically Dawn has tested Mac/CMake on
    # x64 and tests are run on the same machine as compilation.
    kwargs.setdefault("cpu", cpu.X86_64)
    kwargs.setdefault("os", os.MAC_15)
    return kwargs

def _apply_win_cmake_builder_defaults(kwargs):
    kwargs.setdefault("cpu", cpu.X86_64)

    # n2-standard-8 is specifically targeted for Win/CMake instead of the more
    # common e2-standard-8 because Windows compilation takes the most time and
    # the use of MSVC means that RBE is unsupported for remote compilation. The
    # newer CPUs used by n2-standard-8 GCE instances result in significantly
    # faster compile times. e4-standard-8 will be the standard going forward for
    # all builders and appears to have equivalent CMake build times to
    # n2-standard-8, so allow that as well until all n2-standard-8 machines are
    # phased out.
    kwargs.setdefault("machine_type", "n2-standard-8|e4-standard-8")
    kwargs.setdefault("os", os.WINDOWS_DEFAULT)
    kwargs.setdefault("ssd", None)
    return kwargs

cmake_builder_defaults = struct(
    apply_linux_cmake_builder_defaults = _apply_linux_cmake_builder_defaults,
    apply_mac_cmake_builder_defaults = _apply_mac_cmake_builder_defaults,
    apply_win_cmake_builder_defaults = _apply_win_cmake_builder_defaults,
)
