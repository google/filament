# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys

import config_util  # pylint: disable=import-error


# This class doesn't need an __init__ method, so we disable the warning
# pylint: disable=no-init
class Flutter(config_util.Config):
    """Basic Config class for the Flutter repository."""
    @staticmethod
    def fetch_spec(_props):
        solution = {
            'custom_deps': {},
            'deps_file': 'DEPS',
            'managed': False,
            'name': 'src/flutter',
            'safesync_url': '',
            'url': 'https://github.com/flutter/engine.git',
        }
        spec = {'solutions': [solution]}
        return {
            'type': 'gclient_git',
            'gclient_git_spec': spec,
        }

    @staticmethod
    def expected_root(_props):
        return 'src'


def main(argv=None):
    return Flutter().handle_args(argv)


if __name__ == '__main__':
    sys.exit(main(sys.argv))
