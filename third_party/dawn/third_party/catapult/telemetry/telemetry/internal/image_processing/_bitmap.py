# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Bitmap is a basic wrapper for image pixels. It includes some basic processing
tools: crop, find bounding box of a color and compute histogram of color values.
"""

from __future__ import absolute_import
import array

try:
  from cStringIO import StringIO
except ImportError:
  from io import BytesIO as StringIO

import struct
import subprocess
import warnings

from telemetry.internal.util import binary_manager
from telemetry.core import platform
from telemetry.util import color_histogram
from telemetry.util import rgba_color

import png


class _BitmapTools():
  """Wraps a child process of bitmaptools and allows for one command."""
  CROP_PIXELS = 0
  HISTOGRAM = 1
  BOUNDING_BOX = 2

  def __init__(self, dimensions, pixels):
    binary = binary_manager.FetchPath(
        'bitmaptools',
        platform.GetHostPlatform().GetOSName(),
        platform.GetHostPlatform().GetArchName())
    assert binary, 'You must build bitmaptools first!'

    self._popen = subprocess.Popen([binary],
                                   stdin=subprocess.PIPE,
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)

    # dimensions are: bpp, width, height, boxleft, boxtop, boxwidth, boxheight
    packed_dims = struct.pack('iiiiiii', *dimensions)
    self._popen.stdin.write(packed_dims)
    # If we got a list of ints, we need to convert it into a byte buffer.
    if not isinstance(pixels, bytearray):
      pixels = bytearray(pixels)
    self._popen.stdin.write(pixels)

  def _RunCommand(self, *command):
    assert not self._popen.stdin.closed, (
        'Exactly one command allowed per instance of tools.')
    packed_command = struct.pack('i' * len(command), *command)
    self._popen.stdin.write(packed_command)
    self._popen.stdin.close()
    length_packed = self._popen.stdout.read(struct.calcsize('i'))
    if not length_packed:
      raise Exception(self._popen.stderr.read())
    length = struct.unpack('i', length_packed)[0]
    return self._popen.stdout.read(length)

  def CropPixels(self):
    return self._RunCommand(_BitmapTools.CROP_PIXELS)

  def Histogram(self, ignore_color, tolerance):
    ignore_color_int = -1 if ignore_color is None else int(ignore_color)
    response = self._RunCommand(_BitmapTools.HISTOGRAM,
                                ignore_color_int, tolerance)
    out = array.array('i')
    out.frombytes(response)
    assert len(out) == 768, (
        'The ColorHistogram has the wrong number of buckets: %s' % len(out))
    return color_histogram.ColorHistogram(
        out[:256], out[256:512], out[512:], ignore_color)

  def BoundingBox(self, color, tolerance):
    response = self._RunCommand(_BitmapTools.BOUNDING_BOX, int(color),
                                tolerance)
    unpacked = struct.unpack('iiiii', response)
    box, count = unpacked[:4], unpacked[-1]
    if box[2] < 0 or box[3] < 0:
      box = None
    return box, count


class Bitmap():
  """Utilities for parsing and inspecting a bitmap."""

  def __init__(self, bpp, width, height, pixels, metadata=None):
    assert bpp in [3, 4], 'Invalid bytes per pixel'
    assert width > 0, 'Invalid width'
    assert height > 0, 'Invalid height'
    assert pixels, 'Must specify pixels'
    assert bpp * width * height == len(pixels), 'Dimensions and pixels mismatch'

    self._bpp = bpp
    self._width = width
    self._height = height
    self._pixels = pixels
    self._metadata = metadata or {}
    self._crop_box = None

  @property
  def bpp(self):
    return self._bpp

  @property
  def width(self):
    return self._crop_box[2] if self._crop_box else self._width

  @property
  def height(self):
    return self._crop_box[3] if self._crop_box else self._height

  def _PrepareTools(self):
    """Prepares an instance of _BitmapTools which allows exactly one command.
    """
    crop_box = self._crop_box or (0, 0, self._width, self._height)
    return _BitmapTools((self._bpp, self._width, self._height) + crop_box,
                        self._pixels)

  @property
  def pixels(self):
    if self._crop_box:
      self._pixels = self._PrepareTools().CropPixels()
      # pylint: disable=unpacking-non-sequence
      _, _, self._width, self._height = self._crop_box
      self._crop_box = None
    if not isinstance(self._pixels, bytearray):
      self._pixels = bytearray(self._pixels)
    return self._pixels

  @property
  def metadata(self):
    self._metadata['size'] = (self.width, self.height)
    self._metadata['alpha'] = self.bpp == 4
    self._metadata['bitdepth'] = 8
    return self._metadata

  def GetPixelColor(self, x, y):
    pixels = self.pixels
    base = self._bpp * (y * self._width + x)
    if self._bpp == 4:
      return rgba_color.RgbaColor(pixels[base + 0], pixels[base + 1],
                                  pixels[base + 2], pixels[base + 3])
    return rgba_color.RgbaColor(pixels[base + 0], pixels[base + 1],
                                pixels[base + 2])

  @staticmethod
  def FromPng(png_data):
    warnings.warn(
        'Using pure python png decoder, which could be very slow. To speed up, '
        'consider installing numpy & cv2 (OpenCV).')
    width, height, pixels, meta = png.Reader(bytes=png_data).read_flat()
    # Some platforms set a transparent background. For consistency, we override
    # transparency to white.
    if meta['alpha']:
      for i in range(3, len(pixels), 4):
        if pixels[i] == 0:
          pixels[i] = 255
          pixels[i - 1] = 255
          pixels[i - 2] = 255
          pixels[i - 3] = 255
    return Bitmap(4 if meta['alpha'] else 3, width, height, pixels, meta)

  @staticmethod
  def FromPngFile(path):
    with open(path, "rb") as f:
      return Bitmap.FromPng(f.read())

  def WritePngFile(self, path):
    with open(path, "wb") as f:
      png.Writer(**self.metadata).write_array(f, self.pixels)

  def IsEqual(self, other, tolerance=0):
    # Dimensions must be equal
    if self.width != other.width or self.height != other.height:
      return False

    # Loop over each pixel and test for equality
    if tolerance or self.bpp != other.bpp:
      for y in range(self.height):
        for x in range(self.width):
          c0 = self.GetPixelColor(x, y)
          c1 = other.GetPixelColor(x, y)
          if not c0.IsEqual(c1, tolerance):
            return False
    else:
      return self.pixels == other.pixels

    return True

  def Diff(self, other):
    # Output dimensions will be the maximum of the two input dimensions
    out_width = max(self.width, other.width)
    out_height = max(self.height, other.height)

    diff = [[0 for x in range(out_width * 3)] for x in range(out_height)]

    # Loop over each pixel and write out the difference
    for y in range(out_height):
      for x in range(out_width):
        if x < self.width and y < self.height:
          c0 = self.GetPixelColor(x, y)
        else:
          c0 = rgba_color.RgbaColor(0, 0, 0, 0)

        if x < other.width and y < other.height:
          c1 = other.GetPixelColor(x, y)
        else:
          c1 = rgba_color.RgbaColor(0, 0, 0, 0)

        offset = x * 3
        diff[y][offset] = abs(c0.r - c1.r)
        diff[y][offset+1] = abs(c0.g - c1.g)
        diff[y][offset+2] = abs(c0.b - c1.b)

    # This particular method can only save to a file, so the result will be
    # written into an in-memory buffer and read back into a Bitmap
    warnings.warn(
        'Using pure python png decoder, which could be very slow. To speed up, '
        'consider installing numpy & cv2 (OpenCV).')
    diff_img = png.from_array(diff, mode='RGB')
    output = StringIO()
    try:
      diff_img.save(output)
      diff = Bitmap.FromPng(output.getvalue())
    finally:
      output.close()

    return diff

  def GetBoundingBox(self, color, tolerance=0):
    return self._PrepareTools().BoundingBox(color, tolerance)

  def Crop(self, left, top, width, height):
    cur_box = self._crop_box or (0, 0, self._width, self._height)
    cur_left, cur_top, cur_width, cur_height = cur_box

    if (left < 0 or top < 0 or
        (left + width) > cur_width or
        (top + height) > cur_height):
      raise ValueError('Invalid dimensions')

    self._crop_box = cur_left + left, cur_top + top, width, height
    return self

  def ColorHistogram(self, ignore_color=None, tolerance=0):
    return self._PrepareTools().Histogram(ignore_color, tolerance)
