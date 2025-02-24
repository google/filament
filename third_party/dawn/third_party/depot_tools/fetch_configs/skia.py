# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys

import config_util  # pylint: disable=import-error


# This class doesn't need an __init__ method, so we disable the warning
# pylint: disable=no-init
class Skia(config_util.Config):
    """Basic Config class for the Skia repository."""
    @staticmethod
    def fetch_spec(_props):
        solution = {
            'name': 'skia',
            'url': 'https://skia.googlesource.com/skia.git',
            'deps_file': 'DEPS',
            'managed': False,
        }
        spec = {'solutions': [solution]}
        return {
            'type': 'gclient_git',
            'gclient_git_spec': spec,
        }

    @staticmethod
    def expected_root(_props):
        return 'skia'


def main(argv=None):
    return Skia().handle_args(argv)


if __name__ == '__main__':
    sys.exit(main(sys.argv))
