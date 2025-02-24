#!/usr/bin/env python3
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Usage:
#    gclient-new-workdir.py [options] <repository> <new_workdir>
#

import argparse
import os
import shutil
import subprocess
import sys
import textwrap

import gclient_utils
import git_common


def parse_options():
    if sys.platform == 'win32':
        print(
            'ERROR: This script cannot run on Windows because it uses symlinks.'
        )
        sys.exit(1)
    if gclient_utils.IsEnvCog():
        print('ERROR: This script cannot run in non-git environment.')
        sys.exit(1)

    parser = argparse.ArgumentParser(description='''\
      Clone an existing gclient directory, taking care of all sub-repositories.
      Works similarly to 'git new-workdir'.''')
    parser.add_argument('repository',
                        type=os.path.abspath,
                        help='should contain a .gclient file')
    parser.add_argument('new_workdir', help='must not exist')
    parser.add_argument('--reflink',
                        action='store_true',
                        default=None,
                        help='''force to use "cp --reflink" for speed and disk
                              space. need supported FS like btrfs or ZFS.''')
    parser.add_argument(
        '--no-reflink',
        action='store_false',
        dest='reflink',
        help='''force not to use "cp --reflink" even on supported
                              FS like btrfs or ZFS.''')
    args = parser.parse_args()

    if not os.path.exists(args.repository):
        parser.error('Repository "%s" does not exist.' % args.repository)

    gclient = os.path.join(args.repository, '.gclient')
    if not os.path.exists(gclient):
        parser.error('No .gclient file at "%s".' % gclient)

    if os.path.exists(args.new_workdir):
        parser.error('New workdir "%s" already exists.' % args.new_workdir)

    return args


def support_cow(src, dest):
    # 'cp --reflink' always succeeds when 'src' is a symlink or a directory
    assert os.path.isfile(src) and not os.path.islink(src)
    try:
        subprocess.check_output(['cp', '-a', '--reflink', src, dest],
                                stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError:
        return False
    finally:
        if os.path.isfile(dest):
            os.remove(dest)
    return True


def try_vol_snapshot(src, dest):
    try:
        subprocess.check_call(['btrfs', 'subvol', 'snapshot', src, dest],
                              stderr=subprocess.STDOUT)
    except (subprocess.CalledProcessError, OSError):
        return False
    return True


def main():
    args = parse_options()

    gclient = os.path.join(args.repository, '.gclient')
    if os.path.islink(gclient):
        gclient = os.path.realpath(gclient)
    new_gclient = os.path.join(args.new_workdir, '.gclient')

    if try_vol_snapshot(args.repository, args.new_workdir):
        args.reflink = True
    else:
        os.makedirs(args.new_workdir)
        if args.reflink is None:
            args.reflink = support_cow(gclient, new_gclient)
            if args.reflink:
                print('Copy-on-write support is detected.')
        os.symlink(gclient, new_gclient)

    for root, dirs, _ in os.walk(args.repository):
        if '.git' in dirs:
            workdir = root.replace(args.repository, args.new_workdir, 1)
            print('Creating: %s' % workdir)

            if args.reflink:
                if not os.path.exists(workdir):
                    print('Copying: %s' % workdir)
                    subprocess.check_call(
                        ['cp', '-a', '--reflink', root, workdir])
                shutil.rmtree(os.path.join(workdir, '.git'))

            git_common.make_workdir(os.path.join(root, '.git'),
                                    os.path.join(workdir, '.git'))
            if args.reflink:
                subprocess.check_call([
                    'cp', '-a', '--reflink',
                    os.path.join(root, '.git', 'index'),
                    os.path.join(workdir, '.git', 'index')
                ])
            else:
                subprocess.check_call(['git', 'checkout', '-f'], cwd=workdir)

    if args.reflink:
        print(
            textwrap.dedent('''\
      The repo was copied with copy-on-write, and the artifacts were retained.
      More details on http://crbug.com/721585.

      Depending on your usage pattern, you might want to do "gn gen"
      on the output directories. More details: http://crbug.com/723856.'''))


if __name__ == '__main__':
    sys.exit(main())
