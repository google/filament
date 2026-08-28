#!/usr/bin/env python3
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

"""Report JS bindings that exist but that no test exercises.

unbound_apis.py asks whether the bindings cover the C++ API. This asks the opposite question:
whether the tests cover the bindings. A binding nobody calls is a binding nobody notices breaking,
which is how filament.d.ts came to declare methods the bindings had never exposed.

The suite is three files, each covering what the one before it cannot:

  web/filament-js/test.ts            type-checked, never run. Proves filament.d.ts describes the
                                     bindings, including the calls that would abort at runtime.
  web/filament-js/test-runtime.js    run under Node on the NOOP backend. Proves the bindings exist
                                     and marshal, for everything that does not need a GPU.
  web/filament-js/test-browser.html  run in a browser on WebGL. Covers what is left: rendering,
                                     pixel readback, picking and image decoding.

A member is covered when its name appears in any of the three, so the default report is the union.
Pass --target to check one file on its own.

Usage
-----
    ./tools/web-binding-audit/untested_bindings.py           # report untested members, exit 1 if any
    ./tools/web-binding-audit/untested_bindings.py --all     # also list the deliberate exclusions
    ./tools/web-binding-audit/untested_bindings.py --target runtime   # ts | runtime | browser

Known limitations
-----------------
Matching is by name: a member counts as covered when its name appears anywhere in a test file, so
a mention in a comment counts, and one call covers every overload. Treat a clean run as "every
binding is named by a test" rather than "every binding is verified".
"""

import argparse
import os
import re
import sys
from collections import defaultdict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import unbound_apis
from unbound_apis import ROOT, MEMBER_RE, WEB_SOURCES, read

TESTS = {
    "ts": "web/filament-js/test.ts",
    "runtime": "web/filament-js/test-runtime.js",
    "browser": "web/filament-js/test-browser.html",
}

# Only class members are audited. Enum values and value_object fields are bound by the thousand and
# are exercised in bulk: a test that round-trips an options struct covers every field in it without
# naming one.
REGISTRATION_RE = re.compile(
    r'(class_|value_object|value_array|enum_)<[^;]*?>\s*\(\s*"([^"]+)"')

# Members intentionally left untested, keyed by "Class.member" or "Class.*" -> reason.
EXCLUSIONS = {
    # Internal entry points that a wrapper in extensions.js calls on the caller's behalf. The
    # underscore prefix is stripped when the surface is collected, so they show up under the name
    # of the wrapper that hides them.
    "Engine.createSwapChainForCanvas": "internal; Engine.createSwapChain calls it when the engine "
                                       "was created on a canvas",

    # filament::Stream is not bound at all (see Stream.* in unbound_apis.py), so there is no way to
    # obtain the argument this validity check takes.
    "Engine.isValidStream": "no Stream can be constructed from JS, so this cannot be called",
}


def excluded_reason(cls, member):
    return EXCLUSIONS.get(f"{cls}.{member}") or EXCLUSIONS.get(f"{cls}.*")


def collect_class_members():
    """Return {embind class name: {member names}}, classes only."""
    web = defaultdict(set)
    for source in WEB_SOURCES:
        kind, current = None, None
        for line in read(source).splitlines():
            opened = REGISTRATION_RE.search(line)
            if opened:
                kind, current = opened.group(1), opened.group(2)
                if kind == "class_":
                    web.setdefault(current, set())
            if kind != "class_" or not current:
                continue
            for member in MEMBER_RE.finditer(line):
                web[current].add(member.group(1).lstrip("_"))

    # extensions.js both wraps bound entry points and adds members of its own; either way what it
    # defines is part of the surface a test can reach.
    extensions = read("web/filament-js/extensions.js")
    for cls, method in re.findall(
            r"Filament\.([A-Za-z0-9_$]+)\.prototype\.([A-Za-z0-9_]+)\s*=", extensions):
        web[cls].add(method)
    for cls, method in re.findall(
            r"Filament\.([A-Za-z0-9_$]+)\.([A-Za-z0-9_]+)\s*=\s*function", extensions):
        web[cls].add(method)
    return web


def identifiers(path):
    return set(re.findall(r"[A-Za-z_$][A-Za-z0-9_$]*", read(path)))


def audit(targets):
    exercised = set()
    for target in targets:
        exercised |= identifiers(TESTS[target])

    surface = collect_class_members()
    gaps, excluded, total = [], [], 0
    for cls in sorted(surface):
        for member in sorted(surface[cls]):
            total += 1
            if member in exercised:
                continue
            reason = excluded_reason(cls, member)
            (excluded if reason else gaps).append((cls, member, reason))
    return gaps, excluded, total


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--target", choices=sorted(TESTS), action="append",
                        help="check one test file rather than the union of all three; repeatable")
    parser.add_argument("--all", action="store_true",
                        help="also list the deliberate exclusions and their reasons")
    args = parser.parse_args()

    targets = args.target or sorted(TESTS)
    gaps, excluded, total = audit(targets)
    covered = total - len(gaps) - len(excluded)

    where = ", ".join(os.path.basename(TESTS[t]) for t in targets)
    if gaps:
        print(f"Bound but not exercised by {where} ({len(gaps)} of {total}):\n")
        current = None
        for cls, member, _ in gaps:
            if cls != current:
                print(f"  {cls}")
                current = cls
            print(f"    {member}")
        print("\nCall these from a test, or add them to EXCLUSIONS in this script with the reason")
        print("they cannot be called.")
    else:
        print(f"Every binding is exercised by {where}.")

    print(f"\n{covered}/{total} bound class members covered, {len(excluded)} deliberately not.")

    if args.all:
        print(f"\nDeliberately untested ({len(excluded)}):\n")
        current = None
        for cls, member, reason in excluded:
            if cls != current:
                print(f"  {cls}")
                current = cls
            print(f"    {member:<34} {reason}")

    return 1 if gaps else 0


if __name__ == "__main__":
    sys.exit(main())
