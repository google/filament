#!/usr/bin/env python3
# Copyright 2023 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""This script is a wrapper around the //buildtools/reclient/reclientreport
binary that populates the log paths correctly for builds run via autoninja
Call this script with the same -C argument used for the autoninja build
Example usage:
$ reclientreport -C out/my-ninja-out
"""

import argparse
import os
import sys
import tarfile
import tempfile


# TODO(b/301574845): Remove once reclientreport binary saves all logs
def temp_impl_b_301574845(out_dir):
    '''Temporary implementation until b/301574845 is fixed'''
    log_dir = os.path.abspath(os.path.join(out_dir, '.reproxy_tmp', 'logs'))
    with tempfile.NamedTemporaryFile(prefix='reclientreport',
                                     suffix='.tar.gz',
                                     delete=False) as f:
        with tarfile.open(fileobj=f, mode='w:gz') as tar:
            tar.add(log_dir, arcname=os.path.basename(log_dir))
        print(f'Created log file at {f.name}. Please attach this to your bug '
              'report!')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ninja_out",
                        "-C",
                        required=True,
                        help="ninja out directory used for the autoninja build")
    parser.add_argument('args', nargs=argparse.REMAINDER)

    args, _ = parser.parse_known_args()
    temp_impl_b_301574845(args.ninja_out)
    #if sys.platform.startswith('win'):
    #    temp_win_impl__b_296402157(args.ninja_out)
    #    return
    #if args.args and args.args[0] == '--':
    #    args.args.pop(0)
    #if extras:
    #    args.args = extras + args.args

    #log_dir = os.path.join(args.ninja_out, '.reproxy_tmp', 'logs')
    #reclient_helper.set_reproxy_path_flags(args.ninja_out, make_dirs=False)
    #os.environ["RBE_proxy_log_dir"] = ",".join(
    #    os.path.join(log_dir, d) for d in os.listdir(log_dir))
    #reclient_bin_dir = reclient_helper.find_reclient_bin_dir()
    #code = subprocess.call([os.path.join(reclient_bin_dir, 'reclientreport')] +
    #                       args.args)
    #if code != 0:
    #    print("Failed to collect logs, make sure that %s/.reproxy_tmp exists" %
    #          args.ninja_out,
    #          file=sys.stderr)


if __name__ == '__main__':
    sys.exit(main())
