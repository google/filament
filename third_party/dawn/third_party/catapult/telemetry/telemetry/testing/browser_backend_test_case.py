# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry.testing import browser_test_case


class BrowserBackendTestCase(browser_test_case.BrowserTestCase):
  @classmethod
  def setUpClass(cls):
    super(BrowserBackendTestCase, cls).setUpClass()
    cls._browser_backend = cls._browser._browser_backend
