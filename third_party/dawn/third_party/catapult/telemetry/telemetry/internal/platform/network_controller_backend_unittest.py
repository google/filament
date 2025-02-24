# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest
from unittest import mock

from telemetry.testing import fakes
from telemetry.core import exceptions
from telemetry.internal.platform import network_controller_backend
from telemetry.internal.platform import platform_backend
from telemetry.util import wpr_modes


class NetworkControllerBackendTest(unittest.TestCase):
  def testReOpenSuccessAfterFirstFailure(self):
    backend = platform_backend.PlatformBackend()
    fake_forwarder_factory = fakes.FakeForwarderFactory()
    fake_forwarder_factory.raise_exception_on_create = True
    with mock.patch(
        'telemetry.internal.platform.platform_backend.'
        'PlatformBackend.forwarder_factory', new=fake_forwarder_factory):
      nb = network_controller_backend.NetworkControllerBackend(backend)
      try:
        try:
          # First time initializing network_controller_backend would fail.
          nb.Open(wpr_modes.WPR_REPLAY)
        except exceptions.IntentionalException:
          pass
        fake_forwarder_factory.raise_exception_on_create = False
        # Second time initializing network_controller_backend would succeed
        nb.Open(wpr_modes.WPR_REPLAY)
      finally:
        nb.Close()
