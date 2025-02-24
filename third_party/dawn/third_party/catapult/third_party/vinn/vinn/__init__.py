#!/usr/bin/env python
# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from ._vinn import ExecuteFile
from ._vinn import ExecuteJsString
from ._vinn import RunFile
from ._vinn import RunJsString

__all__ = [
  'ExecuteFile',
  'ExecuteJsString',
  'RunFile',
  'RunJsString'
]
