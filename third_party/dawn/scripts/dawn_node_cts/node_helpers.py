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
"""Helper code for running isolated versions of NodeJS."""

import functools
import os
import platform
import sys

THIS_DIR = os.path.dirname(__file__)
DAWN_ROOT = os.path.realpath(os.path.join(THIS_DIR, '..', '..'))

sys.path.insert(0, DAWN_ROOT)

from tools.python import cipd_deps


@functools.cache
def get_node_dir() -> str:
    """Retrieves the directory that NodeJS should be available in."""
    cipd_platform = cipd_deps.get_cipd_platform()
    return os.path.join(DAWN_ROOT, 'third_party', 'node', cipd_platform)


@functools.cache
def get_node_path() -> str:
    """Retrieves the path to the node executable.

    Returns:
        A filepath to the standalone node executable.
    """
    path = os.path.join(get_node_dir(), 'bin', 'node')
    if sys.platform == 'win32':
        path = os.path.join(get_node_dir(), 'node.exe')
    if not os.path.exists(path):
        raise RuntimeError(
            f'Unable to find the node binary under {get_node_dir()}. Is the '
            f'dawn_node gclient variable set?')
    return path


@functools.cache
def get_npm_command() -> list[str]:
    """Retrieves a command to run npm

    Returns:
        A list of strings that will run "npm" when run as a command for a
        subprocess.
    """
    path = os.path.join(get_node_dir(), 'lib', 'node_modules', 'npm', 'bin',
                        'npm-cli.js')
    if sys.platform == 'win32':
        path = os.path.join(get_node_dir(), 'node_modules', 'npm', 'bin',
                            'npm-cli.js')
    if not os.path.exists(path):
        raise RuntimeError(
            f'Unable to find the npm-cli.js file under {get_node_dir()}. Is the '
            f'dawn_node gclient variable set?')
    cmd = [
        get_node_path(),
        os.path.join(get_node_dir(), path),
    ]
    return cmd


@functools.cache
def get_npx_command() -> list[str]:
    """Retrieves a command to run npx.

    Returns:
        A list of strings that will run "npx" when run as a command for a
        subprocess.
    """
    # npx is normally an alias for "npm exec", so just use that instead of
    # looking for npx-cli.js.
    cmd = get_npm_command()
    cmd.append('exec')
    return cmd


def add_node_to_path() -> None:
    """Adds the directory for the standalone node binary to PATH."""
    node_install_dir = os.path.dirname(get_node_path())
    new_path = os.pathsep.join([node_install_dir, os.environ['PATH']])
    os.environ['PATH'] = new_path
