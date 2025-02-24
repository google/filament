# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import abc
import six


class FrameReadError(Exception):
  pass


class FrameGenerator(six.with_metaclass(abc.ABCMeta, object)):
  """ Defines an interface for reading input frames.

  Attributes:
    _generator: A reference to the created generator.
  """

  def __init__(self):
    """ Initializes the FrameGenerator object. """
    self._generator = self._CreateGenerator()

  @abc.abstractmethod
  def _CreateGenerator(self):
    """ Creates a new generator.

    Implemented in derived classes.

    Raises:
      FrameReadError: A error occurred in reading the frame.
    """
    raise NotImplementedError

  @property
  def Generator(self):
    """ Returns:
          A reference to the created generator.
    """
    return self._generator

  @abc.abstractproperty
  def CurrentTimestamp(self):
    """ Returns:
          float, The timestamp of the current frame in milliseconds.
    """
    raise NotImplementedError

  @abc.abstractproperty
  def CurrentFrameNumber(self):
    """ Returns:
          int, The frame index of the current frame.
    """
    raise NotImplementedError

  @abc.abstractproperty
  def Dimensions(self):
    """ Returns:
          The dimensions of the frame sequence as a tuple int (width, height).
          This value should be constant across frames.
    """
    raise NotImplementedError
