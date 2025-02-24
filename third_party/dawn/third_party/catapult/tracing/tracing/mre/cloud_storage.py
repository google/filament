# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import os
import subprocess
import sys

_GSUTIL_PATH = os.path.abspath(
    os.path.join(
        os.path.dirname(__file__),
        '..', '..', 'third_party', 'gsutil', 'gsutil'))


class CloudStorageError(Exception):

  @staticmethod
  def _GetConfigInstructions():
    command = _GSUTIL_PATH
    return ('To configure your credentials:\n'
            '  1. Run "%s config" and follow its instructions.\n'
            '  2. If you have a @google.com account, use that account.\n'
            '  3. For the project-id, just enter 0.' % command)


# TODO(https://1277796): Rename this once we're Python 3-only.
# pylint: disable=redefined-builtin
class PermissionError(CloudStorageError):

  def __init__(self):
    super(PermissionError, self).__init__(
        'Attempted to access a file from Cloud Storage but you don\'t '
        'have permission. ' + self._GetConfigInstructions())


class CredentialsError(CloudStorageError):

  def __init__(self):
    super(CredentialsError, self).__init__(
        'Attempted to access a file from Cloud Storage but you have no '
        'configured credentials. ' + self._GetConfigInstructions())


class NotFoundError(CloudStorageError):
  pass


class ServerError(CloudStorageError):
  pass


def Copy(src, dst):
  # TODO(simonhatch): switch to use py_utils.cloud_storage.
  args = [sys.executable, _GSUTIL_PATH, 'cp', src, dst]
  gsutil = subprocess.Popen(args, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
  _, stderr = gsutil.communicate()

  if gsutil.returncode:
    if stderr.startswith((
        'You are attempting to access protected data with no configured',
        'Failure: No handler was ready to authenticate.')):
      raise CredentialsError()
    if ('status=403' in stderr or 'status 403' in stderr or
        '403 Forbidden' in stderr):
      raise PermissionError()
    if (stderr.startswith('InvalidUriError') or 'No such object' in stderr or
        'No URLs matched' in stderr or
        'One or more URLs matched no' in stderr):
      raise NotFoundError(stderr)
    if '500 Internal Server Error' in stderr:
      raise ServerError(stderr)
    raise CloudStorageError(stderr)
  return gsutil.returncode
