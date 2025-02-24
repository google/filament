# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Shell scripting helpers (created for Telemetry dependency roll scripts).

from __future__ import print_function

from __future__ import absolute_import
import os as _os
import shutil as _shutil
import subprocess as _subprocess
import tempfile as _tempfile
from contextlib import contextmanager as _contextmanager

@_contextmanager
def ScopedChangeDir(new_path):
  old_path = _os.getcwd()
  _os.chdir(new_path)
  print('> cd', _os.getcwd())
  try:
    yield
  finally:
    _os.chdir(old_path)
    print('> cd', old_path)

@_contextmanager
def ScopedTempDir():
  temp_dir = _tempfile.mkdtemp()
  try:
    with ScopedChangeDir(temp_dir):
      yield
  finally:
    _shutil.rmtree(temp_dir)

def CallProgram(path_parts, *args, **kwargs):
  '''Call an executable os.path.join(*path_parts) with the arguments specified
  by *args. Any keyword arguments are passed as environment variables.'''
  args = [_os.path.join(*path_parts)] + list(args)
  env = dict(_os.environ)
  env.update(kwargs)
  print('>', ' '.join(args))
  _subprocess.check_call(args, env=env)
