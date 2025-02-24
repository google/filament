# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
""" Commonly used utilities """

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import functools
import re
import time


# We need mocking time for testing.
def Time():
  return time.time()


def LRUCacheWithTTL(ttl_seconds=60, **lru_args):

  def Wrapper(func):

    @functools.lru_cache(**lru_args)
    # pylint: disable=unused-argument
    def Cached(ttl, *args, **kargs):
      return func(*args, **kargs)

    def Wrapping(*args, **kargs):
      ttl = kargs.get('_cache_timestamp', int(Time())) // ttl_seconds
      kargs.pop('_cache_timestamp', None)
      return Cached(ttl, *args, **kargs)

    return Wrapping

  return Wrapper


# Modified from fnmatch.translate because it add \Z to the end of the
# translated regexp which re2 doesn't support. The main differences are:
# 1. * in glob won't match /, that should be unsuperising for most of users.
# 2. Support ** for recursively matching. It's same to .* in regexp.
# 3. Generate $ at the end of regexp instead of \Z so re2 can use the regular
# expression.
def Translate(pat):
  """
  Translate a shell PATTERN to a regular expression. There is no way to
  quote meta-characters. Note: * in glob won't match /.
  """

  i, n = 0, len(pat)
  res = ''
  while i < n:
    c = pat[i]
    i = i + 1
    if c == '*':
      if i < n and pat[i] == '*':
        res = res + '.*'
        i = i + 1
      else:
        res = res + '[^/]*'
    elif c == '?':
      res = res + '[^/]'
    elif c == '[':
      j = i
      if j < n and pat[j] == '!':
        j = j + 1
      if j < n and pat[j] == ']':
        j = j + 1
      while j < n and pat[j] != ']':
        j = j + 1
      if j >= n:
        res = res + '\\['
      else:
        stuff = pat[i:j].replace('\\', '\\\\')
        i = j + 1
        if stuff[0] == '!':
          stuff = '^' + stuff[1:]
        elif stuff[0] == '^':
          stuff = '\\' + stuff
        res = '%s[%s]' % (res, stuff)
    else:
      res = res + re.escape(c)
  return res + '$'
