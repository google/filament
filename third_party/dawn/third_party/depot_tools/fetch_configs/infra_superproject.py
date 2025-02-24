# Copyright 2023 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
import ast

import config_util  # pylint: disable=import-error


# This class doesn't need an __init__ method, so we disable the warning
# pylint: disable=no-init
class InfraSuperproject(config_util.Config):
    """Basic Config class for the whole set of Infrastructure repositories."""
    @staticmethod
    def fetch_spec(props):
        def url(host, repo):
            return 'https://%s.googlesource.com/%s.git' % (host, repo)

        spec = {
            'solutions': [
                {
                    'name': '.',
                    'url': url('chromium', 'infra/infra_superproject'),
                    'managed': True,
                    'custom_vars': {},
                },
            ],
        }
        if ast.literal_eval(props.get('checkout_internal', 'False')):
            spec['solutions'][0]['custom_vars']['checkout_internal'] = True
        return {
            'type': 'gclient_git',
            'gclient_git_spec': spec,
        }

    @staticmethod
    def expected_root(_props):
        return '.'


def main(argv=None):
    return InfraSuperproject().handle_args(argv)


if __name__ == '__main__':
    sys.exit(main(sys.argv))
