# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from distutils import version  # pylint: disable=no-name-in-module


def RequireVersion(module, min_version, max_version=None):
  """Ensure that an imported module's version is within a required range.

  Version strings are parsed with LooseVersion, so versions like "1.8.0rc1"
  (default numpy on macOS Sierra) and "2.4.13.2" (a version of OpenCV 2.x)
  are allowed.

  Args:
    module: An already imported python module.
    min_version: The module must have this or a higher version.
    max_version: Optional, the module should not have this or a higher version.

  Raises:
    ImportError if the module's __version__ is not within the allowed range.
  """
  module_version = version.LooseVersion(module.__version__)
  min_version = version.LooseVersion(str(min_version))
  valid_version = min_version <= module_version

  if max_version is not None:
    max_version = version.LooseVersion(str(max_version))
    valid_version = valid_version and (module_version < max_version)
    wants_version = 'at or above %s and below %s' % (min_version, max_version)
  else:
    wants_version = '%s or higher' % min_version

  if not valid_version:
    raise ImportError('%s has version %s, but version %s is required' % (
        module.__name__, module_version, wants_version))
