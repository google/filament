# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import contextlib
import os
import tempfile

from tracing.mre import cloud_storage


class FilePreparationError(Exception):
  """Raised if something goes wrong while preparing a file for processing."""


class FileHandle(object):
  def __init__(self, canonical_url):
    self._canonical_url = canonical_url

  @property
  def canonical_url(self):
    return self._canonical_url

  @contextlib.contextmanager
  def PrepareFileForProcessing(self):
    """Ensure that the URL to the file will be acessible during processing.

    This function must do any pre-work to ensure that mappers will
    be able to read from the URL contained in the file handle.

    Raises:
      FilePreparationError: If something went wrong while preparing the file.
    """
    yield self._WillProcess()
    self._DidProcess()

  def _WillProcess(self):
    raise NotImplementedError()

  def _DidProcess(self):
    raise NotImplementedError()


class URLFileHandle(FileHandle):
  def __init__(self, canonical_url, url_to_load):
    super(URLFileHandle, self).__init__(canonical_url)

    self._url_to_load = url_to_load

  def AsDict(self):
    return {
        'type': 'url',
        'canonical_url': self._canonical_url,
        'url_to_load': self._url_to_load
    }

  def _WillProcess(self):
    return self

  def _DidProcess(self):
    pass


class GCSFileHandle(FileHandle):
  def __init__(self, canonical_url, cache_directory):
    super(GCSFileHandle, self).__init__(canonical_url)
    file_name = canonical_url.split('/')[-1]
    self.cache_file = os.path.join(
        cache_directory, file_name + '.gz')

  def _WillProcess(self):
    if not os.path.exists(self.cache_file):
      try:
        cloud_storage.Copy(self.canonical_url, self.cache_file)
      except cloud_storage.CloudStorageError:
        return None
    return URLFileHandle(self.canonical_url, 'file://' + self.cache_file)

  def _DidProcess(self):
    pass


class InMemoryFileHandle(FileHandle):
  def __init__(self, canonical_url, data):
    super(InMemoryFileHandle, self).__init__(canonical_url)

    self.data = data
    self._temp_file_path = None

  def _WillProcess(self):
    temp_file = tempfile.NamedTemporaryFile(delete=False)
    temp_file.write(self.data)
    temp_file.close()
    self._temp_file_path = temp_file.name

    return URLFileHandle(self.canonical_url, 'file://' + self._temp_file_path)

  def _DidProcess(self):
    os.remove(self._temp_file_path)
    self._temp_file_path = None
