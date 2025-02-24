#!/usr/bin/env python3
# Copyright 2022 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""This script is a wrapper around the ninja binary that is pulled to
third_party as part of gclient sync. It will automatically find the ninja
binary when run inside a gclient source tree, so users can just type
"ninja" on the command line."""

import os
import subprocess
import sys

import gclient_paths
import gn_helper


def findNinjaInPath():
    env_path = os.getenv("PATH")
    if not env_path:
        return
    exe = "ninja"
    if sys.platform in ("win32", "cygwin"):
        exe += ".exe"
    for bin_dir in env_path.split(os.pathsep):
        if bin_dir.rstrip(os.sep).endswith("depot_tools"):
            # skip depot_tools to avoid calling ninja.py infinitely.
            continue
        ninja_path = os.path.join(bin_dir, exe)
        if os.path.isfile(ninja_path):
            return ninja_path

def checkOutdir(ninja_args):
    out_dir = "."
    tool = ""
    for i, arg in enumerate(ninja_args):
        if arg == "-t":
            tool = ninja_args[i + 1]
        elif arg.startswith("-t"):
            tool = arg[2:]
        elif arg == "-C":
            out_dir = ninja_args[i + 1]
        elif arg.startswith("-C"):
            out_dir = arg[2:]
    if tool in ["list", "commands", "inputs", "targets"]:
        # These tools are just inspect ninja rules and not modify out dir.
        # TODO: b/339320220 - implement these in siso
        return
    siso_marker = os.path.join(out_dir, ".siso_deps")
    if os.path.exists(siso_marker):
        print("depot_tools/ninja.py: %s contains Siso state file.\n"
              "Use `autoninja` to choose appropriate build tool,\n"
              "or run `gn clean %s` to switch from siso to ninja\n" %
              (out_dir, out_dir),
              file=sys.stderr)
        sys.exit(1)
    if os.environ.get("AUTONINJA_BUILD_ID"):
        # autoninja sets AUTONINJA_BUILD_ID
        return
    for k, v in gn_helper.args(out_dir):
        if k == "use_remoteexec" and v == "true":
            print(
                "depot_tools/ninja.py: detect use_remoteexec=true\n"
                "Use `autoninja` to choose appropriate build tool\n",
                file=sys.stderr)
            sys.exit(1)

def fallback(ninja_args):
    # Try to find ninja in PATH.
    ninja_path = findNinjaInPath()
    if ninja_path:
        checkOutdir(ninja_args)
        return subprocess.call([ninja_path] + ninja_args)

    print(
        "depot_tools/ninja.py: Could not find Ninja in the third_party of "
        "the current project, nor in your PATH.\n"
        "Please take one of the following actions to install Ninja:\n"
        "- If your project has DEPS, add a CIPD Ninja dependency to DEPS.\n"
        "- Otherwise, add Ninja to your PATH *after* depot_tools.",
        file=sys.stderr,
    )
    return 1


def main(args):
    # On Windows the ninja.bat script passes along the arguments enclosed in
    # double quotes. This prevents multiple levels of parsing of the special '^'
    # characters needed when compiling a single file.  When this case is
    # detected, we need to split the argument. This means that arguments
    # containing actual spaces are not supported by ninja.bat, but that is not a
    # real limitation.
    if sys.platform.startswith("win") and len(args) == 2:
        args = args[:1] + args[1].split()

    # macOS's python sets CPATH, LIBRARY_PATH, SDKROOT implicitly.
    # https://openradar.appspot.com/radar?id=5608755232243712
    #
    # Removing those environment variables to avoid affecting clang's behaviors.
    if sys.platform == "darwin":
        os.environ.pop("CPATH", None)
        os.environ.pop("LIBRARY_PATH", None)
        os.environ.pop("SDKROOT", None)

    # Get gclient root + src.
    primary_solution_path = gclient_paths.GetPrimarySolutionPath()
    gclient_root_path = gclient_paths.FindGclientRoot(os.getcwd())
    gclient_src_root_path = None
    if gclient_root_path:
        gclient_src_root_path = os.path.join(gclient_root_path, "src")

    for base_path in set(
        [primary_solution_path, gclient_root_path, gclient_src_root_path]):
        if not base_path:
            continue
        ninja_path = os.path.join(
            base_path,
            "third_party",
            "ninja",
            "ninja" + gclient_paths.GetExeSuffix(),
        )
        if os.path.isfile(ninja_path):
            checkOutdir(args[1:])
            return subprocess.call([ninja_path] + args[1:])

    return fallback(args[1:])


if __name__ == "__main__":
    try:
        sys.exit(main(sys.argv))
    except KeyboardInterrupt:
        sys.exit(1)
