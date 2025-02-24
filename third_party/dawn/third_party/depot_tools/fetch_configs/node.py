# Copyright 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys

import config_util  # pylint: disable=import-error


# This class doesn't need an __init__ method, so we disable the warning
# pylint: disable=no-init
class Node(config_util.Config):
    """An alias of node-ci config that build upstream node."""

    @staticmethod
    def fetch_spec(props):
        url = 'https://chromium.googlesource.com/v8/node-ci.git'
        return {
            'type': 'gclient_git',
            'gclient_git_spec': {
                'solutions': [{
                    'name': 'node',
                    'url': url,
                    'deps_file': 'DEPS',
                    'managed': False,
                    'custom_deps': {},
                    'custom_vars': {
                        'build_upstream_node': True,
                        'node_url': 'https://github.com/nodejs/node.git',
                        'node_revision': 'main',
                    },
                }],
            },
        }

    @staticmethod
    def expected_root(_props):
        return 'node'


def main(argv=None):
    return Node().handle_args(argv)


if __name__ == '__main__':
    sys.exit(main(sys.argv))
