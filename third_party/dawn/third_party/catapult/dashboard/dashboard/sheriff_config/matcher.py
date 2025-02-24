# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import re
import utils


def CompilePattern(pattern):
  if pattern.HasField('glob'):
    return re.compile(utils.Translate(pattern.glob))
  if pattern.HasField('regex'):
    return re.compile(pattern.regex)
  # TODO(dberris): this is the extension point for supporting new
  # matchers; for now we'll skip the new patterns we don't handle yet.
  return None


def CompilePatterns(patterns, ignore_broken=False):
  compiled = []
  for pattern in patterns:
    try:
      c = CompilePattern(pattern)
      if c:
        compiled.append(c)
    except re.error:
      if ignore_broken:
        continue
      raise
  return lambda s: any(c.match(s) for c in compiled)


def CompileRules(rules, ignore_broken=False):
  match = CompilePatterns(rules.match, ignore_broken)
  exclude = CompilePatterns(rules.exclude, ignore_broken)
  return lambda s: match(s) and not exclude(s)
