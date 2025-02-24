# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Allows multiple duplicate interactions of the same type
REPEATABLE = 'repeatable'

FLAGS = [REPEATABLE]


def _AssertFlagsAreValid(flags):
  assert isinstance(flags, list)
  for f in flags:
    if f not in FLAGS:
      raise AssertionError(
          'Unrecognized flag for a timeline interaction record: %s' % f)


def GetJavaScriptMarker(label, flags):
  """Computes the marker string of an interaction record.

  This marker string can be used with JavaScript API console.time()
  and console.timeEnd() to mark the beginning and end of the
  interaction record..

  Args:
    label: The label used to identify the interaction record.
    flags: the flags for the interaction record see FLAGS above.

  Returns:
    The interaction record marker string (e.g., Interaction.Label/flag1,flag2).

  Raises:
    AssertionError: If one or more of the flags is unrecognized.
  """
  _AssertFlagsAreValid(flags)
  marker = 'Interaction.%s' % label
  if flags:
    marker += '/%s' % (','.join(flags))
  return marker
