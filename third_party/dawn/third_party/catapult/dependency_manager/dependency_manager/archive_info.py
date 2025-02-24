# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import glob
import os
import shutil

from dependency_manager import exceptions
from dependency_manager import dependency_manager_util


class ArchiveInfo():

  def __init__(self, archive_file, unzip_path, path_within_archive,
               stale_unzip_path_glob=None):
    """ Container for the information needed to unzip a downloaded archive.

    Args:
        archive_path: Path to the archive file.
        unzip_path: Path to unzip the archive into. Assumes that this path
            is unique for the archive.
        path_within_archive: Specify if and how to handle zip archives
            downloaded from cloud_storage. Expected values:
                None: Do not unzip the file downloaded from cloud_storage.
                '.': Unzip the file downloaded from cloud_storage. The
                    unzipped file/folder is the expected dependency.
                file_path: Unzip the file downloaded from cloud_storage.
                    |file_path| is the path to the expected dependency,
                    relative to the unzipped archive path.
        stale_unzip_path_glob: Optional argument specifying a glob matching
            string which matches directories that should be removed before this
            archive is extracted (if it is extracted at all).
    """
    self._archive_file = archive_file
    self._unzip_path = unzip_path
    self._path_within_archive = path_within_archive
    self._dependency_path = os.path.join(
        self._unzip_path, self._path_within_archive)
    self._stale_unzip_path_glob = stale_unzip_path_glob
    if not self._has_minimum_data:
      raise ValueError(
          'Not enough information specified to initialize an archive info.'
          ' %s' % self)

  def GetUnzippedPath(self):
    if self.ShouldUnzipArchive():
      # Remove stale unzip results
      if self._stale_unzip_path_glob:
        for path in glob.glob(self._stale_unzip_path_glob):
          shutil.rmtree(path, ignore_errors=True)
      # TODO(aiolos): Replace UnzipFile with zipfile.extractall once python
      # version 2.7.4 or later can safely be assumed.
      dependency_manager_util.UnzipArchive(
          self._archive_file, self._unzip_path)
      if self.ShouldUnzipArchive():
        raise exceptions.ArchiveError(
            "Expected path '%s' was not extracted from archive '%s'." %
            (self._dependency_path, self._archive_file))
    return self._dependency_path

  def ShouldUnzipArchive(self):
    if not self._has_minimum_data:
      raise exceptions.ArchiveError(
          'Missing needed info to unzip archive. Know data: %s' % self)
    return not os.path.exists(self._dependency_path)

  @property
  def _has_minimum_data(self):
    return all([self._archive_file, self._unzip_path,
                self._dependency_path])

  def __repr__(self):
    return (
        'ArchiveInfo(archive_file=%s, unzip_path=%s, path_within_archive=%s, '
        'dependency_path =%s)' % (
            self._archive_file, self._unzip_path, self._path_within_archive,
            self._dependency_path))
