# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re


class AtosRegexMatcher(object):
  def __init__(self):
    """
    Atos output has two useful forms:
      1. <name> (in <library name>) (<filename>:<linenumber>)
      2. <name> (in <library name>) + <symbol offset>
    And two less useful forms:
      3. <address>
      4. <address> (in <library name>)
    e.g.
      1. -[SKTGraphicView drawRect:] (in Sketch) (SKTGraphicView.m:445)
      2. malloc (in libsystem_malloc.dylib) + 42
      3. 0x4a12
      4. 0x00000d9a (in Chromium)

    We don't bother checking for the latter two, and just return the full
    output.
    """
    self._regex1 = re.compile(r"(.*) \(in (.+)\) \((.+):(\d+)\)")
    self._regex2 = re.compile(r"(.*) \(in (.+)\) \+ (\d+)")

  def Match(self, text):
    result = self._regex1.match(text)
    if result:
      return result.group(1)
    result = self._regex2.match(text)
    if result:
      return result.group(1)
    return text
