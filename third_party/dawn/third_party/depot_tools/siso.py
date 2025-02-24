#!/usr/bin/env python3
# Copyright 2023 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""This script is a wrapper around the siso binary that is pulled to
third_party as part of gclient sync. It will automatically find the siso
binary when run inside a gclient source tree, so users can just type
"siso" on the command line."""

import os
import signal
import subprocess
import sys

import gclient_paths


def checkOutdir(args):
    subcmd = ''
    out_dir = "."
    for i, arg in enumerate(args):
        if not arg.startswith("-") and not subcmd:
            subcmd = arg
            continue
        if arg == "-C":
            out_dir = args[i + 1]
        elif arg.startswith("-C"):
            out_dir = arg[2:]
    if subcmd != "ninja":
        return
    ninja_marker = os.path.join(out_dir, ".ninja_deps")
    if os.path.exists(ninja_marker):
        print("depot_tools/siso.py: %s contains Ninja state file.\n"
              "Use `autoninja` to use reclient,\n"
              "or run `gn clean %s` to switch from ninja to siso\n" %
              (out_dir, out_dir),
              file=sys.stderr)
        sys.exit(1)


def main(args):
    # Do not raise KeyboardInterrupt on SIGINT so as to give siso time to run
    # cleanup tasks. Siso will be terminated immediately after the second
    # Ctrl-C.
    original_sigint_handler = signal.getsignal(signal.SIGINT)

    def _ignore(signum, frame):
        try:
            # Call the original signal handler.
            original_sigint_handler(signum, frame)
        except KeyboardInterrupt:
            # Do not reraise KeyboardInterrupt so as to not kill siso too early.
            pass

    signal.signal(signal.SIGINT, _ignore)

    if not sys.platform.startswith('win'):
        signal.signal(signal.SIGTERM, lambda signum, frame: None)

    # On Windows the siso.bat script passes along the arguments enclosed in
    # double quotes. This prevents multiple levels of parsing of the special '^'
    # characters needed when compiling a single file.  When this case is
    # detected, we need to split the argument. This means that arguments
    # containing actual spaces are not supported by siso.bat, but that is not a
    # real limitation.
    if sys.platform.startswith('win') and len(args) == 2:
        args = args[:1] + args[1].split()

    # macOS's python sets CPATH, LIBRARY_PATH, SDKROOT implicitly.
    # https://openradar.appspot.com/radar?id=5608755232243712
    #
    # Removing those environment variables to avoid affecting clang's behaviors.
    if sys.platform == 'darwin':
        os.environ.pop("CPATH", None)
        os.environ.pop("LIBRARY_PATH", None)
        os.environ.pop("SDKROOT", None)

    # if user doesn't set PYTHONPYCACHEPREFIX and PYTHONDONTWRITEBYTECODE
    # set PYTHONDONTWRITEBYTECODE=1 not to create many *.pyc in workspace
    # and keep workspace clean.
    if not os.environ.get("PYTHONPYCACHEPREFIX"):
        os.environ.setdefault("PYTHONDONTWRITEBYTECODE", "1")

    environ = os.environ.copy()

    # Get gclient root + src.
    primary_solution_path = gclient_paths.GetPrimarySolutionPath()
    gclient_root_path = gclient_paths.FindGclientRoot(os.getcwd())
    gclient_src_root_path = None
    if gclient_root_path:
        gclient_src_root_path = os.path.join(gclient_root_path, 'src')

    siso_override_path = os.environ.get('SISO_PATH')
    if siso_override_path:
        print('depot_tools/siso.py: Using Siso binary from SISO_PATH: %s.' %
              siso_override_path,
              file=sys.stderr)
        if not os.path.isfile(siso_override_path):
            print(
                'depot_tools/siso.py: Could not find Siso at provided '
                'SISO_PATH.',
                file=sys.stderr)
            return 1

    for base_path in set(
        [primary_solution_path, gclient_root_path, gclient_src_root_path]):
        if not base_path:
            continue
        env = environ.copy()
        sisoenv_path = os.path.join(base_path, 'build', 'config', 'siso',
                                    '.sisoenv')
        if not os.path.exists(sisoenv_path):
            continue
        with open(sisoenv_path) as f:
            for line in f.readlines():
                k, v = line.rstrip().split('=', 1)
                env[k] = v
        siso_paths = [
            siso_override_path,
            os.path.join(base_path, 'third_party', 'siso', 'cipd',
                         'siso' + gclient_paths.GetExeSuffix()),
            os.path.join(base_path, 'third_party', 'siso',
                         'siso' + gclient_paths.GetExeSuffix()),
        ]
        for siso_path in siso_paths:
            if siso_path and os.path.isfile(siso_path):
                checkOutdir(args[1:])
                return subprocess.call([siso_path] + args[1:], env=env)
        print(
            'depot_tools/siso.py: Could not find siso in third_party/siso '
            'of the current project. Did you run gclient sync?',
            file=sys.stderr)
        return 1
    if siso_override_path:
        return subprocess.call([siso_override_path] + args[1:])

    print(
        'depot_tools/siso.py: Could not find .sisoenv under build/config/siso '
        'of the current project. Did you run gclient sync?',
        file=sys.stderr)
    return 1


if __name__ == '__main__':
    sys.exit(main(sys.argv))
