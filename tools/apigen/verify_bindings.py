#!/usr/bin/env python3
#
# Copyright (C) 2026 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

"""Standalone CLI utility to statically validate Filament Java and JNI C++ bindings."""

import argparse
from pathlib import Path
import sys

REPO_ROOT = Path(__file__).parent.parent.parent.resolve()
DEFAULT_JAVA_DIR = REPO_ROOT / "android/filament-android/src/main/java"
DEFAULT_CPP_DIR = REPO_ROOT / "android/filament-android/src/main/cpp"

# Add tools/apigen to path for javagen imports
sys.path.insert(0, str(REPO_ROOT / "tools/apigen"))
from javagen.validator import BindingValidator


def main():
    parser = argparse.ArgumentParser(
        description="Statically validate Java native methods, JNI signatures, and reflection descriptors."
    )
    parser.add_argument(
        "--java-dir",
        type=Path,
        default=DEFAULT_JAVA_DIR,
        help="Path to Java source directory (default: %(default)s)"
    )
    parser.add_argument(
        "--jni-dir",
        type=Path,
        default=DEFAULT_CPP_DIR,
        help="Path to JNI C++ source directory (default: %(default)s)"
    )
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Enable verbose output"
    )
    parser.add_argument(
        "targets",
        nargs="*",
        help="Optional list of specific classes to validate (e.g. View LightManager)"
    )

    args = parser.parse_args()

    java_dir = args.java_dir.resolve()
    cpp_dir = args.jni_dir.resolve()
    target_set = set(args.targets) if args.targets else None

    print(f"Validating Java and JNI C++ bindings...")
    print(f"  Java dir: {java_dir}")
    print(f"  JNI dir:  {cpp_dir}")
    if target_set:
        print(f"  Targets:  {', '.join(sorted(target_set))}")

    validator = BindingValidator(
        java_dir=java_dir,
        cpp_dir=cpp_dir,
        verbose=args.verbose,
        target_classes=target_set
    )
    result = validator.run()

    report = result.format_report(verbose=args.verbose)
    print("\n" + report)

    if not result.is_clean:
        sys.exit(1)

    print("\n✓ Validation completed with zero errors.")
    sys.exit(0)


if __name__ == "__main__":
    main()
