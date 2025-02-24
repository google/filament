#!/usr/bin/env python3
# Copyright (c) 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from string import Template

import argparse
import io
import json
import os
import re
import shutil
import sys

ARCH_VAR = 'arch'
OS_VAR = 'os'
PLATFORM_VAR = 'platform'

CIPD_SUBDIR_RE = '@Subdir (.*)'
CIPD_DESCRIBE = 'describe'
CIPD_ENSURE = 'ensure'
CIPD_ENSURE_FILE_RESOLVE = 'ensure-file-resolve'
CIPD_EXPAND_PKG = 'expand-package-name'
CIPD_EXPORT = 'export'

DESCRIBE_STDOUT_TEMPLATE = """\
Package:       ${package}
Instance ID:   ${package}-fake-instance-id
Registered by: user:fake-testing-support
Registered at: 2023-05-10 18:53:55.078574 +0000 UTC
Refs:
  ${package}-latest
Tags:
  git_revision:${package}-fake-git-revision
  ${package}-fake-tag:1.0
  ${package}-fake-tag:2.0
"""

DESCRIBE_JSON_TEMPLATE = """{
  "result": {
    "pin": {
      "package": "${package}",
      "instance_id": "${package}-fake-instance-id"
    },
    "registered_by": "user:fake-testing-support",
    "registered_ts": 1683744835,
    "refs": [
      {
        "ref": "${package}-latest",
        "instance_id": "${package}-fake-instance-id",
        "modified_by": "user:fake-testing-support",
        "modified_ts": 1683744835
      }
    ],
    "tags": [
      {
        "tag": "git_revision:${package}-fake-git-revision",
        "registered_by": "user:fake-testing-support",
        "registered_ts": 1683744835
      },
      {
        "tag": "${package}-fake-tag:1.0",
        "registered_by": "user:fake-testing-support",
        "registered_ts": 1683744835
      },
      {
        "tag": "${package}-fake-tag:2.0",
        "registered_by": "user:fake-testing-support",
        "registered_ts": 1683744835
      }
    ]
  }
}"""


def parse_cipd(root, contents):
    tree = {}
    current_subdir = None
    for line in contents:
        line = line.strip()
        match = re.match(CIPD_SUBDIR_RE, line)
        if match:
            print('match')
            current_subdir = os.path.join(root, *match.group(1).split('/'))
            if not root:
                current_subdir = match.group(1)
        elif line and current_subdir:
            print('no match')
            tree.setdefault(current_subdir, []).append(line)
    return tree


def expand_package_name_cmd(package_name):
    package_split = package_name.split("/")
    suffix = package_split[-1]
    # Any use of var equality should return empty for testing.
    if "=" in suffix:
        if suffix != "${platform=fake-platform-ok}":
            return ""
        package_name = "/".join(package_split[:-1] + ["${platform}"])
    for v in [ARCH_VAR, OS_VAR, PLATFORM_VAR]:
        var = "${%s}" % v
        if package_name.endswith(var):
            package_name = package_name.replace(var,
                                                "%s-expanded-test-only" % v)
    return package_name


def ensure_file_resolve():
    resolved = {"result": {}}
    parser = argparse.ArgumentParser()
    parser.add_argument('-ensure-file', required=True)
    parser.add_argument('-json-output')
    args, _ = parser.parse_known_args()
    with io.open(args.ensure_file, 'r', encoding='utf-8') as f:
        new_content = parse_cipd("", f.readlines())
        for path, packages in new_content.items():
            resolved_packages = []
            for package in packages:
                package_name = expand_package_name_cmd(package.split(" ")[0])
                resolved_packages.append({
                    "package": package_name,
                    "pin": {
                        "package": package_name,
                        "instance_id": package_name + "-fake-resolved-id",
                    }
                })
            resolved["result"][path] = resolved_packages
        with io.open(args.json_output, 'w', encoding='utf-8') as f:
            f.write(json.dumps(resolved, indent=4))


def describe_cmd(package_name):
    parser = argparse.ArgumentParser()
    parser.add_argument('-json-output')
    parser.add_argument('-version', required=True)
    args, _ = parser.parse_known_args()
    json_template = Template(DESCRIBE_JSON_TEMPLATE).substitute(
        package=package_name)
    cli_out = Template(DESCRIBE_STDOUT_TEMPLATE).substitute(
        package=package_name)
    json_out = json.loads(json_template)
    found = False
    for tag in json_out['result']['tags']:
        if tag['tag'] == args.version:
            found = True
            break
    for tag in json_out['result']['refs']:
        if tag['ref'] == args.version:
            found = True
            break
    if found:
        if args.json_output:
            with io.open(args.json_output, 'w', encoding='utf-8') as f:
                f.write(json.dumps(json_out, indent=4))
        sys.stdout.write(cli_out)
        return 0
    sys.stdout.write('Error: no such ref.\n')
    return 1


def main():
    cmd = sys.argv[1]
    assert cmd in [
        CIPD_DESCRIBE, CIPD_ENSURE, CIPD_ENSURE_FILE_RESOLVE, CIPD_EXPAND_PKG,
        CIPD_EXPORT
    ]
    # Handle cipd expand-package-name
    if cmd == CIPD_EXPAND_PKG:
        # Expecting argument after cmd
        assert len(sys.argv) == 3
        # Write result to stdout
        sys.stdout.write(expand_package_name_cmd(sys.argv[2]))
        return 0
    if cmd == CIPD_DESCRIBE:
        # Expecting argument after cmd
        assert len(sys.argv) >= 3
        return describe_cmd(sys.argv[2])
    if cmd == CIPD_ENSURE_FILE_RESOLVE:
        return ensure_file_resolve()

    parser = argparse.ArgumentParser()
    parser.add_argument('-ensure-file')
    parser.add_argument('-root')
    args, _ = parser.parse_known_args()

    with io.open(args.ensure_file, 'r', encoding='utf-8') as f:
        new_content = parse_cipd(args.root, f.readlines())

    # Install new packages
    for path, packages in new_content.items():
        if not os.path.exists(path):
            os.makedirs(path)
        with io.open(os.path.join(path, '_cipd'), 'w', encoding='utf-8') as f:
            f.write('\n'.join(packages))

    # Save the ensure file that we got
    shutil.copy(args.ensure_file, os.path.join(args.root, '_cipd'))

    return 0


if __name__ == '__main__':
    sys.exit(main())
