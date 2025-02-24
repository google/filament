# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""A binary manager the prioritizes local versions first.

This is a wrapper around telemetry.internal.util.binary_manager which first
checks for local versions of dependencies in the build directory before falling
back to whatever binary_manager finds, typically versions downloaded from
Google Storage.

This is not meant to be used everywhere, but is useful for dependencies that
get stale relatively quickly and produce hard-to-diagnose issues such as
dependencies for stack symbolization.
"""

from __future__ import absolute_import
import datetime
import logging
import os
from typing import Optional

from telemetry.core import exceptions
from telemetry.internal.util import binary_manager


class LocalFirstBinaryManager():
  """Singleton wrapper around telemetry.internal.util.binary_manager.

  Prioritizes locally built versions of dependencies.
  """
  _instance: Optional['LocalFirstBinaryManager'] = None

  def __init__(self, build_dir, browser_binary, os_name, arch,
               ignored_dependencies, os_version):
    """
    Args:
      build_dir: A string containing a path to the build directory used to
          build the browser being used. Can be None, in which case the fallback
          to binary_manager will be immediate.
      browser_binary: A string containing a path to the browser binary that will
          be used. Can be None, in which case dependency staleness cannot be
          determined.
      os_name: A string containing the OS name that will be used with
          binary_manager if fallback is required, e.g. "linux".
      arch: A string containing the architecture that will be used with
          binary manager if fallback is required, e.g. "x86_64".
      ignored_dependencies: A list of strings containing names of dependencies
          to skip the local check for.
      os_version: A string containing a specific OS version that will be used
          with binary_manager if fallback is required.
    """
    assert LocalFirstBinaryManager._instance is None
    self._build_dir = build_dir
    self._os = os_name
    self._arch = arch
    self._ignored_dependencies = ignored_dependencies
    self._os_version = os_version
    self._dependency_cache = {}
    self._browser_mtime = None
    if browser_binary:
      mtime = os.path.getmtime(browser_binary)
      self._browser_mtime = datetime.date.fromtimestamp(mtime)

  def FetchPath(self, dependency_name):
    """Fetches a path for |dependency_name|.

    Checks for a locally built version of the dependency before falling back to
    using binary_manager.

    Args:
      dependency_name: A string containing the name of the dependency.

    Returns:
      A string containing the path to the dependency, or None if it could not be
      found.
    """
    if dependency_name not in self._dependency_cache:
      path = self._FetchLocalPath(dependency_name)
      if not path:
        try:
          path = self._FetchBinaryManagerPath(dependency_name)
        except binary_manager.NoPathFoundError:
          path = None
      self._dependency_cache[dependency_name] = path
    return self._dependency_cache[dependency_name]

  def _FetchLocalPath(self, dependency_name):
    """Fetches the path for the locally built dependency if possible.

    Args:
      dependency_name: A string containing the name of the dependency.

    Returns:
      A string containing the path to the dependency, or None if it could not be
      found.
    """
    if not self._build_dir:
      logging.info(
          'No build directory set for LocalFirstBinaryManager, not looking for '
          'local version of %s', dependency_name)
      return None
    if dependency_name in self._ignored_dependencies:
      logging.info('Not looking for local version of ignored dependency %s',
                   dependency_name)
      return None
    local_path = os.path.join(self._build_dir, dependency_name)
    # Try the .exe version on Windows if the non-.exe version does not exist.
    if not os.path.exists(local_path) and self._os == 'win':
      local_path = local_path + '.exe'
    if not os.path.exists(local_path):
      logging.info(
          'No local version of dependency found for %s', dependency_name)
      return None
    logging.info(
        'Found local version of dependency %s: %s', dependency_name, local_path)

    # Check the mtime of the dependency relative to the browser executable - if
    # they're far apart, it could mean that we're using a stale version. It's
    # likely still better than using the downloaded version, but is worth
    # warning the user about.
    if not self._browser_mtime:
      logging.info(
          'No browser executable was provided - unable to verify staleness of '
          'local dependency %s', dependency_name)
    else:
      dependency_mtime = datetime.date.fromtimestamp(
          os.path.getmtime(local_path))
      time_delta = self._browser_mtime - dependency_mtime
      if abs(time_delta.days) > 14:
        logging.warning(
            'Local dependency %s was built more than two weeks apart from the '
            'browser it is being used with - this may indicate that stale'
            'dependencies are present.', dependency_name)
    return local_path

  def _FetchBinaryManagerPath(self, dependency_name):
    """Fetches the path for the dependency via binary_manager.

    Initializes binary_manager with defaults if it is not already initialized.

    Args:
      dependency_name: A string containing the name of the dependency.

    Returns:
      A string containing the path to the dependency, or None if it could not be
      found.
    """
    if binary_manager.NeedsInit():
      logging.info(
          'binary_manager was not initialized. Initializing with default '
          'values.')
      binary_manager.InitDependencyManager(None)
    return binary_manager.FetchPath(
        dependency_name, self._os, self._arch, self._os_version)

  @classmethod
  def NeedsInit(cls):
    return not cls._instance

  @classmethod
  def Init(cls, build_dir, browser_binary, os_name, arch,
           ignored_dependencies=None, os_version=None):
    """Initializes the singleton.

    Args:
      See constructor.
    """
    if not cls.NeedsInit():
      raise exceptions.InitializationError(
          'Tried to re-initialize LocalFirstBinarymanager with build dir %s '
          'and browser binary %s' % (build_dir, browser_binary))
    ignored_dependencies = ignored_dependencies or []
    cls._instance = LocalFirstBinaryManager(
        build_dir, browser_binary, os_name, arch, ignored_dependencies,
        os_version)

  @classmethod
  def GetInstance(cls):
    """Returns the singleton instance."""
    if cls.NeedsInit():
      raise exceptions.InitializationError(
          'Attempted to get LocalFirstBinaryManager without prior '
          'initialization.')
    assert cls._instance is not None  # Necessary for type checker.
    return cls._instance


# Helper to prevent having to type out an extra LocalFirstBinaryManager every
# time it's used.
def GetInstance():
  return LocalFirstBinaryManager.GetInstance()
