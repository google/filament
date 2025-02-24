# Copyright 2022 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import ast
import sys

import config_util  # pylint: disable=import-error


# This class doesn't need an __init__ method, so we disable the warning
# pylint: disable=no-init
class ANGLE(config_util.Config):
    """Basic Config class for ANGLE."""
    @staticmethod
    def fetch_spec(props):
        url = 'https://chromium.googlesource.com/angle/angle.git'
        solution = {
            'name': '.',
            'url': url,
            'deps_file': 'DEPS',
            'managed': False,
            'custom_vars': {},
        }
        spec = {'solutions': [solution]}
        if props.get('target_os'):
            spec['target_os'] = props['target_os'].split(',')
        if ast.literal_eval(props.get('internal', 'False')):
            solution['custom_vars']['checkout_angle_internal'] = True
        return {
            'type': 'gclient_git',
            'gclient_git_spec': spec,
        }

    @staticmethod
    def expected_root(_props):
        return ''


def main(argv=None):
    return ANGLE().handle_args(argv)


if __name__ == '__main__':
    sys.exit(main(sys.argv))
