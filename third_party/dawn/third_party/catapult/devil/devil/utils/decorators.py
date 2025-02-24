# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import functools


def Memoize(f):
  """Decorator to cache return values of function."""
  memoize_dict = {}
  @functools.wraps(f)
  def wrapper(*args, **kwargs):
    key = repr((args, kwargs))
    if key not in memoize_dict:
      memoize_dict[key] = f(*args, **kwargs)
    return memoize_dict[key]
  return wrapper
