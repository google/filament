# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys

import config_util  # pylint: disable=import-error


# This class doesn't need an __init__ method, so we disable the warning
# pylint: disable=no-init
class DepotTools(config_util.Config):
    """Basic Config class for DepotTools."""
    @staticmethod
    def fetch_spec(props):
        url = 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'
        solution = {
            'name': 'depot_tools',
            'url': url,
            'deps_file': 'DEPS',
            'managed': False,
        }
        spec = {
            'solutions': [solution],
        }
        checkout_type = 'gclient_git'
        spec_type = '%s_spec' % checkout_type
        return {
            'type': checkout_type,
            spec_type: spec,
        }

    @staticmethod
    def expected_root(_props):
        return 'depot_tools'


def main(argv=None):
    return DepotTools().handle_args(argv)


if __name__ == '__main__':
    sys.exit(main(sys.argv))
