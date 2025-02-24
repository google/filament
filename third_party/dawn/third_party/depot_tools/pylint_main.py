#!/usr/bin/env python3
# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Chromium wrapper for pylint for passing args via stdin.

This will be executed by vpython with the right pylint versions.
"""

import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
PYLINT = os.path.join(HERE, 'pylint_main.py')
RC_FILE = os.path.join(HERE, 'pylintrc')

ARGS_ON_STDIN = '--args-on-stdin'


def find_rcfile() -> str:
    """Locate the config file for this wrapper."""
    arg0 = os.path.basename(sys.argv[0])
    if arg0.startswith('pylint-'):
        rc_file = RC_FILE + '-' + arg0.split('-', 1)[1]
        if os.path.exists(rc_file):
            return rc_file
    return RC_FILE


def main(argv):
    """Our main wrapper."""
    # Add support for a custom mode where arguments are fed line by line on
    # stdin. This allows us to get around command line length limitations.
    if ARGS_ON_STDIN in argv:
        argv = [x for x in argv if x != ARGS_ON_STDIN]
        argv.extend(x.strip() for x in sys.stdin)

    rc_file = find_rcfile()

    # Set default config options with the PYLINTRC environment variable. This
    # will allow overriding with "more local" config file options, such as a
    # local "pylintrc" file, the "--rcfile" command-line flag, or an existing
    # PYLINTRC.
    #
    # Note that this is not quite the same thing as replacing pylint's built-in
    # defaults, since, based on config file precedence, it will not be
    # overridden by "more global" config file options, such as ~/.pylintrc,
    # ~/.config/pylintrc, or /etc/pylintrc. This is generally the desired
    # behavior, since we want to enforce these defaults in most cases, but allow
    # them to be overridden for specific code or repos.
    #
    # If someone really doesn't ever want the depot_tools pylintrc, they can set
    # their own PYLINTRC, or set an empty PYLINTRC to use pylint's normal config
    # file resolution, which would include the "more global" options that are
    # normally overridden by the depot_tools config.
    os.environ.setdefault('PYLINTRC', rc_file)

    # This import has to happen after PYLINTRC is set because the module tries
    # to resolve the config file location on load.
    from pylint import lint  # pylint: disable=bad-option-value,import-outside-toplevel
    lint.Run(argv)


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
