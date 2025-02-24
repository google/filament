# Copyright 2020 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""ANGLE implementation of //build/skia_gold_common/skia_gold_session.py."""

from skia_gold_common import output_managerless_skia_gold_session as omsgs


class ANGLESkiaGoldSession(omsgs.OutputManagerlessSkiaGoldSession):

    def _GetDiffGoldInstance(self):
        return str(self._instance)
