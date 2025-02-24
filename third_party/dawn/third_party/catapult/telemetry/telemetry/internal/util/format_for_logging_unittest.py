# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest

from telemetry.internal.util import format_for_logging


class FormatForLoggingTest(unittest.TestCase):

  def testTrim_NoMatches(self):
    COMMAND = ['./chrome', '--blah', '--blahblah']
    command = list(COMMAND)
    format_for_logging._Trim(command)
    self.assertEqual(command, COMMAND)

  def testTrim_DisableFeatures(self):
    COMMAND = ['./chrome', '--force-fieldtrials=FeatureThatIsVerbose',
               '--blahblah']
    command = list(COMMAND)
    format_for_logging._Trim(command)
    self.assertEqual(command[0], COMMAND[0])
    self.assertEqual(command[1], '--force-fieldtrials=...')
    self.assertEqual(command[2], COMMAND[2])

  def testShellFormat_NoTrim_SmokeTest(self):
    COMMAND = ['./chrome', '--force-fieldtrials=FeatureThatIsVerbose',
               '--blahblah']
    command = list(COMMAND)
    formatted_command = format_for_logging.ShellFormat(command)
    self.assertEqual(command, COMMAND, 'logging command should not edit input')
    self.assertIn('chrome', formatted_command)
    self.assertIn('blahblah', formatted_command)
    self.assertIn('--force-fieldtrials=FeatureThatIsVerbose', formatted_command)

  def testShellFormat_Trim_UsesCopyAndSmokeTest(self):
    COMMAND = ['./chrome', '--force-fieldtrials=FeatureThatIsVerbose',
               '--blahblah']
    command = list(COMMAND)
    formatted_command = format_for_logging.ShellFormat(command, trim=True)
    self.assertEqual(command, COMMAND, 'logging command should not edit input')
    self.assertIn('chrome', formatted_command)
    self.assertIn('blahblah', formatted_command)
