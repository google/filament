# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import contextlib
import os
import shutil
import tempfile


@contextlib.contextmanager
def NamedTemporaryDirectory(suffix='', prefix='tmp', dir=None):
  """A context manager that manages a temporary directory.

  This is a context manager version of tempfile.mkdtemp. The arguments to this
  function are the same as the arguments for that one.

  This can be used to automatically manage the lifetime of a temporary file
  without maintaining an open file handle on it. Doing so can be useful in
  scenarios where a parent process calls a child process to create a temporary
  file and then does something with the resulting file.
  """
  # This uses |dir| as a parameter name for consistency with mkdtemp.
  # pylint: disable=redefined-builtin

  d = tempfile.mkdtemp(suffix=suffix, prefix=prefix, dir=dir)
  try:
    yield d
  finally:
    shutil.rmtree(d)


@contextlib.contextmanager
def NamedTemporaryFile(mode='w+b', suffix='', prefix='tmp'):
  """A conext manager to hold a named temporary file.

  It's similar to Python's tempfile.NamedTemporaryFile except:
  - The file is _not_ deleted when you close the temporary file handle, so you
    can close it and then use the name of the file to re-open it later.
  - The file *is* always deleted when exiting the context managed code.
  """
  with NamedTemporaryDirectory() as temp_dir:
    yield tempfile.NamedTemporaryFile(
        mode=mode, suffix=suffix, prefix=prefix, dir=temp_dir, delete=False)


@contextlib.contextmanager
def TemporaryFileName(prefix='tmp', suffix=''):
  """A context manager to just get the path to a file that does not exist.

  The parent directory of the file is a newly clreated temporary directory,
  and the name of the file is just `prefix + suffix`. The file istelf is not
  created, you are in fact guaranteed that it does not exit.

  The entire parent directory, possibly including the named temporary file and
  any sibling files, is entirely deleted when exiting the context managed code.
  """
  with NamedTemporaryDirectory() as temp_dir:
    yield os.path.join(temp_dir, prefix + suffix)
