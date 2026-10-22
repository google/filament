#!/usr/bin/env vpython3
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
"""Invokes the hermetic Bazelisk binary to build LiteRT-LM and copy the output
binary to GN's output directory."""

import os
from pathlib import Path
import shutil
import subprocess
import sys

# Add the project root to sys.path to allow package imports from tools.python.
_SCRIPT_DIR = Path(__file__).resolve().parent
_PROJECT_ROOT = _SCRIPT_DIR.parent.parent
if str(_PROJECT_ROOT) not in sys.path:
    sys.path.append(str(_PROJECT_ROOT))

from tools.python import cipd_deps

BAZEL_TARGET = '//runtime/engine:litert_lm_advanced_main'
BAZEL_BIN_SUBPATH = 'runtime/engine/litert_lm_advanced_main'


def get_platform_name():
    cipd_os = cipd_deps.get_cipd_compatible_current_os()
    cipd_arch = cipd_deps.get_cipd_compatible_current_arch()

    plat_os = 'macos' if cipd_os == 'mac' else cipd_os
    arch = 'x86_64' if cipd_arch == 'amd64' else cipd_arch

    return f"{plat_os}_{arch}"


def main():
    if len(sys.argv) < 2:
        print("Error: Missing output path argument.", file=sys.stderr)
        print("Usage: build_litert_lm.py <output_path>", file=sys.stderr)
        sys.exit(1)

    dest_path = Path(sys.argv[1]).resolve()

    script_dir = Path(__file__).resolve().parent
    project_root = script_dir.parent.parent
    litert_lm_dir = script_dir / 'src'
    platform_name = get_platform_name()
    prebuilt_cipd_dir = script_dir / 'data' / 'prebuilt' / platform_name

    # LiteRT-LM is a submodule and also relies on a CIPD-unpacked prebuilts
    # package. Verify both are checked out. Since git creates an empty directory
    # or git link for submodules, check for a sentinel file (WORKSPACE) to
    # verify the source is actually present.
    is_checked_out = (litert_lm_dir /
                      'WORKSPACE').exists() and prebuilt_cipd_dir.exists()

    if not is_checked_out:
        print("Error: LiteRT-LM dependencies not found.", file=sys.stderr)
        print(
            "Please add the following to your .gclient file under 'custom_vars':",
            file=sys.stderr)
        print('    "checkout_litert_lm": True,', file=sys.stderr)
        print("And then run `gclient sync` to download the dependencies.",
              file=sys.stderr)
        sys.exit(1)

    bazelisk_path = project_root / 'tools' / 'bazelisk' / 'bazelisk'
    prebuilt_src_dir = litert_lm_dir / 'prebuilt'
    backup_dir = litert_lm_dir / 'prebuilt_backup'

    # Back up the original Git LFS smudge prebuilt directory.
    if prebuilt_src_dir.is_symlink():
        prebuilt_src_dir.unlink()
    elif prebuilt_src_dir.exists():
        shutil.move(prebuilt_src_dir, backup_dir)

    try:
        # Symlink the CIPD-unpacked prebuilts and libwebgpu directly so Bazel
        # can resolve targets.
        platform_name = get_platform_name()

        # Determine Dawn monolithic library name for current platform.
        if platform_name.startswith('macos'):
            dawn_lib_name = 'libwebgpu_dawn.dylib'
        elif platform_name.startswith('windows'):
            dawn_lib_name = 'libwebgpu_dawn.dll'
        else:
            dawn_lib_name = 'libwebgpu_dawn.so'

        # Find the locally compiled Dawn library in the active GN build
        # directory.
        local_dawn_lib = dest_path.parent / dawn_lib_name
        if not local_dawn_lib.exists():
            print(
                f"Error: Local Dawn library (libwebgpu_dawn) not found in GN output directory: {local_dawn_lib}",
                file=sys.stderr)
            sys.exit(1)

        print("Symlinking prebuilt binaries to LiteRT-LM workspace...")
        prebuilt_src_dir.mkdir()
        platform_dir = prebuilt_cipd_dir
        if not platform_dir.exists():
            print(
                f"Error: Prebuilt shared library dependencies not found in: {platform_dir}",
                file=sys.stderr)
            sys.exit(1)

        platform_dest = prebuilt_src_dir / platform_name
        platform_dest.mkdir()

        # Link all prebuilts from CIPD.
        for f in platform_dir.iterdir():
            if f.is_file():
                (platform_dest / f.name).symlink_to(f)

        # Link the local Dawn library into the platform directory.
        dest_dawn_path = platform_dest / dawn_lib_name
        dest_dawn_path.symlink_to(local_dawn_lib)

        # Isolate Bazel's output_user_root to the GN output directory to prevent stale
        # caches across checkouts, or filesystem inconsistencies.
        bazel_user_root = dest_path.parent / '.bazel_root'

        # Compile the target using Bazelisk inside LiteRT-LM's standalone
        # workspace.
        print(f"Building Bazel target: {BAZEL_TARGET}...")
        build_cmd = [
            str(bazelisk_path),
            f"--output_user_root={bazel_user_root}",
            'build',
            '--compilation_mode=opt',
            '--define=litert_link_capi_so=true',
            '--define=resolve_symbols_in_exec=false',
            BAZEL_TARGET,
        ]

        env = dict(os.environ)

        # Prepend the hermetic LLVM toolchain bin directory to PATH and set CC/CXX.
        llvm_bin_dir = project_root / 'third_party' / 'llvm-build' / 'Release+Asserts' / 'bin'
        if llvm_bin_dir.exists():
            env['PATH'] = f"{llvm_bin_dir}:{env.get('PATH', '')}"

            # Explicitly set CC and CXX to the hermetic compilers so Bazel's
            # cc_configure selects them.
            env['CC'] = str(llvm_bin_dir / 'clang')
            env['CXX'] = str(llvm_bin_dir / 'clang++')

        # Unconditionally clean the Bazel workspace before building to prevent
        # filesystem inconsistency errors on the bots.
        # TODO(crbug.com/550309673): Try removing this clean step in a follow up if
        # it's not actually necessary anymore.
        clean_cmd = [str(bazelisk_path), 'clean']
        subprocess.run(clean_cmd, cwd=litert_lm_dir, env=env)

        proc = subprocess.run(build_cmd, cwd=litert_lm_dir, env=env)
        if proc.returncode != 0:
            print("Error: Bazel build failed.", file=sys.stderr)
            sys.exit(proc.returncode)

    finally:
        # Restore the original Git-tracked prebuilt directory regardless of
        # success or failure. This keeps the Git tree clean for gclient sync.
        print("Restoring original Git-tracked prebuilt directory...")
        if prebuilt_src_dir.is_symlink():
            prebuilt_src_dir.unlink()
        elif prebuilt_src_dir.exists():
            shutil.rmtree(prebuilt_src_dir)
        if backup_dir.exists():
            shutil.move(backup_dir, prebuilt_src_dir)

    # Locate and copy the compiled binary into the GN target directory.
    compiled_path = litert_lm_dir / 'bazel-bin' / BAZEL_BIN_SUBPATH
    if not compiled_path.exists():
        print(
            f"Error: Compiled binary not found at expected path: {compiled_path}",
            file=sys.stderr)
        sys.exit(1)

    print(f"Copying compiled binary to: {dest_path}")
    shutil.copy2(compiled_path, dest_path)
    dest_path.chmod(0o755)

    # Copy other prebuilt LiteRT dependencies to the build output directory for
    # running.
    target_prebuilt_dir = prebuilt_cipd_dir
    if target_prebuilt_dir.exists():
        for f in target_prebuilt_dir.iterdir():
            if f.is_file():
                dest_file = dest_path.parent / f.name
                shutil.copy2(f, dest_file)
                dest_file.chmod(0o755)

    print("Build and copy successful.")


if __name__ == '__main__':
    main()
