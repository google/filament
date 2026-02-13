# Copyright 2023 The Dawn & Tint Authors
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
"""
Helper script to download Dawn's source dependencies without the need to
install depot_tools by manually. This script implements a subset of
`gclient sync`.

This helps embedders, for example through CMake, get all the sources with
a single add_subdirectory call (or FetchContent) instead of more complex setups

Note that this script executes blindly the content of DEPS file, run it only on
a project that you trust not to contain malicious DEPS files.
"""

import os
import sys
import subprocess
import argparse
from pathlib import Path

parser = argparse.ArgumentParser(
    prog='fetch_dawn_dependencies',
    description=__doc__,
)

parser.add_argument('-d',
                    '--directory',
                    type=str,
                    default="",
                    help="""
    Working directory, in which we read and apply DEPS files recusively. If not
    specified, the current working directory is used.
    """)

parser.add_argument('-g',
                    '--git',
                    type=str,
                    default="git",
                    help="""
    Path to the git command used to. By default, git is retrieved from the PATH.
    You may also use this option to specify extra argument for all calls to git.
    """)

parser.add_argument('-s',
                    '--shallow',
                    action='store_true',
                    default=True,
                    help="""
    Clone repositories without commit history (only getting data for the
    requested commit).
    NB: The git server hosting the dependencies must have turned on the
    `uploadpack.allowReachableSHA1InWant` option.
    NB2: git submodules may not work as expected (but they are not used by Dawn
    dependencies).
    """)
parser.add_argument('-ns',
                    '--no-shallow',
                    action='store_false',
                    dest='shallow',
                    help="Deactivate shallow cloning.")

parser.add_argument('-t',
                    '--use-test-deps',
                    action='store_true',
                    default=False,
                    help="""
    Deprecated: Test dependencies are now always included.
    """)


def main(args):
    # The dependencies that we need to pull from the DEPS files.
    # Dependencies of dependencies are prefixed by their ancestors.
    required_submodules = [
        'third_party/abseil-cpp',
        'third_party/dxc',
        'third_party/dxheaders',
        'third_party/glfw',
        'third_party/jinja2',
        'third_party/khronos/EGL-Registry',
        'third_party/khronos/OpenGL-Registry',
        'third_party/libprotobuf-mutator/src',
        'third_party/protobuf',
        'third_party/markupsafe',
        'third_party/glslang/src',
        'third_party/google_benchmark/src',
        'third_party/googletest',
        'third_party/spirv-headers/src',
        'third_party/spirv-tools/src',
        'third_party/vulkan-headers/src',
        'third_party/vulkan-loader/src',
        'third_party/vulkan-utility-libraries/src',
    ]

    root_dir = Path(args.directory).resolve()

    process_dir(args, root_dir, required_submodules)


def process_dir(args, dir_path, required_submodules):
    """
    Install dependencies for the provided directory by processing the DEPS file
    that it contains (if it exists).
    Recursively install dependencies in sub-directories that are created by
    cloning dependencies.
    """
    deps_path = dir_path / 'DEPS'
    if not deps_path.is_file():
        return

    log(f"Listing dependencies from {dir_path}")
    DEPS = open(deps_path).read()

    ldict = {}
    exec(DEPS, globals(), ldict)
    deps = ldict.get('deps')
    variables = ldict.get('vars', {})

    if deps is None:
        log(f"WARNING: DEPS file '{deps_path}' does not define a 'deps' variable"
            )
        return

    for submodule in required_submodules:
        if submodule not in deps:
            continue
        submodule_path = dir_path / Path(submodule)

        raw_url = deps[submodule]['url']
        git_url, git_tag = raw_url.format(**variables).rsplit('@', 1)

        # Run git from within the submodule's path (don't use for clone)
        git = lambda *x: subprocess.run([args.git, '-C', submodule_path, *x],
                                        capture_output=True)

        log(f"Fetching dependency '{submodule}'")
        if (submodule_path / ".git").is_dir():
            # The module was already cloned, but we may need to update it
            proc = git('rev-parse', 'HEAD')
            need_update = proc.stdout.decode().strip() != git_tag

            if need_update:
                # The module was already cloned, but we may need to update it
                proc = git('cat-file', '-t', git_tag)
                git_tag_exists = proc.returncode == 0

                if not git_tag_exists:
                    log(f"Updating '{submodule_path}' from '{git_url}'")
                    if args.shallow:
                        git('fetch', 'origin', git_tag, '--depth', '1')
                    else:
                        git('fetch', 'origin')

                log(f"Checking out tag '{git_tag}'")
                git('checkout', git_tag)

        else:
            if args.shallow:
                log(f"Shallow cloning '{git_url}' at '{git_tag}' into '{submodule_path}'"
                    )
                shallow_clone(git, git_url, git_tag)
            else:
                log(f"Cloning '{git_url}' into '{submodule_path}'")
                subprocess.run([
                    args.git,
                    'clone',
                    '--recurse-submodules',
                    git_url,
                    submodule_path,
                ],
                               capture_output=True)

            log(f"Checking out tag '{git_tag}'")
            git('checkout', git_tag)

        # Recursive call
        required_subsubmodules = [
            m[len(submodule) + 1:] for m in required_submodules
            if m.startswith(submodule + "/")
        ]
        process_dir(args, submodule_path, required_subsubmodules)


def shallow_clone(git, git_url, git_tag):
    """
    Fetching only 1 commit is not exposed in the git clone API, so we decompose
    it manually in git init, git fetch, git reset.
    """
    git('init')
    git('fetch', git_url, git_tag, '--depth', '1')


def log(msg):
    """Just makes it look good in the CMake log flow."""
    print(f"-- -- {msg}")


class Var:
    """
    Mock Var class, that the content of DEPS files assume to exist when they
    are exec-ed.
    """
    def __init__(self, name):
        self.name = name

    def __add__(self, text):
        return self.name + text

    def __radd__(self, text):
        return text + self.name


if __name__ == "__main__":
    main(parser.parse_args())
