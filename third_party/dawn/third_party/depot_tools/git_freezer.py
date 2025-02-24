#!/usr/bin/env python3
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
import optparse

import subcommand

from git_common import freeze, thaw

import gclient_utils


def CMDfreeze(parser, args):
    """Freeze a branch's changes, excluding unstaged gitlinks changes."""
    parser.parse_args(args)
    return freeze()


def CMDthaw(parser, args):
    """Returns a frozen branch to the state before it was frozen."""
    parser.parse_args(args)
    return thaw()


def main(args):
    if gclient_utils.IsEnvCog():
        print(f'{args[0]} command is not supported in non-git environment.',
              file=sys.stderr)
        return 1
    dispatcher = subcommand.CommandDispatcher(__name__)
    ret = dispatcher.execute(optparse.OptionParser(), args)
    if ret:
        print(ret)
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main(sys.argv[1:]))
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
