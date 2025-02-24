# Copyright 2020 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""ANGLE implementation of //build/skia_gold_common/skia_gold_session_manager.py."""

from skia_gold_common import skia_gold_session_manager as sgsm
from .angle_skia_gold_session import ANGLESkiaGoldSession


class ANGLESkiaGoldSessionManager(sgsm.SkiaGoldSessionManager):

    @staticmethod
    def _GetDefaultInstance():
        return 'angle'

    @staticmethod
    def GetSessionClass():
        return ANGLESkiaGoldSession
