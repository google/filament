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

"""Code shared by both CI and try Bazel builders."""

load("@chromium-luci//builders.star", "cpu", "os")

def _apply_linux_bazel_builder_defaults(kwargs):
    kwargs.setdefault("cores", 8)
    kwargs.setdefault("cpu", cpu.X86_64)
    kwargs.setdefault("os", os.LINUX_DEFAULT)
    kwargs.setdefault("ssd", None)
    return kwargs

def _apply_mac_bazel_builder_defaults(kwargs):
    kwargs.setdefault("cpu", "arm64")
    kwargs.setdefault("os", os.MAC_DEFAULT)
    return kwargs

bazel_builder_defaults = struct(
    apply_linux_bazel_builder_defaults = _apply_linux_bazel_builder_defaults,
    apply_mac_bazel_builder_defaults = _apply_mac_bazel_builder_defaults,
)
