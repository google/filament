# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""A Resource is a file and its various associated canonical names."""

from __future__ import absolute_import
import codecs
import os


class Resource(object):
  """Represents a file found via a path search."""

  def __init__(self, toplevel_dir, absolute_path, binary=False):
    self.toplevel_dir = toplevel_dir
    self.absolute_path = absolute_path
    self._contents = None
    self._binary = binary

  @property
  def relative_path(self):
    """The path to the file from the top-level directory"""
    return os.path.relpath(self.absolute_path, self.toplevel_dir)

  @property
  def unix_style_relative_path(self):
    return self.relative_path.replace(os.sep, '/')

  @property
  def name(self):
    """The dotted name for this resource based on its relative path."""
    return self.name_from_relative_path(self.relative_path)

  @staticmethod
  def name_from_relative_path(relative_path):
    dirname = os.path.dirname(relative_path)
    basename = os.path.basename(relative_path)
    modname = os.path.splitext(basename)[0]
    if len(dirname):
      name = dirname.replace(os.path.sep, '.') + '.' + modname
    else:
      name = modname
    return name

  @property
  def contents(self):
    if self._contents:
      return self._contents
    if not os.path.exists(self.absolute_path):
      raise Exception('%s not found.' % self.absolute_path)
    if self._binary:
      f = open(self.absolute_path, mode='rb')
    else:
      f = codecs.open(self.absolute_path, mode='r', encoding='utf-8')
    self._contents = f.read()
    f.close()
    return self._contents
