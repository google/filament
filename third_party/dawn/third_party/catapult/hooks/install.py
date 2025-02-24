#!/usr/bin/env python3
# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
import os

_TOP_PATH = os.path.abspath(os.path.join(
    os.path.dirname(__file__), '..'))


class Link(object):

  def __init__(self, dst_path, src_path):
    self.dst_path = dst_path
    self.src_path = src_path

  def Update(self):
    full_src_path = os.path.join(_TOP_PATH, self.src_path)
    full_dst_path = os.path.join(_TOP_PATH, self.dst_path)

    full_dst_path_dirname = os.path.dirname(full_dst_path)

    src_path_rel = os.path.relpath(full_src_path, full_dst_path_dirname)

    assert os.path.exists(full_src_path)
    if not os.path.exists(full_dst_path_dirname):
      sys.stdout.write('ERROR\n\n')
      sys.stdout.write(' dst dir doesn\'t exist: %s\n' % full_dst_path_dirname)
      sys.stdout.write('\n\n')
      sys.exit(255)

    if os.path.exists(full_dst_path) or os.path.islink(full_dst_path):
      if not os.path.islink(full_dst_path):
        sys.stdout.write('ERROR\n\n')
        sys.stdout.write('  Cannot install %s, dst already exists:\n  %s\n' % (
          os.path.basename(self.src_path), full_dst_path))
        sys.stdout.write('\n\n')
        sys.exit(255)

      existing_src_path_rel = os.readlink(full_dst_path)
      if existing_src_path_rel == src_path_rel:
        return
      else:
        sys.stdout.write('ERROR\n\n')
        sys.stdout.write(
          '  Cannot install %s, because %s is linked elsewhere.\n' % (
          os.path.basename(self.src_path),
          os.path.relpath(full_dst_path)))
        sys.stdout.write('\n\n')
        sys.exit(255)

    os.symlink(src_path_rel, full_dst_path)


def InstallHooks():
  """Installs the git pre-push hooks."""
  if sys.platform == 'win32':
    return

  # Remove old pre-commit, see https://github.com/google/trace-viewer/issues/932
  old_precommit = os.path.join(_TOP_PATH, '.git', 'hooks', 'pre-commit')
  old_precommit_target = os.path.join(_TOP_PATH, 'hooks', 'pre_commit')
  if (os.path.islink(old_precommit) and
      os.path.abspath(os.readlink(old_precommit)) == old_precommit_target):
    os.remove(old_precommit)

  # The pre-push hook prevents forced pushes; see ./pre_push.
  Link(os.path.join('.git', 'hooks', 'pre-push'),
           os.path.join('hooks', 'pre_push')).Update()
