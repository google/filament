# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Python port of legacy_unit_info.html. This is a mapping of various units
# reported by gtest perf tests to histogram units, including improvement
# direction and conversion factors.
# Note that some of the converted names don't match up exactly with the ones
# in the HTML file, as unit names are sometimes different between the two
# implementations. For example, timeDurationInMs in the JavaScript
# implementation is ms in the Python implementation.

IMPROVEMENT_DIRECTION_DONT_CARE = ''
IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER = '_smallerIsBetter'
IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER = '_biggerIsBetter'


class LegacyUnit(object):
  """Simple object for storing data to improve readability."""
  def __init__(self, name, improvement_direction, conversion_factor=1):
    assert improvement_direction in [
        IMPROVEMENT_DIRECTION_DONT_CARE,
        IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER,
        IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER]
    self._name = name
    self._improvement_direction = improvement_direction
    self._conversion_factor = conversion_factor

  @property
  def name(self):
    return self._name + self._improvement_direction

  @property
  def conversion_factor(self):
    return self._conversion_factor

  def AsTuple(self):
    return self.name, self.conversion_factor


LEGACY_UNIT_INFO = {
    '%': LegacyUnit('n%', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER),
    '': LegacyUnit('unitless', IMPROVEMENT_DIRECTION_DONT_CARE),
    'Celsius': LegacyUnit('unitless', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER),
    'Hz': LegacyUnit('Hz', IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'KB': LegacyUnit('sizeInBytes', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER,
                     conversion_factor=1024),
    'MB': LegacyUnit('sizeInBytes', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER,
                     conversion_factor=(1024 * 1024)),
    'ObjectsAt30FPS': LegacyUnit('unitless',
                                 IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'available_kB': LegacyUnit('sizeInBytes',
                               IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER,
                               conversion_factor=1024),
    'bit/s': LegacyUnit('unitless', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER),
    'bytes': LegacyUnit('sizeInBytes', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER),
    'chars/s': LegacyUnit('unitless', IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'commit_count': LegacyUnit('count', IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'count': LegacyUnit('count', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER),
    'coverage%': LegacyUnit('n%', IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'dB': LegacyUnit('unitless', IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'files': LegacyUnit('count', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER),
    'fps': LegacyUnit('unitless', IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'frame_count': LegacyUnit('count', IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'frame_time': LegacyUnit('ms', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER),
    'frames': LegacyUnit('count', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER),
    'garbage_collections': LegacyUnit('count',
                                      IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER),
    'idle%': LegacyUnit('n%', IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'janks': LegacyUnit('count', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER),
    'lines': LegacyUnit('count', IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'mWh': LegacyUnit('J', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER,
                      conversion_factor=3.6),
    'milliseconds': LegacyUnit('ms', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER),
    'milliseconds-per-frame': LegacyUnit(
        'ms', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER),
    'minutes': LegacyUnit('msBestFitFormat',
                          IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER,
                          conversion_factor=60000),
    'mips': LegacyUnit('unitless', IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'mpixels_sec': LegacyUnit('unitless',
                              IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'ms': LegacyUnit('ms', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER),
    'mtri_sec': LegacyUnit('unitless', IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'mvtx_sec': LegacyUnit('unitless', IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'objects (bigger is better)': LegacyUnit(
        'count', IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'packets': LegacyUnit('count', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER),
    'percent': LegacyUnit('n%', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER),
    'points': LegacyUnit('unitless', IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'ports': LegacyUnit('count', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER),
    'reduction%': LegacyUnit('n%', IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'relocs': LegacyUnit('count', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER),
    'runs/s': LegacyUnit('unitless', IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'score (bigger is better)': LegacyUnit(
        'unitless', IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'seconds': LegacyUnit('ms', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER,
                          conversion_factor=1000),
    'tokens/s': LegacyUnit('unitless', IMPROVEMENT_DIRECTION_BIGGER_IS_BETTER),
    'tasks': LegacyUnit('unitless', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER),
    'us': LegacyUnit('msBestFitFormat', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER,
                     conversion_factor=0.001),
    'ns': LegacyUnit('msBestFitFormat', IMPROVEMENT_DIRECTION_SMALLER_IS_BETTER,
                     conversion_factor=0.000001),
}

# Add duplicate units here
LEGACY_UNIT_INFO['frames-per-second'] = LEGACY_UNIT_INFO['fps']
LEGACY_UNIT_INFO['kb'] = LEGACY_UNIT_INFO['KB']
LEGACY_UNIT_INFO['ms'] = LEGACY_UNIT_INFO['milliseconds']
LEGACY_UNIT_INFO['runs_per_s'] = LEGACY_UNIT_INFO['runs/s']
LEGACY_UNIT_INFO['runs_per_second'] = LEGACY_UNIT_INFO['runs/s']
LEGACY_UNIT_INFO['score'] = LEGACY_UNIT_INFO['score (bigger is better)']
LEGACY_UNIT_INFO['score_(bigger_is_better)'] = LEGACY_UNIT_INFO['score']
