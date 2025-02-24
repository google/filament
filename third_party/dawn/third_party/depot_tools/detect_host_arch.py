#!/usr/bin/env python3
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Outputs host CPU architecture in format recognized by gyp."""

from functools import lru_cache
import platform
import re


@lru_cache(maxsize=None)
def HostArch():
    """Returns the host architecture with a predictable string."""
    host_arch = platform.machine().lower()
    host_processor = platform.processor().lower()

    # Convert machine type to format recognized by gyp.
    if re.match(r'i.86', host_arch) or host_arch == 'i86pc':
        host_arch = 'x86'
    elif host_arch in ['x86_64', 'amd64']:
        host_arch = 'x64'
    elif host_arch == 'arm64' or host_arch.startswith('aarch64'):
        host_arch = 'arm64'
    elif host_arch.startswith('arm'):
        host_arch = 'arm'
    elif host_arch.startswith('mips64'):
        host_arch = 'mips64'
    elif host_arch.startswith('mips'):
        host_arch = 'mips'
    elif host_arch.startswith('ppc') or host_processor == 'powerpc':
        host_arch = 'ppc'
    elif host_arch.startswith('s390'):
        host_arch = 's390'
    elif host_arch.startswith('riscv'):
        host_arch = 'riscv64'
    elif platform.system() == 'OS/390':
        host_arch = 's390x'

    if host_arch == 'arm64':
        host_platform = platform.architecture()
        if len(host_platform) > 1:
            if host_platform[1].lower() == 'windowspe':
                # Special case for Windows on Arm: windows-386 packages no
                # longer work so use the x64 emulation (this restricts us to
                # Windows 11). Python 32-bit returns the host_arch as arm64,
                # 64-bit does not.
                return 'x64'

    # platform.machine is based on running kernel. It's possible to use 64-bit
    # kernel with 32-bit userland, e.g. to give linker slightly more memory.
    # Distinguish between different userland bitness by querying
    # the python binary.
    if host_arch == 'x64' and platform.architecture()[0] == '32bit':
        host_arch = 'x86'
    if host_arch == 'arm64' and platform.architecture()[0] == '32bit':
        host_arch = 'arm'

    return host_arch


def DoMain(_):
    """Hook to be called from gyp without starting a separate python
    interpreter."""
    return HostArch()


if __name__ == '__main__':
    print(DoMain([]))
