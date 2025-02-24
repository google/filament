# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import builtins
import codecs
import collections
import os
import six

from io import BytesIO


class WithableStringIO(six.StringIO):

  def __enter__(self, *args):
    return self

  def __exit__(self, *args):
    pass

class WithableBytesIO(BytesIO):

  def __enter__(self, *args):
    return self

  def __exit__(self, *args):
    pass

class FakeFS(object):

  def __init__(self, initial_filenames_and_contents=None):
    self._file_contents = {}
    if initial_filenames_and_contents:
      for k, v in six.iteritems(initial_filenames_and_contents):
        self._file_contents[k] = v

    self._bound = False
    self._real_codecs_open = codecs.open
    self._real_open = builtins.open

    self._real_abspath = os.path.abspath
    self._real_exists = os.path.exists
    self._real_walk = os.walk
    self._real_listdir = os.listdir

  def __enter__(self):
    self.Bind()
    return self

  def __exit__(self, *args):
    self.Unbind()

  def Bind(self):
    assert not self._bound
    codecs.open = self._FakeCodecsOpen
    builtins.open = self._FakeOpen
    os.path.abspath = self._FakeAbspath
    os.path.exists = self._FakeExists
    os.walk = self._FakeWalk
    os.listdir = self._FakeListDir
    self._bound = True

  def Unbind(self):
    assert self._bound
    codecs.open = self._real_codecs_open
    builtins.open = self._real_open
    os.path.abspath = self._real_abspath
    os.path.exists = self._real_exists
    os.walk = self._real_walk
    os.listdir = self._real_listdir
    self._bound = False

  def AddFile(self, path, contents):
    assert path not in self._file_contents
    path = os.path.normpath(path)
    self._file_contents[path] = contents

  def _FakeOpen(self, path, mode=None):
    if mode is None:
      mode = 'r'
    if mode == 'r' or mode == 'rU' or mode == 'rb':
      if path not in self._file_contents:
        return self._real_open(path, mode)

      if mode == 'rb':
        return WithableBytesIO(self._file_contents[path])
      else:
        return WithableStringIO(self._file_contents[path])

    raise NotImplementedError()

  def _FakeCodecsOpen(self, path, mode=None,
                      encoding=None):  # pylint: disable=unused-argument
    if mode is None:
      mode = 'r'
    if mode == 'r' or mode == 'rU' or mode == 'rb':
      if path not in self._file_contents:
        return self._real_open(path, mode)

      if mode == 'rb':
        return WithableBytesIO(self._file_contents[path])
      else:
        return WithableStringIO(self._file_contents[path])

    raise NotImplementedError()

  def _FakeAbspath(self, path):
    """Normalize the path and ensure it starts with os.path.sep.

    The tests all assume paths start with things like '/my/project',
    and this abspath implementaion makes that assumption work correctly
    on Windows.
    """
    normpath = os.path.normpath(path)
    if not normpath.startswith(os.path.sep):
      normpath = os.path.sep + normpath
    return normpath

  def _FakeExists(self, path):
    if path in self._file_contents:
      return True
    return self._real_exists(path)

  def _FakeWalk(self, top):
    assert os.path.isabs(top)
    all_filenames = list(self._file_contents.keys())
    pending_prefixes = collections.deque()
    pending_prefixes.append(top)
    visited_prefixes = set()
    while len(pending_prefixes):
      prefix = pending_prefixes.popleft()
      if prefix in visited_prefixes:
        continue
      visited_prefixes.add(prefix)
      if prefix.endswith(os.path.sep):
        prefix_with_trailing_sep = prefix
      else:
        prefix_with_trailing_sep = prefix + os.path.sep

      dirs = set()
      files = []
      for filename in all_filenames:
        if not filename.startswith(prefix_with_trailing_sep):
          continue
        relative_to_prefix = os.path.relpath(filename, prefix)

        dirpart = os.path.dirname(relative_to_prefix)
        if len(dirpart) == 0:
          files.append(relative_to_prefix)
          continue
        parts = dirpart.split(os.sep)
        if len(parts) == 0:
          dirs.add(dirpart)
        else:
          pending = os.path.join(prefix, parts[0])
          dirs.add(parts[0])
          pending_prefixes.appendleft(pending)

      dirs = sorted(dirs)
      yield prefix, dirs, files

  def _FakeListDir(self, dirname):
    raise NotImplementedError()
