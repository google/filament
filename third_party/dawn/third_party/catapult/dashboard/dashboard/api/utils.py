# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime


def ParseBool(value):
  """Parse a string representation into a True/False value."""
  if value is None:
    return None
  value_lower = value.lower()
  if value_lower in ('true', '1'):
    return True
  if value_lower in ('false', '0'):
    return False
  raise ValueError(value)


def ParseISO8601(s):
  if s is None:
    return None
  # ISO8601 specifies many possible formats. The dateutil library is much more
  # flexible about parsing all of the possible formats, but it would be annoying
  # to third_party it just for this. A few formats should cover enough users.
  try:
    return datetime.datetime.strptime(s, '%Y-%m-%dT%H:%M:%S.%f')
  except ValueError:
    return datetime.datetime.strptime(s, '%Y-%m-%dT%H:%M:%S')
