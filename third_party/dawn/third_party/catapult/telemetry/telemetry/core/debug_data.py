# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

class DebugData():
  """A class for storing debug data in string format for later use.

  This is largely so that telemetry.core.exceptions' AppCrashException can
  share code with telemetry.internal.backends.browser_backends'
  CollectDebugData.
  """

  def __init__(self):
    super().__init__()

    # List of strings, each element being a human-readable symbolized minidump.
    self.symbolized_minidumps = []
    self.stdout = ''
    self.system_log = ''
