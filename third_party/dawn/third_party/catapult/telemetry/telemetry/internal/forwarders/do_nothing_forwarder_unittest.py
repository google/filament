# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest

from telemetry.internal.forwarders import do_nothing_forwarder


DoNothingForwarder = do_nothing_forwarder.DoNothingForwarder


class CheckPortPairsTest(unittest.TestCase):
  def testBasicCheck(self):
    f = DoNothingForwarder(local_port=80, remote_port=80)
    self.assertEqual(f.local_port, f.remote_port)

  def testDefaultLocalPort(self):
    f = DoNothingForwarder(local_port=None, remote_port=80)
    self.assertEqual(f.local_port, f.remote_port)

  def testDefaultRemotePort(self):
    f = DoNothingForwarder(local_port=42, remote_port=0)
    self.assertEqual(f.local_port, 42)
    self.assertEqual(f.remote_port, 42)

  def testMissingPortsRaisesError(self):
    # At least one of the two ports must be given
    with self.assertRaises(AssertionError):
      DoNothingForwarder(local_port=None, remote_port=None)

  def testPortMismatchRaisesPortsMismatchError(self):
    # The do_nothing_forward cannot forward from one port to another.
    with self.assertRaises(do_nothing_forwarder.PortsMismatchError):
      DoNothingForwarder(local_port=80, remote_port=81)
