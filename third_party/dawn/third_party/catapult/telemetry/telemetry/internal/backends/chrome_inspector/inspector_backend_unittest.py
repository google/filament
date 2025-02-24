# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry import decorators
from telemetry.core import exceptions
from telemetry.testing import browser_test_case


class InspectorBackendTest(browser_test_case.BrowserTestCase):
  @property
  def _devtools_client(self):
    return self._browser._browser_backend.devtools_client

  # https://github.com/catapult-project/catapult/issues/3099 (Android)
  # crbug.com/483212 (CrOS)
  @decorators.Disabled('android', 'chromeos')
  def testWaitForJavaScriptCondition(self):
    context_map = self._devtools_client.GetUpdatedInspectableContexts()
    backend = context_map.GetInspectorBackend(context_map.contexts[0]['id'])
    backend.WaitForJavaScriptCondition('true')

  # https://github.com/catapult-project/catapult/issues/3099 (Android)
  # crbug.com/483212 (CrOS)
  @decorators.Disabled('android', 'chromeos')
  def testWaitForJavaScriptConditionPropagatesEvaluateException(self):
    context_map = self._devtools_client.GetUpdatedInspectableContexts()
    backend = context_map.GetInspectorBackend(context_map.contexts[0]['id'])
    with self.assertRaises(exceptions.EvaluateException):
      backend.WaitForJavaScriptCondition('syntax error!')
