# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys

import config_util  # pylint: disable=import-error


# This class doesn't need an __init__ method, so we disable the warning
# pylint: disable=no-init
class DevToolsInternal(config_util.Config):
    """Basic Config class for the whole set of DevTools."""
    @staticmethod
    def fetch_spec(props):
        url = ('https://chrome-internal.googlesource.com/devtools/'
               'devtools-internal.git')
        solution = {
            'name': 'devtools-internal',
            'url': url,
            'deps_file': 'DEPS',
            'managed': False,
            'custom_deps': {},
        }
        spec = {
            'solutions': [solution],
            'with_branch_heads': True,
        }
        return {
            'type': 'gclient_git',
            'gclient_git_spec': spec,
        }

    @staticmethod
    def expected_root(_props):
        return 'devtools-internal'


def main(argv=None):
    return DevToolsInternal().handle_args(argv)


if __name__ == '__main__':
    sys.exit(main(sys.argv))
