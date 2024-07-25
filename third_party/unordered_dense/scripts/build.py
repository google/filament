#!/bin/env python

import os
from pathlib import Path
import subprocess


cmd_and_dir = [
    # needs honggfuzz installed
    ['env', 'CXX_LD=mold', 'CXX=ccache clang++', 'meson', 'setup', '--buildtype', 'debug',   '-Dcpp_std=c++17', 'builddir/clang_cpp17_debug'],
    ['env', 'CXX_LD=mold', 'CXX=ccache clang++', 'meson', 'setup', '--buildtype', 'release', '-Dcpp_std=c++17', 'builddir/clang_cpp17_release'],
    ['env', 'CXX_LD=mold', 'CXX=ccache g++',     'meson', 'setup', '--buildtype', 'release', '-Dcpp_std=c++17', 'builddir/gcc_cpp17_release'],
    ['env', 'CXX_LD=mold', 'CXX=ccache hfuzz-clang++ -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION', 'meson', 'setup', '--buildtype', 'release', '-Dcpp_std=c++17', 'builddir/hfuzz-clang_cpp17_release'],
    ['env', 'CXX_LD=mold', 'CXX=ccache g++',     'meson', 'setup', '--buildtype', 'debug',   '-Dcpp_std=c++17', 'builddir/gcc_cpp17_debug'],

    # 32bit. Install lib32-clang
    ['env', 'CXX_LD=mold', 'CXX=ccache g++',     'meson', 'setup', '--buildtype', 'debug', '-Dcpp_std=c++17', '-Dcpp_args=-m32', '-Dcpp_link_args=-m32', '-Dc_args=-m32', '-Dc_link_args=-m32', 'builddir/gcc_cpp17_debug_32'],
    ['env', 'CXX_LD=mold', 'CXX=ccache clang++', 'meson', 'setup', '--buildtype', 'debug', '-Dcpp_std=c++17', '-Dcpp_args=-m32', '-Dcpp_link_args=-m32', '-Dc_args=-m32', '-Dc_link_args=-m32', 'builddir/clang_cpp17_debug_32'],

    # c++20
    ['env', 'CXX_LD=mold', 'CXX=ccache clang++', 'meson', 'setup', '--buildtype', 'debug', '-Dcpp_std=c++20', 'builddir/clang_cpp20_debug'],
    ['env', 'CXX_LD=mold', 'CXX=ccache g++',     'meson', 'setup', '--buildtype', 'debug', '-Dcpp_std=c++20', 'builddir/gcc_cpp20_debug'],

    # coverage; use "ninja clean && ninja test && ninja coverage"
    #['env', 'CXX_LD=mold', 'CXX=ccache clang++', 'meson', 'setup', '-Db_coverage=true', 'builddir/coverage'],

    # sanitizers
    # It is not possible to combine more than one of the -fsanitize=address, -fsanitize=thread, and -fsanitize=memory checkers in the same program.
    # see https://clang.llvm.org/docs/UsersManual.html#controlling-code-generation
    #
    # can't use ccache, it doesn't work with the ignorelist.txt
    ['env', 'CXX_LD=mold', 'CXX=ccache g++',     'meson', 'setup', '-Db_sanitize=address',   'builddir/gcc_sanitize_address'],
    ['env', 'CXX_LD=mold', 'CXX=ccache clang++', 'meson', 'setup', '-Db_sanitize=address',   'builddir/clang_sanitize_address'],

    ['env', 'CXX_LD=mold', 'CXX=ccache g++',     'meson', 'setup', '-Db_sanitize=thread',    'builddir/gcc_sanitize_thread'],
    ['env', 'CXX_LD=mold', 'CXX=ccache clang++', 'meson', 'setup', '-Db_sanitize=thread',    'builddir/clang_sanitize_thread'],

    # ['env', 'CXX_LD=mold', 'CXX=ccache g++',     'meson', 'setup', '-Db_sanitize=memory',    'builddir/gcc_sanitize_memory'], # doesn't work due to STL, and ignore doesn't work either :-(
    # ['env', 'CXX_LD=mold', 'CXX=ccache clang++', 'meson', 'setup', '-Db_sanitize=memory',    'builddir/clang_sanitize_memory'], # doesn't work due to STL, and ignore doesn't work either :-(

    ['env', 'CXX_LD=mold', 'CXX=ccache g++',     'meson', 'setup', '-Db_sanitize=undefined', 'builddir/gcc_sanitize_undefined'],
    ['env', 'CXX_LD=mold', 'CXX=ccache clang++', 'meson', 'setup', '-Db_sanitize=undefined', 'builddir/clang_sanitize_undefined'],
]

root_path = Path(__file__).parent.parent
os.chdir(root_path)


def run(cmd):
    result = subprocess.run(cmd)
    if result.returncode != 0:
        exit(result.returncode)

run('scripts/lint/lint-all.py')

for cmd_dir in cmd_and_dir:
    workdir = cmd_dir[-1]

    # setup
    if not os.path.isdir(workdir):
        out = run(cmd_dir)
    
    # clean
    run(['meson', 'compile', '--clean', '-C', workdir])

    # compile everything
    run(['meson', 'compile', '-C', workdir])

    # test
    #if workdir.find("clang_cpp17_debug") != -1:
    #    run(['meson', 'test', '--wrap=\'valgrind --leak-check=full --error-exitcode=1\'', '-q', '--print-errorlogs', '-C', workdir])
    #else:

    if workdir.find("hfuzz") == -1:
        # no testing for hfuzz
        run(['meson', 'test', '-q', '--print-errorlogs', '-C', workdir])

    # coverage
    if workdir.find("coverage") != -1:
        print(workdir)
        run(['meson', 'compile', '--ninja-args', 'coverage', '-C', workdir])
    

for cmd_dir in cmd_and_dir:
    workdir = cmd_dir[-1]

