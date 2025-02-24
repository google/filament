#!/usr/bin/env python3
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
Checks out a downstream branch from the currently checked out branch. If there
is more than one downstream branch, then this script will prompt you to select
which branch.
"""

import argparse
import sys

from git_common import current_branch, branches, upstream, run, hash_one

import gclient_utils
import metrics


@metrics.collector.collect_metrics('git nav-downstream')
def main(args):
    if gclient_utils.IsEnvCog():
        print('nav-downstream command is not supported in non-git environment.',
              file=sys.stderr)
        return 1
    parser = argparse.ArgumentParser()
    parser.add_argument('--pick',
                        help=('The number to pick if this command would '
                              'prompt'))
    opts = parser.parse_args(args)

    upfn = upstream
    cur = current_branch()
    if cur == 'HEAD':

        def _upfn(b):
            parent = upstream(b)
            if parent:
                return hash_one(parent)

        upfn = _upfn
        cur = hash_one(cur)
    downstreams = [b for b in branches() if upfn(b) == cur]
    if not downstreams:
        print("No downstream branches")
        return 1

    if len(downstreams) == 1:
        run('checkout', downstreams[0], stdout=sys.stdout, stderr=sys.stderr)
    else:
        high = len(downstreams) - 1
        while True:
            print("Please select a downstream branch")
            for i, b in enumerate(downstreams):
                print("  %d. %s" % (i, b))
            prompt = "Selection (0-%d)[0]: " % high
            r = opts.pick
            if r:
                print(prompt + r)
            else:
                r = gclient_utils.AskForData(prompt).strip() or '0'
            if not r.isdigit() or (0 > int(r) > high):
                print("Invalid choice.")
            else:
                run('checkout',
                    downstreams[int(r)],
                    stdout=sys.stdout,
                    stderr=sys.stderr)
                break
    return 0


if __name__ == '__main__':
    with metrics.collector.print_notice_and_exit():
        sys.exit(main(sys.argv[1:]))
