# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
    'recipe_engine/cipd',
    'recipe_engine/context',
    'recipe_engine/json',
    'recipe_engine/path',
    'recipe_engine/step',
]

from recipe_engine.recipe_api import Property
from recipe_engine.config import ConfigGroup, Single

PROPERTIES = {
  '$depot_tools/windows_sdk': Property(
    help='Properties specifically for the infra windows_sdk module.',
    param_name='sdk_properties',
    kind=ConfigGroup(
      # CIPD instance ID, tag or ref for the Windows SDK version.
      version=Single(str),
    ), default={'version': 'uploaded:2018-06-13'},
  )
}
