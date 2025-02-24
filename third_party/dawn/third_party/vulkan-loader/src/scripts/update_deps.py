#!/usr/bin/env python3

# Copyright 2017 The Glslang Authors. All rights reserved.
# Copyright (c) 2018-2023 Valve Corporation
# Copyright (c) 2018-2023 LunarG, Inc.
# Copyright (c) 2023-2023 RasterGrid Kft.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This script was heavily leveraged from KhronosGroup/glslang
# update_glslang_sources.py.
"""update_deps.py

Get and build dependent repositories using known-good commits.

Purpose
-------

This program is intended to assist a developer of this repository
(the "home" repository) by gathering and building the repositories that
this home repository depend on.  It also checks out each dependent
repository at a "known-good" commit in order to provide stability in
the dependent repositories.

Known-Good JSON Database
------------------------

This program expects to find a file named "known-good.json" in the
same directory as the program file.  This JSON file is tailored for
the needs of the home repository by including its dependent repositories.

Program Options
---------------

See the help text (update_deps.py --help) for a complete list of options.

Program Operation
-----------------

The program uses the user's current directory at the time of program
invocation as the location for fetching and building the dependent
repositories.  The user can override this by using the "--dir" option.

For example, a directory named "build" in the repository's root directory
is a good place to put the dependent repositories because that directory
is not tracked by Git. (See the .gitignore file.)  The "external" directory
may also be a suitable location.
A user can issue:

$ cd My-Repo
$ mkdir build
$ cd build
$ ../scripts/update_deps.py

or, to do the same thing, but using the --dir option:

$ cd My-Repo
$ mkdir build
$ scripts/update_deps.py --dir=build

With these commands, the "build" directory is considered the "top"
directory where the program clones the dependent repositories.  The
JSON file configures the build and install working directories to be
within this "top" directory.

Note that the "dir" option can also specify an absolute path:

$ cd My-Repo
$ scripts/update_deps.py --dir=/tmp/deps

The "top" dir is then /tmp/deps (Linux filesystem example) and is
where this program will clone and build the dependent repositories.

Helper CMake Config File
------------------------

When the program finishes building the dependencies, it writes a file
named "helper.cmake" to the "top" directory that contains CMake commands
for setting CMake variables for locating the dependent repositories.
This helper file can be used to set up the CMake build files for this
"home" repository.

A complete sequence might look like:

$ git clone git@github.com:My-Group/My-Repo.git
$ cd My-Repo
$ mkdir build
$ cd build
$ ../scripts/update_deps.py
$ cmake -C helper.cmake ..
$ cmake --build .

JSON File Schema
----------------

There's no formal schema for the "known-good" JSON file, but here is
a description of its elements.  All elements are required except those
marked as optional.  Please see the "known_good.json" file for
examples of all of these elements.

- name

The name of the dependent repository.  This field can be referenced
by the "deps.repo_name" structure to record a dependency.

- api

The name of the API the dependency is specific to (e.g. "vulkan").

- url

Specifies the URL of the repository.
Example: https://github.com/KhronosGroup/Vulkan-Loader.git

- sub_dir

The directory where the program clones the repository, relative to
the "top" directory.

- build_dir

The directory used to build the repository, relative to the "top"
directory.

- install_dir

The directory used to store the installed build artifacts, relative
to the "top" directory.

- commit

The commit used to checkout the repository.  This can be a SHA-1
object name or a refname used with the remote name "origin".

- deps (optional)

An array of pairs consisting of a CMake variable name and a
repository name to specify a dependent repo and a "link" to
that repo's install artifacts.  For example:

"deps" : [
    {
        "var_name" : "VULKAN_HEADERS_INSTALL_DIR",
        "repo_name" : "Vulkan-Headers"
    }
]

which represents that this repository depends on the Vulkan-Headers
repository and uses the VULKAN_HEADERS_INSTALL_DIR CMake variable to
specify the location where it expects to find the Vulkan-Headers install
directory.
Note that the "repo_name" element must match the "name" element of some
other repository in the JSON file.

- prebuild (optional)
- prebuild_linux (optional)  (For Linux and MacOS)
- prebuild_windows (optional)

A list of commands to execute before building a dependent repository.
This is useful for repositories that require the execution of some
sort of "update" script or need to clone an auxillary repository like
googletest.

The commands listed in "prebuild" are executed first, and then the
commands for the specific platform are executed.

- custom_build (optional)

A list of commands to execute as a custom build instead of using
the built in CMake way of building. Requires "build_step" to be
set to "custom"

You can insert the following keywords into the commands listed in
"custom_build" if they require runtime information (like whether the
build config is "Debug" or "Release").

Keywords:
{0} reference to a dictionary of repos and their attributes
{1} reference to the command line arguments set before start
{2} reference to the CONFIG_MAP value of config.

Example:
{2} returns the CONFIG_MAP value of config e.g. debug -> Debug
{1}.config returns the config variable set when you ran update_dep.py
{0}[Vulkan-Headers][repo_root] returns the repo_root variable from
                                   the Vulkan-Headers GoodRepo object.

- cmake_options (optional)

A list of options to pass to CMake during the generation phase.

- ci_only (optional)

A list of environment variables where one must be set to "true"
(case-insensitive) in order for this repo to be fetched and built.
This list can be used to specify repos that should be built only in CI.

- build_step (optional)

Specifies if the dependent repository should be built or not. This can
have a value of 'build', 'custom',  or 'skip'. The dependent repositories are
built by default.

- build_platforms (optional)

A list of platforms the repository will be built on.
Legal options include:
"windows"
"linux"
"darwin"
"android"

Builds on all platforms by default.

Note
----

The "sub_dir", "build_dir", and "install_dir" elements are all relative
to the effective "top" directory.  Specifying absolute paths is not
supported.  However, the "top" directory specified with the "--dir"
option can be a relative or absolute path.

"""

import argparse
import json
import os
import os.path
import subprocess
import sys
import platform
import multiprocessing
import shlex
import shutil
import stat
import time

KNOWN_GOOD_FILE_NAME = 'known_good.json'

CONFIG_MAP = {
    'debug': 'Debug',
    'release': 'Release',
    'relwithdebinfo': 'RelWithDebInfo',
    'minsizerel': 'MinSizeRel'
}

# NOTE: CMake also uses the VERBOSE environment variable. This is intentional.
VERBOSE = os.getenv("VERBOSE")

DEVNULL = open(os.devnull, 'wb')


def on_rm_error( func, path, exc_info):
    """Error handler for recursively removing a directory. The
    shutil.rmtree function can fail on Windows due to read-only files.
    This handler will change the permissions for the file and continue.
    """
    os.chmod( path, stat.S_IWRITE )
    os.unlink( path )

def make_or_exist_dirs(path):
    "Wrapper for os.makedirs that tolerates the directory already existing"
    # Could use os.makedirs(path, exist_ok=True) if we drop python2
    if not os.path.isdir(path):
        os.makedirs(path)

def command_output(cmd, directory):
    # Runs a command in a directory and returns its standard output stream.
    # Captures the standard error stream and prints it an error occurs.
    # Raises a RuntimeError if the command fails to launch or otherwise fails.
    if VERBOSE:
        print('In {d}: {cmd}'.format(d=directory, cmd=cmd))

    result = subprocess.run(cmd, cwd=directory, capture_output=True, text=True)

    if result.returncode != 0:
        print(f'{result.stderr}', file=sys.stderr)
        raise RuntimeError(f'Failed to run {cmd} in {directory}')

    if VERBOSE:
        print(result.stdout)
    return result.stdout

def run_cmake_command(cmake_cmd):
    # NOTE: Because CMake is an exectuable that runs executables
    # stdout/stderr are mixed together. So this combines the outputs
    # and prints them properly in case there is a non-zero exit code.
    result = subprocess.run(cmake_cmd,
        stdout = subprocess.PIPE,
        stderr = subprocess.STDOUT,
        text = True
    )

    if VERBOSE:
        print(result.stdout)
        print(f"CMake command: {cmake_cmd} ", flush=True)

    if result.returncode != 0:
        print(result.stdout, file=sys.stderr)
        sys.exit(result.returncode)

def escape(path):
    return path.replace('\\', '/')

class GoodRepo(object):
    """Represents a repository at a known-good commit."""

    def __init__(self, json, args):
        """Initializes this good repo object.

        Args:
        'json':  A fully populated JSON object describing the repo.
        'args':  Results from ArgumentParser
        """
        self._json = json
        self._args = args
        # Required JSON elements
        self.name = json['name']
        self.url = json['url']
        self.sub_dir = json['sub_dir']
        self.commit = json['commit']
        # Optional JSON elements
        self.build_dir = None
        self.install_dir = None
        if json.get('build_dir'):
            self.build_dir = os.path.normpath(json['build_dir'])
        if json.get('install_dir'):
            self.install_dir = os.path.normpath(json['install_dir'])
        self.deps = json['deps'] if ('deps' in json) else []
        self.prebuild = json['prebuild'] if ('prebuild' in json) else []
        self.prebuild_linux = json['prebuild_linux'] if (
            'prebuild_linux' in json) else []
        self.prebuild_windows = json['prebuild_windows'] if (
            'prebuild_windows' in json) else []
        self.custom_build = json['custom_build'] if ('custom_build' in json) else []
        self.cmake_options = json['cmake_options'] if (
            'cmake_options' in json) else []
        self.ci_only = json['ci_only'] if ('ci_only' in json) else []
        self.build_step = json['build_step'] if ('build_step' in json) else 'build'
        self.build_platforms = json['build_platforms'] if ('build_platforms' in json) else []
        self.optional = set(json.get('optional', []))
        self.api = json['api'] if ('api' in json) else None
        # Absolute paths for a repo's directories
        dir_top = os.path.abspath(args.dir)
        self.repo_dir = os.path.join(dir_top, self.sub_dir)
        if self.build_dir:
            self.build_dir = os.path.join(dir_top, self.build_dir)
        if self.install_dir:
            self.install_dir = os.path.join(dir_top, self.install_dir)

        # By default the target platform is the host platform.
        target_platform = platform.system().lower()
        # However, we need to account for cross-compiling.
        for cmake_var in self._args.cmake_var:
            if "android.toolchain.cmake" in cmake_var:
                target_platform = 'android'

        self.on_build_platform = False
        if self.build_platforms == [] or target_platform in self.build_platforms:
            self.on_build_platform = True

    def Clone(self, retries=10, retry_seconds=60):
        if VERBOSE:
            print('Cloning {n} into {d}'.format(n=self.name, d=self.repo_dir))
        for retry in range(retries):
            make_or_exist_dirs(self.repo_dir)
            try:
                command_output(['git', 'clone', self.url, '.'], self.repo_dir)
                # If we get here, we didn't raise an error
                return
            except RuntimeError as e:
                print("Error cloning on iteration {}/{}: {}".format(retry + 1, retries, e))
                if retry + 1 < retries:
                    if retry_seconds > 0:
                        print("Waiting {} seconds before trying again".format(retry_seconds))
                        time.sleep(retry_seconds)
                    if os.path.isdir(self.repo_dir):
                        print("Removing old tree {}".format(self.repo_dir))
                        shutil.rmtree(self.repo_dir, onerror=on_rm_error)
                    continue

                # If we get here, we've exhausted our retries.
                print("Failed to clone {} on all retries.".format(self.url))
                raise e

    def Fetch(self, retries=10, retry_seconds=60):
        for retry in range(retries):
            try:
                command_output(['git', 'fetch', 'origin'], self.repo_dir)
                # if we get here, we didn't raise an error, and we're done
                return
            except RuntimeError as e:
                print("Error fetching on iteration {}/{}: {}".format(retry + 1, retries, e))
                if retry + 1 < retries:
                    if retry_seconds > 0:
                        print("Waiting {} seconds before trying again".format(retry_seconds))
                        time.sleep(retry_seconds)
                    continue

                # If we get here, we've exhausted our retries.
                print("Failed to fetch {} on all retries.".format(self.url))
                raise e

    def Checkout(self):
        if VERBOSE:
            print('Checking out {n} in {d}'.format(n=self.name, d=self.repo_dir))

        if os.path.exists(os.path.join(self.repo_dir, '.git')):
            url_changed = command_output(['git', 'config', '--get', 'remote.origin.url'], self.repo_dir).strip() != self.url
        else:
            url_changed = False

        if self._args.do_clean_repo or url_changed:
            if os.path.isdir(self.repo_dir):
                if VERBOSE:
                    print('Clearing directory {d}'.format(d=self.repo_dir))
                shutil.rmtree(self.repo_dir, onerror = on_rm_error)
        if not os.path.exists(os.path.join(self.repo_dir, '.git')):
            self.Clone()
        self.Fetch()
        if len(self._args.ref):
            command_output(['git', 'checkout', self._args.ref], self.repo_dir)
        else:
            command_output(['git', 'checkout', self.commit], self.repo_dir)

        if VERBOSE:
            print(command_output(['git', 'status'], self.repo_dir))

    def CustomPreProcess(self, cmd_str, repo_dict):
        return cmd_str.format(repo_dict, self._args, CONFIG_MAP[self._args.config])

    def PreBuild(self):
        """Execute any prebuild steps from the repo root"""
        for p in self.prebuild:
            command_output(shlex.split(p), self.repo_dir)
        if platform.system() == 'Linux' or platform.system() == 'Darwin':
            for p in self.prebuild_linux:
                command_output(shlex.split(p), self.repo_dir)
        if platform.system() == 'Windows':
            for p in self.prebuild_windows:
                command_output(shlex.split(p), self.repo_dir)

    def CustomBuild(self, repo_dict):
        """Execute any custom_build steps from the repo root"""

        # It's not uncommon for builds to not support universal binaries
        if self._args.OSX_ARCHITECTURES:
            print("Universal Binaries not supported for custom builds", file=sys.stderr)
            exit(-1)

        for p in self.custom_build:
            cmd = self.CustomPreProcess(p, repo_dict)
            command_output(shlex.split(cmd), self.repo_dir)

    def CMakeConfig(self, repos):
        """Build CMake command for the configuration phase and execute it"""
        if self._args.do_clean_build:
            if os.path.isdir(self.build_dir):
                shutil.rmtree(self.build_dir, onerror=on_rm_error)
        if self._args.do_clean_install:
            if os.path.isdir(self.install_dir):
                shutil.rmtree(self.install_dir, onerror=on_rm_error)

        # Create and change to build directory
        make_or_exist_dirs(self.build_dir)
        os.chdir(self.build_dir)

        cmake_cmd = [
            'cmake', self.repo_dir,
            '-DCMAKE_INSTALL_PREFIX=' + self.install_dir
        ]

        # Allow users to pass in arbitrary cache variables
        for cmake_var in self._args.cmake_var:
            pieces = cmake_var.split('=', 1)
            cmake_cmd.append('-D{}={}'.format(pieces[0], pieces[1]))

        # For each repo this repo depends on, generate a CMake variable
        # definitions for "...INSTALL_DIR" that points to that dependent
        # repo's install dir.
        for d in self.deps:
            dep_commit = [r for r in repos if r.name == d['repo_name']]
            if len(dep_commit) and dep_commit[0].on_build_platform:
                cmake_cmd.append('-D{var_name}={install_dir}'.format(
                    var_name=d['var_name'],
                    install_dir=dep_commit[0].install_dir))

        # Add any CMake options
        for option in self.cmake_options:
            cmake_cmd.append(escape(option.format(**self.__dict__)))

        # Set build config for single-configuration generators (this is a no-op on multi-config generators)
        cmake_cmd.append(f'-D CMAKE_BUILD_TYPE={CONFIG_MAP[self._args.config]}')

        if self._args.OSX_ARCHITECTURES:
            # CMAKE_OSX_ARCHITECTURES must be a semi-colon seperated list
            cmake_osx_archs = self._args.OSX_ARCHITECTURES.replace(':', ';')
            cmake_cmd.append(f'-D CMAKE_OSX_ARCHITECTURES={cmake_osx_archs}')

        # Use the CMake -A option to select the platform architecture
        # without needing a Visual Studio generator.
        if platform.system() == 'Windows' and self._args.generator != "Ninja":
            cmake_cmd.append('-A')
            if self._args.arch.lower() == '64' or self._args.arch == 'x64' or self._args.arch == 'win64':
                cmake_cmd.append('x64')
            elif self._args.arch == 'arm64':
                cmake_cmd.append('arm64')
            elif self._args.arch == 'arm':
                cmake_cmd.append('arm')
            else:
                cmake_cmd.append('Win32')

        # Apply a generator, if one is specified.  This can be used to supply
        # a specific generator for the dependent repositories to match
        # that of the main repository.
        if self._args.generator is not None:
            cmake_cmd.extend(['-G', self._args.generator])

        # Removes warnings related to unused CLI
        # EX: Setting CMAKE_CXX_COMPILER for a C project
        if not VERBOSE:
            cmake_cmd.append("--no-warn-unused-cli")

        run_cmake_command(cmake_cmd)

    def CMakeBuild(self):
        """Build CMake command for the build phase and execute it"""
        cmake_cmd = ['cmake', '--build', self.build_dir, '--target', 'install', '--config', CONFIG_MAP[self._args.config]]
        if self._args.do_clean:
            cmake_cmd.append('--clean-first')

        # Xcode / Ninja are parallel by default.
        if self._args.generator != "Ninja" or self._args.generator != "Xcode":
            cmake_cmd.append('--parallel')
            cmake_cmd.append(format(multiprocessing.cpu_count()))

        run_cmake_command(cmake_cmd)

    def Build(self, repos, repo_dict):
        """Build the dependent repo and time how long it took"""
        if VERBOSE:
            print('Building {n} in {d}'.format(n=self.name, d=self.repo_dir))
            print('Build dir = {b}'.format(b=self.build_dir))
            print('Install dir = {i}\n'.format(i=self.install_dir))

        start = time.time()

        self.PreBuild()

        if self.build_step == 'custom':
            self.CustomBuild(repo_dict)
        else:
            self.CMakeConfig(repos)
            self.CMakeBuild()

        total_time = time.time() - start

        print(f"Installed {self.name} ({self.commit}) in {total_time} seconds", flush=True)

    def IsOptional(self, opts):
        return len(self.optional.intersection(opts)) > 0

def GetGoodRepos(args):
    """Returns the latest list of GoodRepo objects.

    The known-good file is expected to be in the same
    directory as this script unless overridden by the 'known_good_dir'
    parameter.
    """
    if args.known_good_dir:
        known_good_file = os.path.join( os.path.abspath(args.known_good_dir),
            KNOWN_GOOD_FILE_NAME)
    else:
        known_good_file = os.path.join(
            os.path.dirname(os.path.abspath(__file__)), KNOWN_GOOD_FILE_NAME)
    with open(known_good_file) as known_good:
        return [
            GoodRepo(repo, args)
            for repo in json.loads(known_good.read())['repos']
        ]


def GetInstallNames(args):
    """Returns the install names list.

    The known-good file is expected to be in the same
    directory as this script unless overridden by the 'known_good_dir'
    parameter.
    """
    if args.known_good_dir:
        known_good_file = os.path.join(os.path.abspath(args.known_good_dir),
            KNOWN_GOOD_FILE_NAME)
    else:
        known_good_file = os.path.join(
            os.path.dirname(os.path.abspath(__file__)), KNOWN_GOOD_FILE_NAME)
    with open(known_good_file) as known_good:
        install_info = json.loads(known_good.read())
        if install_info.get('install_names'):
            return install_info['install_names']
        else:
            return None


def CreateHelper(args, repos, filename):
    """Create a CMake config helper file.

    The helper file is intended to be used with 'cmake -C <file>'
    to build this home repo using the dependencies built by this script.

    The install_names dictionary represents the CMake variables used by the
    home repo to locate the install dirs of the dependent repos.
    This information is baked into the CMake files of the home repo and so
    this dictionary is kept with the repo via the json file.
    """
    install_names = GetInstallNames(args)
    with open(filename, 'w') as helper_file:
        for repo in repos:
            # If the repo has an API tag and that does not match
            # the target API then skip it
            if repo.api is not None and repo.api != args.api:
                continue
            if install_names and repo.name in install_names and repo.on_build_platform:
                helper_file.write('set({var} "{dir}" CACHE STRING "")\n'
                                  .format(
                                      var=install_names[repo.name],
                                      dir=escape(repo.install_dir)))


def main():
    parser = argparse.ArgumentParser(
        description='Get and build dependent repos at known-good commits')
    parser.add_argument(
        '--known_good_dir',
        dest='known_good_dir',
        help="Specify directory for known_good.json file.")
    parser.add_argument(
        '--dir',
        dest='dir',
        default='.',
        help="Set target directory for repository roots. Default is \'.\'.")
    parser.add_argument(
        '--ref',
        dest='ref',
        default='',
        help="Override 'commit' with git reference. E.g., 'origin/main'")
    parser.add_argument(
        '--no-build',
        dest='do_build',
        action='store_false',
        help=
        "Clone/update repositories and generate build files without performing compilation",
        default=True)
    parser.add_argument(
        '--clean',
        dest='do_clean',
        action='store_true',
        help="Clean files generated by compiler and linker before building",
        default=False)
    parser.add_argument(
        '--clean-repo',
        dest='do_clean_repo',
        action='store_true',
        help="Delete repository directory before building",
        default=False)
    parser.add_argument(
        '--clean-build',
        dest='do_clean_build',
        action='store_true',
        help="Delete build directory before building",
        default=False)
    parser.add_argument(
        '--clean-install',
        dest='do_clean_install',
        action='store_true',
        help="Delete install directory before building",
        default=False)
    parser.add_argument(
        '--skip-existing-install',
        dest='skip_existing_install',
        action='store_true',
        help="Skip build if install directory exists",
        default=False)
    parser.add_argument(
        '--arch',
        dest='arch',
        choices=['32', '64', 'x86', 'x64', 'win32', 'win64', 'arm', 'arm64'],
        type=str.lower,
        help="Set build files architecture (Visual Studio Generator Only)",
        default='64')
    parser.add_argument(
        '--config',
        dest='config',
        choices=['debug', 'release', 'relwithdebinfo', 'minsizerel'],
        type=str.lower,
        help="Set build files configuration",
        default='debug')
    parser.add_argument(
        '--api',
        dest='api',
        default='vulkan',
        choices=['vulkan'],
        help="Target API")
    parser.add_argument(
        '--generator',
        dest='generator',
        help="Set the CMake generator",
        default=None)
    parser.add_argument(
        '--optional',
        dest='optional',
        type=lambda a: set(a.lower().split(',')),
        help="Comma-separated list of 'optional' resources that may be skipped. Only 'tests' is currently supported as 'optional'",
        default=set())
    parser.add_argument(
        '--cmake_var',
        dest='cmake_var',
        action='append',
        metavar='VAR[=VALUE]',
        help="Add CMake command line option -D'VAR'='VALUE' to the CMake generation command line; may be used multiple times",
        default=[])
    parser.add_argument(
        '--osx-archs',
        dest='OSX_ARCHITECTURES',
        help="Architectures when building a universal binary. Takes a colon seperated list. Ex: arm64:x86_64",
        type=str,
        default=None)

    args = parser.parse_args()
    save_cwd = os.getcwd()

    if args.OSX_ARCHITECTURES:
        print(f"Building dependencies as universal binaries targeting {args.OSX_ARCHITECTURES}")

    # Create working "top" directory if needed
    make_or_exist_dirs(args.dir)
    abs_top_dir = os.path.abspath(args.dir)

    repos = GetGoodRepos(args)
    repo_dict = {}

    print('Starting builds in {d}'.format(d=abs_top_dir))
    for repo in repos:
        # If the repo has an API tag and that does not match
        # the target API then skip it
        if repo.api is not None and repo.api != args.api:
            continue

        # If the repo has a platform whitelist, skip the repo
        # unless we are building on a whitelisted platform.
        if not repo.on_build_platform:
            continue

        # Skip building the repo if its install directory already exists
        # and requested via an option.  This is useful for cases where the
        # install directory is restored from a cache that is known to be up
        # to date.
        if args.skip_existing_install and os.path.isdir(repo.install_dir):
            print('Skipping build for repo {n} due to existing install directory'.format(n=repo.name))
            continue

        # Skip test-only repos if the --tests option was not passed in
        if repo.IsOptional(args.optional):
            continue

        field_list = ('url',
                      'sub_dir',
                      'commit',
                      'build_dir',
                      'install_dir',
                      'deps',
                      'prebuild',
                      'prebuild_linux',
                      'prebuild_windows',
                      'custom_build',
                      'cmake_options',
                      'ci_only',
                      'build_step',
                      'build_platforms',
                      'repo_dir',
                      'on_build_platform')
        repo_dict[repo.name] = {field: getattr(repo, field) for field in field_list}

        # If the repo has a CI whitelist, skip the repo unless
        # one of the CI's environment variable is set to true.
        if len(repo.ci_only):
            do_build = False
            for env in repo.ci_only:
                if env not in os.environ:
                    continue
                if os.environ[env].lower() == 'true':
                    do_build = True
                    break
            if not do_build:
                continue

        # Clone/update the repository
        repo.Checkout()

        # Build the repository
        if args.do_build and repo.build_step != 'skip':
            repo.Build(repos, repo_dict)

    # Need to restore original cwd in order for CreateHelper to find json file
    os.chdir(save_cwd)
    CreateHelper(args, repos, os.path.join(abs_top_dir, 'helper.cmake'))

    sys.exit(0)


if __name__ == '__main__':
    main()

