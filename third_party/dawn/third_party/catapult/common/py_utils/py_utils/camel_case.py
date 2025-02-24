# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
import re
import six


def ToUnderscore(obj):
  """Converts a string, list, or dict from camelCase to lower_with_underscores.

  Descends recursively into lists and dicts, converting all dict keys.
  Returns a newly allocated object of the same structure as the input.
  """
  if isinstance(obj, six.string_types):
    return re.sub('(?!^)([A-Z]+)', r'_\1', obj).lower()

  if isinstance(obj, list):
    return [ToUnderscore(item) for item in obj]

  if isinstance(obj, dict):
    output = {}
    for k, v in six.iteritems(obj):
      if isinstance(v, (list, dict)):
        output[ToUnderscore(k)] = ToUnderscore(v)
      else:
        output[ToUnderscore(k)] = v
    return output

  return obj
