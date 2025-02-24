# Copyright 2022 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""A wrapper for common operations on a device with Cast capabilities."""

import os

from telemetry.core import util

CAST_BROWSERS = [
    'platform_app'
]
DEFAULT_CAST_CORE_DIR = os.path.join(util.GetCatapultDir(), '..', 'cast_core',
                                     'prebuilts')
DEFAULT_CWR_EXE = os.path.join(util.GetCatapultDir(), '..', 'cast_web_runtime')
SSH_PWD = "root"
SSH_USER = "root"
