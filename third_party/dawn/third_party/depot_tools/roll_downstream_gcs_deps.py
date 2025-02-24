#!/usr/bin/env python3
# Copyright 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""This scripts copies DEPS package information from one source onto
destination.

If the destination doesn't have packages, the script errors out.

Example usage:
  roll_downstream_gcs_deps.py  \
      --source some/repo/DEPS \
      --destination some/downstream/repo/DEPS \
      --package src/build/linux/debian_bullseye_amd64-sysroot \
      --package src/build/linux/debian_bullseye_arm64-sysroot

"""

import argparse
import ast
import sys
from typing import Dict, List


def _get_deps(deps_ast: ast.Module) -> Dict[str, ast.Dict]:
    """Searches for the deps dict in a DEPS file AST.

    Args:
      deps_ast: AST of the DEPS file.

    Raises:
      Exception: If the deps dict is not found.

    Returns:
      The deps dict.
    """
    for statement in deps_ast.body:
        if not isinstance(statement, ast.Assign):
            continue
        if len(statement.targets) != 1:
            continue
        target = statement.targets[0]
        if not isinstance(target, ast.Name):
            continue
        if target.id != 'deps':
            continue
        if not isinstance(statement.value, ast.Dict):
            continue
        deps = {}
        for key, value in zip(statement.value.keys, statement.value.values):
            if not isinstance(key, ast.Constant):
                continue
            deps[key.value] = value
        return deps
    raise Exception('no deps found')


def _get_gcs_object_list_ast(package_ast: ast.Dict) -> ast.List:
    """Searches for the objects list in a GCS package AST.

    Args:
      package_ast: AST of the GCS package.

    Raises:
      Exception: If the package is not a GCS package.

    Returns:
      AST of the objects list.
    """
    is_gcs = False
    result = None
    for key, value in zip(package_ast.keys, package_ast.values):
        if not isinstance(key, ast.Constant):
            continue
        if key.value == 'dep_type' and isinstance(
                value, ast.Constant) and value.value == 'gcs':
            is_gcs = True
        if key.value == 'objects' and isinstance(value, ast.List):
            result = value

    assert is_gcs, 'Not a GCS dependency!'
    assert result, 'No objects found!'
    return result


def _replace_ast(destination: str, dest_ast: ast.Module, source: str,
                 source_ast: ast.Module) -> str:
    """Replaces the content of dest_ast with the content of the
    same package in source_ast.

    Args:
      destination: Destination DEPS file content.
      dest_ast: AST in the destination DEPS file that will be replaced.
      source: Source DEPS file content.
      source_ast: AST in the source DEPS file that will replace content of
      destination.

    Returns:
      Content of destination DEPS file with replaced content.
    """
    source_lines = source.splitlines()
    lines = destination.splitlines()
    # Copy all lines before the replaced AST.
    result = '\n'.join(lines[:dest_ast.lineno - 1]) + '\n'

    # Partially copy the line content before AST's value.
    result += lines[dest_ast.lineno - 1][:dest_ast.col_offset]

    # Copy data from source AST.
    if source_ast.lineno == source_ast.end_lineno:
        # Starts and ends on the same line.
        result += source_lines[
            source_ast.lineno -
            1][source_ast.col_offset:source_ast.end_col_offset]
    else:
        # Copy multiline content from source. The first line and the last line
        # of source AST should be partially copied as `result` has a partial
        # line from `destination`.

        # Partially copy the first line of source AST.
        result += source_lines[source_ast.lineno -
                               1][source_ast.col_offset:] + '\n'
        # Copy content in the middle.
        result += '\n'.join(
            source_lines[source_ast.lineno:source_ast.end_lineno - 1]) + '\n'
        # Partially copy the last line of source AST.
        result += source_lines[source_ast.end_lineno -
                               1][:source_ast.end_col_offset]

    # Copy the rest of the line after the package value.
    result += lines[dest_ast.end_lineno - 1][dest_ast.end_col_offset:] + '\n'

    # Copy the rest of the lines after the package value.
    result += '\n'.join(lines[dest_ast.end_lineno:])
    # Add trailing newline
    if destination.endswith('\n'):
        result += '\n'
    return result


def copy_packages(source_content: str, destination_content: str,
                  source_packages: List[str],
                  destination_packages: List[str]) -> str:
    """Copies GCS packages from source to destination.

    Args:
      source: Source DEPS file content.
      destination: Destination DEPS file content.
      packages: List of GCS packages to copy. Only objects are copied.

    Returns:
      Destination DEPS file content with packages copied.
    """
    source_deps = _get_deps(ast.parse(source_content, mode='exec'))
    for i in range(len(source_packages)):
        source_package = source_packages[i]
        destination_package = destination_packages[i]
        if source_package not in source_deps:
            raise Exception('Package %s not found in source' % source_package)
        dest_deps = _get_deps(ast.parse(destination_content, mode='exec'))
        if destination_package not in dest_deps:
            raise Exception('Package %s not found in destination' %
                            destination_package)
        destination_content = _replace_ast(
            destination_content,
            _get_gcs_object_list_ast(dest_deps[destination_package]),
            source_content,
            _get_gcs_object_list_ast(source_deps[source_package]))

    return destination_content


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-deps',
                        required=True,
                        help='Source DEPS file where content will be copied '
                        'from')
    parser.add_argument('--source-package',
                        action='append',
                        required=True,
                        help='List of DEPS packages to update')
    parser.add_argument('--destination-deps',
                        required=True,
                        help='Destination DEPS file, where content will be '
                        'saved')
    parser.add_argument('--destination-package',
                        action='append',
                        required=True,
                        help='List of DEPS packages to update')
    args = parser.parse_args()

    if not args.source_package:
        parser.error('No source packages specified to roll, aborting...')

    if not args.destination_package:
        parser.error('No destination packages specified to roll, aborting...')

    if len(args.destination_package) != len(args.source_package):
        parser.error('Source and destination packages must be of the same '
                     'length, aborting...')

    with open(args.source_deps) as f:
        source_content = f.read()

    with open(args.destination_deps) as f:
        destination_content = f.read()

    new_content = copy_packages(source_content, destination_content,
                                args.source_package, args.destination_package)

    with open(args.destination_deps, 'w') as f:
        f.write(new_content)

    print('Run:')
    print('  Destination DEPS file updated. You still need to create and '
          'upload a change.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
