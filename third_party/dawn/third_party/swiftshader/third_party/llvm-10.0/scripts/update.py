#!/usr/bin/env python3

# Copyright 2018 The SwiftShader Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import contextlib
import multiprocessing
import os
import platform
import re
import shutil
import subprocess
import sys
import tempfile
from os import path

# LLVM_BRANCH must match the value of the same variable in third_party/update-llvm-10.sh
LLVM_BRANCH = "release/10.x"

# LLVM_COMMIT must be set to the commit hash that we last updated to when running third_party/update-llvm-10.sh.
# Run 'git show -s origin/llvm10-clean' and look for 'llvm-10-update: <hash>' to retrieve it.
LLVM_COMMIT = "d32170dbd5b0d54436537b6b75beaf44324e0c28"

SCRIPT_DIR = path.dirname(path.realpath(sys.argv[0]))
LLVM_STAGING_DIR = path.abspath(path.join(tempfile.gettempdir(), "llvm-10"))
LLVM_DIR = path.abspath(path.join(LLVM_STAGING_DIR, "llvm"))
LLVM_OBJS = path.join(LLVM_STAGING_DIR, "build")
LLVM_CONFIGS = path.abspath(path.join(SCRIPT_DIR, '..', 'configs'))

# List of all arches SwiftShader supports
LLVM_TARGETS = [
    ('AArch64', ('__aarch64__',)),
    ('ARM', ('__arm__',)),
    ('X86', ('__i386__', '__x86_64__')),
    ('Mips', ('__mips__',)),
    ('PowerPC', ('__powerpc64__',)),
    ('RISCV', ('__riscv',)),
]

# Per-platform arches
LLVM_TRIPLES = {
    'android': [
        ('__x86_64__', 'x86_64-linux-android'),
        ('__i386__', 'i686-linux-android'),
        ('__arm__', 'armv7-linux-androideabi'),
        ('__aarch64__', 'aarch64-linux-android'),
    ],
    'linux': [
        ('__x86_64__', 'x86_64-unknown-linux-gnu'),
        ('__i386__', 'i686-pc-linux-gnu'),
        ('__arm__', 'armv7-linux-gnueabihf'),
        ('__aarch64__', 'aarch64-linux-gnu'),
        ('__mips__', 'mipsel-linux-gnu'),
        ('__mips64', 'mips64el-linux-gnuabi64'),
        ('__powerpc64__', 'powerpc64le-unknown-linux-gnu'),
        ('__riscv', 'riscv64-unknown-linux-gnu'),
    ],
    'darwin': [
        ('__x86_64__', 'x86_64-apple-darwin'),
        ('__aarch64__', 'arm64-apple-darwin'),
    ],
    'windows': [
        ('__x86_64__', 'x86_64-pc-win32'),
        ('__i386__', 'i686-pc-win32'),
        ('__arm__', 'armv7-pc-win32'),
        ('__aarch64__', 'aarch64-pc-win32'),
        ('__mips__', 'mipsel-pc-win32'),
        ('__mips64', 'mips64el-pc-win32'),
    ],
    'fuchsia': [
        ('__x86_64__', 'x86_64-unknown-fuchsia'),
        ('__aarch64__', 'aarch64-unknown-fuchsia'),
    ]
}

# Mapping of target platform to the host it must be built on
LLVM_PLATFORM_TO_HOST_SYSTEM = {
    'android': 'Linux',
    'darwin': 'Darwin',
    'linux': 'Linux',
    'windows': 'Windows',
    'fuchsia': 'Linux'
}

# LLVM configurations to be undefined.
LLVM_UNDEF_MACROS = [
    'BACKTRACE_HEADER',
    'ENABLE_BACKTRACES',
    'ENABLE_CRASH_OVERRIDES',
    'HAVE_BACKTRACE',
    'HAVE_POSIX_SPAWN',
    'HAVE_PTHREAD_GETNAME_NP',
    'HAVE_PTHREAD_SETNAME_NP',
    'HAVE_TERMIOS_H',
    'HAVE_ZLIB_H',
    'HAVE__UNWIND_BACKTRACE',
]

# General CMake options for building LLVM
LLVM_CMAKE_OPTIONS = [
    '-DCMAKE_BUILD_TYPE=Release',
    '-DLLVM_ENABLE_THREADS=ON',
    '-DLLVM_ENABLE_TERMINFO=OFF',
    '-DLLVM_ENABLE_LIBXML2=OFF',
    '-DLLVM_ENABLE_LIBEDIT=OFF',
    '-DLLVM_ENABLE_LIBPFM=OFF',
    '-DLLVM_ENABLE_ZLIB=OFF',
    '-DLLVM_TEMPORARILY_ALLOW_OLD_TOOLCHAIN=ON'
]

# Used when building LLVM for darwin. Affects values set in the generated config files.
MIN_MACOS_VERSION = '10.10'

@contextlib.contextmanager
def pushd(new_dir):
    previous_dir = os.getcwd()
    os.chdir(new_dir)
    try:
        yield
    finally:
        os.chdir(previous_dir)


def log(message, level=1):
    print(' ' * level + '> ' + message)


def run_command(command, log_level=1):
    log(command, log_level)
    os.system(command)


def run_subprocess(*popenargs, log_level=1, cwd=None):
    log([' '.join(t) for t in popenargs][0], log_level)
    return subprocess.run(*popenargs, cwd=cwd)


def _parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('name', help='destination name',
                        choices=['android', 'linux', 'darwin', 'windows', 'fuchsia'])
    parser.add_argument('-j', '--jobs', help='parallel compilation', type=int)
    return parser.parse_args()


def validate_args(args):
    host = platform.system()
    if host not in LLVM_PLATFORM_TO_HOST_SYSTEM.values():
        raise Exception(f"Host system not supported: '{host}'")

    if args.name not in LLVM_PLATFORM_TO_HOST_SYSTEM.keys():
        raise Exception(f"Unknown target platform: '{args.name}'")

    expected_host = LLVM_PLATFORM_TO_HOST_SYSTEM[args.name]
    if LLVM_PLATFORM_TO_HOST_SYSTEM[args.name] != host:
        raise Exception(
            f"Target platform '{args.name}' must be built on '{expected_host}', not on '{host}'")


def get_cmake_targets_to_build(platform):
    """Returns list of LLVM targets to build for the input platform"""
    targets = set()
    for arch_def, triplet in LLVM_TRIPLES[platform]:
        for arch, defs in LLVM_TARGETS:
            if arch_def in defs:
                targets.add(arch)

    # Maintain the sort order of LLVM_TARGETS as this affects how config
    # headers are generated
    return [t[0] for t in LLVM_TARGETS if t[0] in targets]


def clone_llvm():
    log("Cloning/Updating LLVM", 1)
    # Clone or update staging directory
    if not path.exists(LLVM_STAGING_DIR):
        os.mkdir(LLVM_STAGING_DIR)
        with pushd(LLVM_STAGING_DIR):
            run_command('git init', 2)
            run_command(
                'git remote add origin https://github.com/llvm/llvm-project.git', 2)
            run_command('git config core.sparsecheckout true', 2)
            run_command('echo /llvm > .git/info/sparse-checkout', 2)

    with pushd(LLVM_STAGING_DIR):
        run_command('echo /llvm > .git/info/sparse-checkout', 2)
        run_command('git fetch origin {}'.format(LLVM_BRANCH), 2)
        run_command('git checkout {}'.format(LLVM_COMMIT), 2)
    return


def build_llvm(name, num_jobs):
    """Build LLVM and get all generated files."""
    log("Building LLVM", 1)
    if num_jobs is None:
        num_jobs = multiprocessing.cpu_count()

    """On Windows we need to have CMake generate build files for the 64-bit
    Visual Studio host toolchain."""
    host = '-Thost=x64' if name == 'windows' else ''

    cmake_options = LLVM_CMAKE_OPTIONS + ['-DLLVM_TARGETS_TO_BUILD=' +
                                          ';'.join(t for t in get_cmake_targets_to_build(name))]

    if name == 'darwin':
        cmake_options.append('-DCMAKE_OSX_DEPLOYMENT_TARGET={}'.format(MIN_MACOS_VERSION))

    os.makedirs(LLVM_OBJS, exist_ok=True)
    run_subprocess(['cmake', host, LLVM_DIR] +
                   cmake_options, log_level=2, cwd=LLVM_OBJS)
    run_subprocess(['cmake', '--build', '.', '-j',
                    str(num_jobs)], log_level=2, cwd=LLVM_OBJS)


def list_files(src_base, src, dst_base, suffixes):
    """Enumerate the files that are under `src` and end with one of the
    `suffixes` and yield the source path and the destination path."""
    src_base = path.abspath(src_base)
    src = path.join(src_base, src)
    for base_dir, dirnames, filenames in os.walk(src):
        for filename in filenames:
            if path.splitext(filename)[1] in suffixes:
                relative = path.relpath(base_dir, src_base)
                yield (path.join(base_dir, filename),
                       path.join(dst_base, relative, filename))


def copy_common_generated_files(dst_base):
    """Copy platform-independent generated files."""
    log("Copying platform-independent generated files", 1)
    suffixes = {'.inc', '.h', '.def'}
    subdirs = [
        path.join('include', 'llvm', 'IR'),
        path.join('include', 'llvm', 'Support'),
        path.join('lib', 'IR'),
        path.join('lib', 'Transforms', 'InstCombine'),
    ] + [path.join('lib', 'Target', arch) for arch, defs in LLVM_TARGETS]
    for subdir in subdirs:
        for src, dst in list_files(LLVM_OBJS, subdir, dst_base, suffixes):
            log('{} -> {}'.format(src, dst), 2)
            os.makedirs(path.dirname(dst), exist_ok=True)
            shutil.copyfile(src, dst)


def copy_platform_file(platform, src, dst):
    """Copy platform-dependent generated files and add platform-specific
    modifications."""

    # LLVM configuration patterns to be post-processed.
    llvm_target_pattern = re.compile('^LLVM_[A-Z_]+\\(([A-Za-z0-9_]+)\\)$')
    llvm_native_pattern = re.compile(
        '^#define LLVM_NATIVE_([A-Z]+) (LLVMInitialize)?(.*)$')
    llvm_triple_pattern = re.compile('^#define (LLVM_[A-Z_]+_TRIPLE) "(.*)"$')
    llvm_define_pattern = re.compile('^#define ([A-Za-z0-9_]+) (.*)$')

    # Build architecture-specific conditions.
    conds = {}
    for arch, defs in LLVM_TARGETS:
        conds[arch] = ' || '.join('defined(' + v + ')' for v in defs)

    # Get a set of platform-specific triples.
    triples = LLVM_TRIPLES[platform]

    with open(src, 'r') as src_file:
        os.makedirs(path.dirname(dst), exist_ok=True)
        with open(dst, 'w') as dst_file:
            for line in src_file:
                if line == '#define LLVM_CONFIG_H\n':
                    print(line, file=dst_file, end='')
                    print('', file=dst_file)
                    print('#if !defined(__i386__) && defined(_M_IX86)',
                          file=dst_file)
                    print('#define __i386__ 1', file=dst_file)
                    print('#endif', file=dst_file)
                    print('', file=dst_file)
                    print(
                        '#if !defined(__x86_64__) && (defined(_M_AMD64) || defined (_M_X64))', file=dst_file)
                    print('#define __x86_64__ 1', file=dst_file)
                    print('#endif', file=dst_file)
                    print('', file=dst_file)

                match = llvm_target_pattern.match(line)
                if match:
                    arch = match.group(1)
                    print('#if ' + conds[arch], file=dst_file)
                    print(line, file=dst_file, end='')
                    print('#endif', file=dst_file)
                    continue

                match = llvm_native_pattern.match(line)
                if match:
                    name = match.group(1)
                    init = match.group(2) or ''
                    arch = match.group(3)
                    end = ''
                    if arch.lower().endswith(name.lower()):
                        end = arch[-len(name):]
                    directive = '#if '
                    for arch, defs in LLVM_TARGETS:
                        print(directive + conds[arch], file=dst_file)
                        print('#define LLVM_NATIVE_' + name + ' ' +
                              init + arch + end, file=dst_file)
                        directive = '#elif '
                    print('#else', file=dst_file)
                    print('#error "unknown architecture"', file=dst_file)
                    print('#endif', file=dst_file)
                    continue

                match = llvm_triple_pattern.match(line)
                if match:
                    name = match.group(1)
                    directive = '#if'
                    for defs, triple in triples:
                        print(directive + ' defined(' + defs + ')',
                              file=dst_file)
                        print('#define ' + name + ' "' + triple + '"',
                              file=dst_file)
                        directive = '#elif'
                    print('#else', file=dst_file)
                    print('#error "unknown architecture"', file=dst_file)
                    print('#endif', file=dst_file)
                    continue

                match = llvm_define_pattern.match(line)
                if match and match.group(1) in LLVM_UNDEF_MACROS:
                    print('/* #undef ' + match.group(1) + ' */', file=dst_file)
                    continue

                print(line, file=dst_file, end='')


def copy_platform_generated_files(platform, dst_base):
    """Copy platform-specific generated files."""
    log("Copying platform-specific generated files", 1)
    suffixes = {'.inc', '.h', '.def'}
    src_dir = path.join('include', 'llvm', 'Config')
    for src, dst in list_files(LLVM_OBJS, src_dir, dst_base, suffixes):
        log('{}, {} -> {}'.format(platform, src, dst), 2)
        copy_platform_file(platform, src, dst)


def main():
    args = _parse_args()
    validate_args(args)
    clone_llvm()
    build_llvm(args.name, args.jobs)
    copy_common_generated_files(path.join(LLVM_CONFIGS, 'common'))
    copy_platform_generated_files(
        args.name, path.join(LLVM_CONFIGS, args.name))


if __name__ == '__main__':
    main()
