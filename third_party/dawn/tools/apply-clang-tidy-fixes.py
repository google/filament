#!/usr/bin/env python3
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

import argparse
import json
import sys
import subprocess
from pathlib import Path


def fail(message: str) -> None:
    print('ERROR: ' + message, file=sys.stderr)
    sys.exit(1)


DAWN_ROOT = Path(__file__).parent.parent.resolve()

if Path.cwd().resolve() != DAWN_ROOT:
    fail("Must be run from the Dawn root directory.")

# Locate clang-tidy binary
clang_tidy = Path("third_party/llvm-build/Release+Asserts/bin/clang-tidy")
if sys.platform == "win32":
    clang_tidy = clang_tidy.with_suffix(".exe")

if not clang_tidy.is_file():
    fail(f'{clang_tidy} missing. See docs/clang-tidy.md.')


def is_generated_file(file_path: Path) -> bool:
    return file_path.is_relative_to(DAWN_ROOT / "out")


def main():
    parser = argparse.ArgumentParser(
        description=f"""
  Extract files matching a given clang-tidy check from findings.json files and apply autofixes.

  Basic usage: {sys.argv[0]} -c some-clang-tidy-check

  Note the findings files are only used to determine which files to inspect - this script applies ALL fixes for that check in the file, regardless of the exact fixits discovered.

  For more info on clang-tidy, see docs/clang-tidy.md.""",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "-c",
        "--check",
        required=True,
        help=
        "The specific clang-tidy check to apply fixes for (e.g., bugprone-parent-virtual-call)."
    )
    parser.add_argument(
        "-f",
        "--findings",
        help=
        "Optional path to the findings JSON file. If not specified, the script automatically searches all out/*/clang-tidy-*-findings.json files. (Note it's fine to search old findings, because this script re-runs clang-tidy on each file anyway. Just a bit slower.)"
    )
    parser.add_argument(
        "files",
        nargs="*",
        help=
        "Optionally filter fixes to only to the specified files, for example pass `src/dawn/native/**/*` on the command line (which will be expanded by your shell).",
    )

    args = parser.parse_args()

    # Determine findings files and group them by outdir
    outdir_to_findings = {}
    if args.findings:
        findings_path = Path(args.findings)
        if not findings_path.is_file():
            fail(f"Findings file not found: {findings_path}")
        outdir_path = findings_path.parent
        outdir_to_findings[outdir_path] = [findings_path]
    else:
        # Search all out/*/clang-tidy-*-findings.json
        findings_files = sorted(
            DAWN_ROOT.glob("out/*/clang-tidy-*-findings.json"))
        if not findings_files:
            fail("No clang-tidy-*-findings.json found in out/*/.\n"
                 "Please run tools/run-tricium-clang-tidy.py first.")

        print("Auto-discovered the following findings files:")
        for path in findings_files:
            print(f"  {path}")

        try:
            response = input("\nProceed with these findings files? [Y/n]: "
                             ).strip().lower()
        except KeyboardInterrupt:
            print()
            sys.exit(1)
        if response not in ("y", "yes", ""):
            print("Aborted.")
            sys.exit(0)

        for path in findings_files:
            outdir_path = path.parent
            if outdir_path not in outdir_to_findings:
                outdir_to_findings[outdir_path] = []
            outdir_to_findings[outdir_path].append(path)

    if args.files:
        enabled_files = {(DAWN_ROOT / f).resolve() for f in args.files}
        is_target_file = lambda file_path: file_path.resolve() in enabled_files
    else:
        is_target_file = lambda file_path: not is_generated_file(file_path)

    total_files_to_fix = 0

    for outdir_path, findings_paths in sorted(outdir_to_findings.items()):
        # Validate build directory has compile_commands.json
        if (not outdir_path.is_dir()
                or not (outdir_path / "compile_commands.json").is_file()):
            fail(f"Build directory '{outdir_path}' does not exist or missing "
                 "compile_commands.json.")

        # Extract matching files across all findings files for this outdir
        files_to_fix = set()
        for findings_path in findings_paths:
            try:
                with open(findings_path, "r", encoding="utf-8") as f:
                    data = json.load(f)
            except Exception as e:
                fail(f"Error parsing findings JSON file {findings_path}: {e}")

            diagnostics = data.get("diagnostics", [])
            for diag in diagnostics:
                if diag.get("diag_name") == args.check:
                    file_path_str = diag.get("file_path")
                    if file_path_str:
                        file_path = DAWN_ROOT / file_path_str
                        if is_target_file(file_path):
                            files_to_fix.add(file_path.relative_to(DAWN_ROOT))

        if not files_to_fix:
            continue

        total_files_to_fix += len(files_to_fix)

        print(f"\nFound {len(files_to_fix)} file(s) with diagnostics for "
              f"'{args.check}' in build directory '{outdir_path}':")
        for file in sorted(files_to_fix):
            print(f"  {file}")

        # Assemble and run the command for this outdir
        cmd = [
            str(clang_tidy),
            f"-p={outdir_path}",
            f"-checks=-*,{args.check}",
            "--fix",
            "--extra-arg=-w",
        ] + [str(f) for f in sorted(files_to_fix)]

        print(f"\nRunning command:\n  {' '.join(cmd)}")
        try:
            subprocess.run(cmd, check=True)
            print(f"\nFixes successfully applied for '{outdir_path}'!")
        except subprocess.CalledProcessError as e:
            fail(f"\nclang-tidy failed for '{outdir_path}'")

    if total_files_to_fix == 0:
        if args.files:
            print(f"No diagnostics found matching check "
                  f"(in specified fileset): '{args.check}'")
        else:
            print(f"No diagnostics found matching check "
                  f"(in non-generated files): '{args.check}'")


if __name__ == "__main__":
    main()
