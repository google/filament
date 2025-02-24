# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""This module contains a mapping of depot names to paths on gitiles.

This is used to fetch information from gitiles for different
repositories supported by auto-bisect.
"""

# For each entry in this map, the key is a "depot" name (a Chromium dependency
# in the DEPS file) and the value is a path used for the repo on gitiles; each
# repo can be found at https://chromium.googlesource.com/<PATH>.
DEPOT_PATH_MAP = {
    'chromium': 'chromium/src',
    'angle': 'angle/angle',
    'v8': 'v8/v8.git',
    'skia': 'skia',
}
