# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys

import config_util  # pylint: disable=import-error


# This class doesn't need an __init__ method, so we disable the warning
# pylint: disable=no-init
class Infra(config_util.Config):
    """Basic Config class for the Infrastructure repositories."""
    @staticmethod
    def fetch_spec(props):
        return {
            'alias': {
                'config': 'infra_superproject',
                'props': [],
            },
        }

    @staticmethod
    def expected_root(_props):
        return ''


def main(argv=None):
    return Infra().handle_args(argv)


if __name__ == '__main__':
    sys.exit(main(sys.argv))
