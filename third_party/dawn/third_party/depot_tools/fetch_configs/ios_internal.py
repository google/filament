# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys

import config_util  # pylint: disable=import-error


# This class doesn't need an __init__ method, so we disable the warning
# pylint: disable=no-init
class IOSInternal(config_util.Config):
    """Basic Config class for Chrome on iOS."""
    @staticmethod
    def fetch_spec(props):
        return {
            'alias': {
                'config':
                'chromium',
                'props': [
                    '--internal=True',
                    '--target_os=ios',
                    '--target_os_only=True',
                ],
            },
        }

    @staticmethod
    def expected_root(_props):
        return ''


def main(argv=None):
    return IOSInternal().handle_args(argv)


if __name__ == '__main__':
    sys.exit(main(sys.argv))
