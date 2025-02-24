# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys

import config_util  # pylint: disable=import-error


# This class doesn't need an __init__ method, so we disable the warning
# pylint: disable=no-init
class WebRTC(config_util.Config):
    """Basic Config class for WebRTC."""
    @staticmethod
    def fetch_spec(props):
        url = 'https://webrtc.googlesource.com/src.git'
        spec = {
            'solutions': [
                {
                    'name': 'src',
                    'url': url,
                    'deps_file': 'DEPS',
                    'managed': False,
                    'custom_deps': {},
                },
            ],
            'with_branch_heads':
            True,
        }

        if props.get('target_os'):
            spec['target_os'] = props['target_os'].split(',')

        if props.get('cache_dir'):
            spec['cache_dir'] = props['cache_dir']

        return {
            'type': 'gclient_git',
            'gclient_git_spec': spec,
        }

    @staticmethod
    def expected_root(_props):
        return 'src'


def main(argv=None):
    return WebRTC().handle_args(argv)


if __name__ == '__main__':
    sys.exit(main(sys.argv))
