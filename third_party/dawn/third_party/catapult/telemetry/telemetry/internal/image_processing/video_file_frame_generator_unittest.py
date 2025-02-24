# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import unittest

from telemetry import decorators
from telemetry.core import util
from telemetry.internal.image_processing import frame_generator
from telemetry.internal.util import external_modules

try:
  cv2 = external_modules.ImportRequiredModule('cv2')
except (ImportError, NotImplementedError):
  pass
else:
  class VideoFileFrameGeneratorTest(unittest.TestCase):
    def __init__(self, *args, **kwargs):
      super().__init__(*args, **kwargs)
      # Import modules with dependencies that may not be preset in test setup so
      # that importing this unit test doesn't cause the test runner to raise an
      # exception.
      # pylint: disable=import-outside-toplevel
      from telemetry.internal.image_processing import video_file_frame_generator
      # pylint: enable=import-outside-toplevel
      self.VideoFileFrameGenerator = \
          video_file_frame_generator.VideoFileFrameGenerator

    # https://github.com/catapult-project/catapult/issues/3510
    @decorators.Disabled('all')
    @decorators.Isolated
    def testVideoFileFrameGeneratorSuccess(self):
      vid = os.path.join(util.GetUnittestDataDir(), 'screen_3_frames.mov')
      fg = self.VideoFileFrameGenerator(vid)
      timestamps = [0, 33.367, 66.733]
      self.assertTrue(isinstance(fg, frame_generator.FrameGenerator))

      self.assertEqual(fg.CurrentFrameNumber, -1)
      self.assertAlmostEqual(fg.CurrentTimestamp, 0, 3)
      self.assertEqual(fg.Dimensions, (432, 320))
      generator = fg.Generator
      i = 0
      for frame in generator:
        self.assertEqual(fg.CurrentFrameNumber, i)
        self.assertAlmostEqual(fg.CurrentTimestamp, timestamps[i], 3)
        self.assertEqual(fg.Dimensions, (432, 320))
        self.assertEqual(frame.shape[:2], (320, 432))
        i += 1
      self.assertEqual(i, 3)
      try:
        next(generator)
        stopped = False
      except StopIteration:
        stopped = True
      self.assertTrue(stopped)
      try:
        next(fg.Generator)
        stopped = False
      except StopIteration:
        stopped = True
      self.assertTrue(stopped)

    # https://github.com/catapult-project/catapult/issues/3510
    @decorators.Disabled('all')
    @decorators.Isolated
    def testVideoFileFrameGeneratorSkipFrames(self):
      vid = os.path.join(util.GetUnittestDataDir(), 'screen_3_frames.mov')
      fg = self.VideoFileFrameGenerator(vid, 2)
      self.assertEqual(fg.CurrentFrameNumber, 1)
      self.assertAlmostEqual(fg.CurrentTimestamp, 33.367, 3)
      self.assertEqual(fg.Dimensions, (432, 320))
      next(fg.Generator)
      try:
        next(fg.Generator)
        stopped = False
      except StopIteration:
        stopped = True
      self.assertTrue(stopped)

    # https://github.com/catapult-project/catapult/issues/3510
    @decorators.Disabled('all')
    @decorators.Isolated
    def testVideoFileFrameGeneratorFailure(self):
      vid = os.path.join(util.GetUnittestDataDir(), 'screen_3_frames.mov')
      try:
        self.VideoFileFrameGenerator(vid, 4)
        fail = False
      except frame_generator.FrameReadError:
        fail = True
      self.assertTrue(fail)

      try:
        self.VideoFileFrameGenerator('not_a_file', 0)
        fail = False
      except frame_generator.FrameReadError:
        fail = True
      self.assertTrue(fail)
