# Copyright 2020 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""ANGLE implementation of //build/skia_gold_common/skia_gold_properties.py."""

import angle_path_util

from skia_gold_common import skia_gold_properties


class ANGLESkiaGoldProperties(skia_gold_properties.SkiaGoldProperties):

    def _GetGitRepoDirectory(self):
        return angle_path_util.ANGLE_ROOT_DIR
