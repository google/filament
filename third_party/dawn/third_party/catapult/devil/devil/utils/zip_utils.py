# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import json
import logging
import os
import sys
import zipfile

if __name__ == '__main__':
  _DEVIL_ROOT_DIR = os.path.abspath(
      os.path.join(os.path.dirname(__file__), '..', '..'))
  sys.path.append(_DEVIL_ROOT_DIR)

from devil import devil_env
from devil import base_error
from devil.utils import cmd_helper

with devil_env.SysPath(devil_env.PY_UTILS_PATH):
  from py_utils import tempfile_ext

logger = logging.getLogger(__name__)


class ZipFailedError(base_error.BaseError):
  """Raised on a failure to perform a zip operation."""


def _WriteToZipFile(zip_file, path, arc_path):
  """Recursively write |path| to |zip_file| as |arc_path|.

  zip_file: An open instance of zipfile.ZipFile.
  path: An absolute path to the file or directory to be zipped.
  arc_path: A relative path within the zip file to which the file or directory
    located at |path| should be written.
  """
  if os.path.isdir(path):
    for dir_path, _, file_names in os.walk(path):
      dir_arc_path = os.path.join(arc_path, os.path.relpath(dir_path, path))
      logger.debug('dir:  %s -> %s', dir_path, dir_arc_path)
      zip_file.write(dir_path, dir_arc_path, zipfile.ZIP_STORED)
      for f in file_names:
        file_path = os.path.join(dir_path, f)
        file_arc_path = os.path.join(dir_arc_path, f)
        logger.debug('file: %s -> %s', file_path, file_arc_path)
        zip_file.write(file_path, file_arc_path, zipfile.ZIP_STORED)
  else:
    logger.debug('file: %s -> %s', path, arc_path)
    zip_file.write(path, arc_path, zipfile.ZIP_STORED)


def _WriteZipFile(zip_path, zip_contents):
  with zipfile.ZipFile(zip_path, 'w') as zip_file:
    for path, arc_path in zip_contents:
      _WriteToZipFile(zip_file, path, arc_path)


def WriteZipFile(zip_path, zip_contents):
  """Writes the provided contents to the given zip file.

  Note that this uses python's zipfile module and is done in a separate
  process to avoid hogging the GIL.

  Args:
    zip_path: String path to the zip file to write.
    zip_contents: A list of (host path, archive path) tuples.

  Raises:
    ZipFailedError on failure.
  """
  zip_spec = {
      'zip_path': zip_path,
      'zip_contents': zip_contents,
  }
  with tempfile_ext.NamedTemporaryDirectory() as tmpdir:
    json_path = os.path.join(tmpdir, 'zip_spec.json')
    with open(json_path, 'w') as json_file:
      json.dump(zip_spec, json_file)
    ret, output, error = cmd_helper.GetCmdStatusOutputAndError(
        [sys.executable,
         os.path.abspath(__file__), '--zip-spec', json_path])

  if ret != 0:
    exc_msg = ['Failed to create %s' % zip_path]
    exc_msg.extend('stdout:  %s' % l for l in output.splitlines())
    exc_msg.extend('stderr:  %s' % l for l in error.splitlines())
    raise ZipFailedError('\n'.join(exc_msg))


def main(raw_args):
  parser = argparse.ArgumentParser()
  parser.add_argument('--zip-spec', required=True)

  args = parser.parse_args(raw_args)

  with open(args.zip_spec) as zip_spec_file:
    zip_spec = json.load(zip_spec_file)

  return _WriteZipFile(**zip_spec)


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
