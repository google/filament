# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import absolute_import
import importlib
import logging

from py_utils import modules_util


# Map of module names to their required min/max version.
# DEPRECATED: Do not add new modules to this dict. New external depencencies
# should be provided via vpython instead. See: crbug.com/777865.
MODULES = {
    'cv2': ('4.5.3', None),
    'numpy': ('1.8.0', '1.23.6'),
    'psutil': ('0.5.0', None),
}


def ImportRequiredModule(module):
  """Tries to import the desired module (DEPRECATED).

  External modules should be provided via vpython and imported using the
  regular python `import` statement. To ensure a particular module version
  has been loaded use modules_util.RequireVersion instead.

  Returns:
    The module on success, raises error on failure.
  Raises:
    ImportError: The import failed."""
  versions = MODULES.get(module)
  if versions is None:
    raise NotImplementedError('Please teach telemetry about module %s.' %
                              module)

  module = importlib.import_module(module)
  modules_util.RequireVersion(module, *versions)
  return module


def ImportOptionalModule(module):
  """Tries to import the desired module (DEPRECATED).

  External modules should be provided via vpython and imported using the
  regular python `import` statement. To ensure a particular module version
  has been loaded use modules_util.RequireVersion instead.

  Returns:
    The module if successful, None if not."""
  try:
    return ImportRequiredModule(module)
  except ImportError as e:
    # This can happen due to a circular dependency. It is usually not a
    # failure to import module_name, but a failed import somewhere in
    # the implementation. It's important to re-raise the error here
    # instead of failing silently.
    if 'cannot import name' in str(e):
      print('Possible circular dependency!')
      raise
    logging.info('Unable to import %s due to: %s', module, e)
    return None
