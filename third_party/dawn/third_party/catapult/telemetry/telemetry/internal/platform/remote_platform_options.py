# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


class RemotePlatformOptions():
  """Options to be used for creating a remote platform instance."""


class AndroidPlatformOptions(RemotePlatformOptions):
  """Android-specific remote platform options."""

  def __init__(self, device=None, android_denylist_file=None):
    super().__init__()

    self.device = device
    self.android_denylist_file = android_denylist_file
