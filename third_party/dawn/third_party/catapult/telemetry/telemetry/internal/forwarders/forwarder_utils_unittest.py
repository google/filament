# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import tempfile
import unittest

from telemetry import decorators
from telemetry.internal.forwarders import forwarder_utils


class ReadRemotePortTests(unittest.TestCase):
  @decorators.Disabled('win')  # https://crbug.com/793256
  def testReadRemotePort(self):
    sample_output = [
        b'', b'', b'Allocated port 42360 for remote forward to localhost:12345']

    with tempfile.NamedTemporaryFile() as cros_stderr:
      for line in sample_output:
        cros_stderr.write(line + b'\n')
      cros_stderr.flush()
      remote_port = forwarder_utils.ReadRemotePort(cros_stderr.name)

    self.assertEqual(remote_port, 42360)
