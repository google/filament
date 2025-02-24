# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import tempfile
import unittest

from telemetry.core import util
from telemetry.util import image_util
from telemetry.util import rgba_color

# This is a simple base64 encoded 2x2 PNG which contains, in order, a single
# Red, Yellow, Blue, and Green pixel.
test_png = """
 iVBORw0KGgoAAAANSUhEUgAAAAIAAAACCAIAAAD91
 JpzAAAAFklEQVR4Xg3EAQ0AAABAMP1LY3YI7l8l6A
 T8tgwbJAAAAABJRU5ErkJggg==
"""
test_png_path = os.path.join(util.GetUnittestDataDir(), 'test_png.png')
test_png_2_path = os.path.join(util.GetUnittestDataDir(), 'test_png_2.png')

class ImageUtilTest(unittest.TestCase):
  def testReadFromBase64Png(self):
    bmp = image_util.FromBase64Png(test_png)

    self.assertEqual(2, image_util.Width(bmp))
    self.assertEqual(2, image_util.Height(bmp))

    image_util.GetPixelColor(bmp, 0, 0).AssertIsRGB(255, 0, 0)
    image_util.GetPixelColor(bmp, 1, 1).AssertIsRGB(0, 255, 0)
    image_util.GetPixelColor(bmp, 0, 1).AssertIsRGB(0, 0, 255)
    image_util.GetPixelColor(bmp, 1, 0).AssertIsRGB(255, 255, 0)

  def testReadFromPngFile(self):
    file_bmp = image_util.FromPngFile(test_png_path)

    self.assertEqual(2, image_util.Width(file_bmp))
    self.assertEqual(2, image_util.Height(file_bmp))

    image_util.GetPixelColor(file_bmp, 0, 0).AssertIsRGB(255, 0, 0)
    image_util.GetPixelColor(file_bmp, 1, 1).AssertIsRGB(0, 255, 0)
    image_util.GetPixelColor(file_bmp, 0, 1).AssertIsRGB(0, 0, 255)
    image_util.GetPixelColor(file_bmp, 1, 0).AssertIsRGB(255, 255, 0)

  def testWritePngToPngFile(self):
    orig = image_util.FromPngFile(test_png_path)
    temp_file = tempfile.NamedTemporaryFile(suffix='.png').name
    image_util.WritePngFile(orig, temp_file)
    new_file = image_util.FromPngFile(temp_file)
    self.assertTrue(image_util.AreEqual(orig, new_file, likely_equal=True))

  def testWritePngWithoutPngSuffixThrows(self):
    orig = image_util.FromPngFile(test_png_path)
    temp_file = tempfile.NamedTemporaryFile().name
    self.assertRaises(AssertionError, image_util.WritePngFile,
                      orig, temp_file)

  def testWriteCroppedBmpToPngFile(self):
    pixels = [255, 0, 0, 255, 255, 0, 0, 0, 0,
              255, 255, 0, 0, 255, 0, 0, 0, 0]
    orig = image_util.FromRGBPixels(3, 2, pixels)
    orig = image_util.Crop(orig, 0, 0, 2, 2)
    temp_file = tempfile.NamedTemporaryFile(suffix='.png').name
    image_util.WritePngFile(orig, temp_file)
    new_file = image_util.FromPngFile(temp_file)
    self.assertTrue(image_util.AreEqual(orig, new_file, likely_equal=True))

  def testIsEqual(self):
    bmp = image_util.FromBase64Png(test_png)
    file_bmp = image_util.FromPngFile(test_png_path)
    self.assertTrue(image_util.AreEqual(bmp, file_bmp, likely_equal=True))

  def testDiff(self):
    file_bmp = image_util.FromPngFile(test_png_path)
    file_bmp_2 = image_util.FromPngFile(test_png_2_path)

    file_bmp_2 = image_util.Crop(file_bmp_2, 0, 0, 3, 1)

    diff_bmp = image_util.Diff(file_bmp, file_bmp)

    self.assertEqual(2, image_util.Width(diff_bmp))
    self.assertEqual(2, image_util.Height(diff_bmp))

    image_util.GetPixelColor(diff_bmp, 0, 0).AssertIsRGB(0, 0, 0)
    image_util.GetPixelColor(diff_bmp, 1, 1).AssertIsRGB(0, 0, 0)
    image_util.GetPixelColor(diff_bmp, 0, 1).AssertIsRGB(0, 0, 0)
    image_util.GetPixelColor(diff_bmp, 1, 0).AssertIsRGB(0, 0, 0)

    diff_bmp = image_util.Diff(file_bmp, file_bmp_2)

    self.assertEqual(3, image_util.Width(diff_bmp))
    self.assertEqual(2, image_util.Height(diff_bmp))

    image_util.GetPixelColor(diff_bmp, 0, 0).AssertIsRGB(0, 255, 255)
    image_util.GetPixelColor(diff_bmp, 1, 0).AssertIsRGB(0, 0, 255)
    image_util.GetPixelColor(diff_bmp, 2, 0).AssertIsRGB(255, 255, 255)

    image_util.GetPixelColor(diff_bmp, 0, 1).AssertIsRGB(0, 0, 255)
    image_util.GetPixelColor(diff_bmp, 1, 1).AssertIsRGB(0, 255, 0)
    image_util.GetPixelColor(diff_bmp, 2, 1).AssertIsRGB(0, 0, 0)

  def testGetBoundingBox(self):
    pixels = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    bmp = image_util.FromRGBPixels(4, 3, pixels)
    box, count = image_util.GetBoundingBox(bmp, rgba_color.RgbaColor(1, 0, 0))
    self.assertEqual(box, (1, 1, 2, 1))
    self.assertEqual(count, 2)

    box, count = image_util.GetBoundingBox(bmp, rgba_color.RgbaColor(0, 1, 0))
    self.assertEqual(box, None)
    self.assertEqual(count, 0)

  def testCrop(self):
    pixels = [0, 0, 0, 1, 0, 0, 2, 0, 0, 3, 0, 0,
              0, 1, 0, 1, 1, 0, 2, 1, 0, 3, 1, 0,
              0, 2, 0, 1, 2, 0, 2, 2, 0, 3, 2, 0]
    bmp = image_util.FromRGBPixels(4, 3, pixels)
    bmp = image_util.Crop(bmp, 1, 2, 2, 1)

    self.assertEqual(2, image_util.Width(bmp))
    self.assertEqual(1, image_util.Height(bmp))
    image_util.GetPixelColor(bmp, 0, 0).AssertIsRGB(1, 2, 0)
    image_util.GetPixelColor(bmp, 1, 0).AssertIsRGB(2, 2, 0)
    self.assertEqual(image_util.Pixels(bmp), bytearray([1, 2, 0, 2, 2, 0]))
