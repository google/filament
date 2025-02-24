 # Copyright 2019 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import hashlib
import logging
import os
import sys

if sys.version_info.major == 2:
  import urlparse
  import urllib
  url_quote = urllib.quote
else:
  import urllib.parse as urlparse
  url_quote = urlparse.quote

from typ.host import Host

WINDOWS_FORBIDDEN_PATH_CHARACTERS = [
  '<',
  '>',
  ':',
  '"',
  '/',
  '\\',
  '|',
  '?',
  '*',
]

WINDOWS_MAX_PATH = 260
MAC_MAX_FILE_NAME = 255

class Artifacts(object):
  def __init__(self, output_dir, host, iteration=0, artifacts_base_dir='',
               intial_results_base_dir=False, repeat_tests=False):
    """Creates an artifact results object.

    This provides a way for tests to write arbitrary files to disk, either to
    be processed at a later point by a recipe or merge script, or simply as a
    way of saving additional data for users to later view.

    Artifacts are saved to disk in the following hierarchy in the output
    directory:
      * retry_x if the test is being retried for the xth time. If x is 0
            then the files will be saved directly to the output_dir directory,
            unless the intial_results_base_dir argument is set to True. If it set
            to true then a "intial" sub directory will be created and that is
            where the artifacts will be saved.
      * artifacts_base_dir if the argument is specified. If it is not specified,
            then the files will be saved to the retry_x, intial or the output_dir directory.
      * relative file path
    For example,  an artifact with path "images/screenshot.png" for the first iteration of
    the "TestFoo.test_bar" test will have the path:
    TestFoo.test_bar/retry_1/images/screenshot.png

    See https://chromium.googlesource.com/chromium/src/+/master/docs/testing/json_test_results_format.md
    for documentation on the output format for artifacts.

    The original design doc for this class can be found at
    https://docs.google.com/document/d/1gChmrnkHT8_MuSCKlGo-hGPmkEzg425E8DASX57ODB0/edit?usp=sharing,
    open to all chromium.org accounts.

    args:
      output_dir: Output directory where artifacts will be saved.
      iteration: Retry attempt number for test.
      artifacts_base_dir: Name of test, which will be used to create a sub directory for test artifacts
      intial_results_base_dir: Flag to create a sub directory for initial results
      repeat_tests: Flag to signal that tests are repeated and therefore the verification to prevent
          overwriting of artifacts should be skipped
      file_manager: File manager object which is supplied by the test runner. The object needs to support
          the exists, open, maybe_make_directory, dirname and join member functions.
    """
    # Make sure the output directory is an absolute path rather than a relative
    # one, as that can later affect our ability to write to disk on Windows due
    # to MAX_PATH.
    self._output_dir = output_dir
    if self._output_dir is not None and host is not None:
      self._output_dir = host.realpath(self._output_dir)
    self._iteration = iteration
    self._artifacts_base_dir = artifacts_base_dir
    # Replace invalid Windows path characters now with URL-encoded equivalents.
    self._platform = (host.platform if host and hasattr(host, 'platform')
                      else sys.platform)
    if self._platform == 'win32':
      for c in WINDOWS_FORBIDDEN_PATH_CHARACTERS:
        self._artifacts_base_dir = self._artifacts_base_dir.replace(
            c, url_quote(c))
    # A map of artifact names to their filepaths relative to the output
    # directory.
    self.artifacts = {}
    self._artifact_set = set()
    self._intial_results_base_dir = intial_results_base_dir
    self._repeat_tests = repeat_tests
    self._host = host

  def ArtifactsSubDirectory(self):
    sub_dir = self._artifacts_base_dir
    if self._iteration:
        sub_dir = self._host.join(sub_dir, 'retry_%d' % self._iteration)
    elif self._intial_results_base_dir:
        sub_dir = self._host.join(sub_dir, 'initial')
    return sub_dir

  def AddArtifact(self, artifact_name, path, raise_exception_for_duplicates=True):
    if path in self.artifacts.get(artifact_name, []):
        if not self._repeat_tests and raise_exception_for_duplicates:
            raise ValueError('%s already exists in artifacts list for %s.' % (
                path, artifact_name))
        else:
            return
    self.artifacts.setdefault(artifact_name, []).append(path)

  def CreateArtifact(
    self, artifact_name, file_relative_path, data, force_overwrite=False,
    write_as_text=False):
    """Creates an artifact and yields a handle to its File object.

    Args:
      artifact_name: A string specifying the name for the artifact, such as
          "reftest_mismatch_actual" or "screenshot".
    """
    self._AssertOutputDir()
    try:
      subdir_relative_path, abs_artifact_path = (
          self._GetSubDirRelativeAndAbsolutePaths(file_relative_path))
    except PathTooLongException as e:
      logging.error(str(e))
      return

    if (not self._repeat_tests and
            not force_overwrite and self._host.exists(abs_artifact_path)):
        raise ValueError('%s already exists.' % abs_artifact_path)

    self._host.maybe_make_directory(self._host.dirname(abs_artifact_path))

    if subdir_relative_path not in self.artifacts.get(artifact_name, []):
        self.AddArtifact(artifact_name, subdir_relative_path)
    if write_as_text:
        self._host.write_text_file(abs_artifact_path, data)
    else:
        self._host.write_binary_file(abs_artifact_path, data)

  def CreateLink(self, artifact_name, path):
    """Creates a special link/URL artifact.

    Instead of providing a File handle to be written to, the provided |path|
    will be directly used as the artifact's path.

    Args:
      artifact_name: A string specifying the name for the artifact, such as
          "triage_url".
      path: A string to be used for the artifact's path. Must be an HTTPS
          URL.
    """
    # Don't need to assert that we have an output dir since we aren't writing
    # any files.
    path = path.strip()
    # Make sure that what we're given is at least vaguely URL-like.
    parse_result = urlparse.urlparse(path)
    if not parse_result.scheme or not parse_result.netloc or len(
        path.splitlines()) > 1:
      raise ValueError('Given path %s does not appear to be a URL' % path)
    if parse_result.scheme != 'https':
      raise ValueError('Only HTTPS URLs are supported.')
    self.artifacts[artifact_name] = [path]

  def _AssertOutputDir(self):
    if not self._output_dir:
      raise ValueError(
          'CreateArtifact() called on an Artifacts instance without an output '
          'directory set. To fix, pass --write-full-results-to to the test.')

  def _GetSubDirRelativeAndAbsolutePaths(self, file_relative_path):
    """Generates the subdir-relative and absolute paths for a file.

    Args:
      file_relative_path: A string containing the path for an artifact for a
          particular test.

    Returns:
      A tuple (subdir_relative_path, abs_path). |subdir_relative_path| is
      |file_relative_path| preceeded by the relative artifact subdirectory,
      which is typically the test name. |abs_path| is an absolute path version
      of |subdir_relative_path|.
    """
    subdir_relative_path = self._host.join(
        self.ArtifactsSubDirectory(), file_relative_path)
    # Mac has a 255 character limit for any one directory or file name, so if we
    # detect any cases of those, replace that section of the path with a hash of
    # that section.
    if self._platform == 'darwin':
      path_pieces = subdir_relative_path.split(self._host.sep)
      for i, piece in enumerate(path_pieces):
        if len(piece) <= MAC_MAX_FILE_NAME:
          continue
        m = hashlib.sha1()
        m.update(piece.encode('utf-8'))
        path_pieces[i] = m.hexdigest()
      subdir_relative_path = self._host.join(*path_pieces)
    abs_path = self._host.join(self._output_dir, subdir_relative_path)
    # Attempt to work around the 260 character path limit in Windows. This is
    # not guaranteed to solve the issue, but should address the common case of
    # test names being long.
    if self._platform == 'win32' and len(abs_path) >= WINDOWS_MAX_PATH:
      m = hashlib.sha1()
      m.update(self.ArtifactsSubDirectory().encode('utf-8'))
      subdir_relative_path = self._host.join(m.hexdigest(), file_relative_path)
      abs_path = self._host.join(self._output_dir, subdir_relative_path)
      if len(abs_path) < WINDOWS_MAX_PATH:
        return (subdir_relative_path, abs_path)
      raise PathTooLongException(
          'Path %s exceeds Windows MAX_PATH even when attempting to shorten.' %
          (self._host.join(self._output_dir, self.ArtifactsSubDirectory(),
                           file_relative_path)))
    return (subdir_relative_path, abs_path)


class PathTooLongException(Exception):
  pass
