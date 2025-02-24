# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import collections.abc
import os


class PathSet(collections.abc.MutableSet):
  """A set of paths.

  All mutation methods can take both directories or individual files, but the
  iterator yields the individual files. All paths are automatically normalized.
  """
  def __init__(self, iterable=None):
    self._paths = set()
    if iterable:
      self |= iterable

  def __contains__(self, path):
    return os.path.realpath(path) in self._paths

  def __iter__(self):
    return iter(self._paths)

  def __len__(self):
    return len(self._paths)

  def add(self, value):
    value = os.path.realpath(value)
    if os.path.isfile(value):
      self._paths.add(value)
    for root, _, files in os.walk(value):
      for basename in files:
        file_path = os.path.join(root, basename)
        if os.path.isfile(file_path):
          self._paths.add(file_path)

  def discard(self, value):
    value = os.path.realpath(value)
    self._paths.discard(value)
    for root, _, files in os.walk(value):
      for basename in files:
        self._paths.discard(os.path.join(root, basename))
