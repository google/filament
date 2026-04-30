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
"""Helper code for interacting with CIPD packages pulled in through DEPS."""

import functools
import platform
import sys


@functools.cache
def get_cipd_platform() -> str:
    """Retrieves the CIPD platform for the current host.

  The returned string is compatible with CIPD's package naming scheme.
  """
    cipd_os = get_cipd_compatible_current_os()
    cipd_arch = get_cipd_compatible_current_arch()
    return f'{cipd_os}-{cipd_arch}'


@functools.cache
def get_cipd_compatible_current_os() -> str:
    """Retrieves the current OS name.

    The returned string is compatible with CIPD's package naming scheme.
    """
    current_platform = sys.platform
    if current_platform in ('linux', 'cygwin'):
        return 'linux'
    if current_platform == 'win32':
        return 'windows'
    if current_platform == 'darwin':
        return 'mac'
    raise ValueError(f'Unknown platform {current_platform}')


@functools.cache
def get_cipd_compatible_current_arch() -> str:
    """Retrieves the current architecture.

    The returned string is compatible with CIPD's package naming scheme.
    """
    native_arm = platform.machine().lower() in ('arm', 'arm64')
    # This is necessary for the case of running x86 Python on arm devices via
    # an emulator. In this case, platform.machine() will show up as an x86
    # processor.
    emulated_x86 = 'armv8' in platform.processor().lower()
    if native_arm or emulated_x86:
        return 'arm64'

    native_x86 = platform.machine().lower() in ('x86', 'x86_64', 'amd64')
    if native_x86:
        return 'amd64'
    raise ValueError('Unable to determine architecture')
