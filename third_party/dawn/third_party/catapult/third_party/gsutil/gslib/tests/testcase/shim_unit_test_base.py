# -*- coding: utf-8 -*-
# Copyright 2023 Google LLC. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Contains base class to be used for shim unit tests."""

import subprocess
from unittest import mock

from gslib.tests.testcase.unit_testcase import GsUtilUnitTestCase


class ShimUnitTestBase(GsUtilUnitTestCase):
  """Base class for unit testing shim behavior.
  
  This class mocks the `subprocess.run()` method because it gets called
  for all shim operations to check if there is an active account configured
  for gcloud.
  """

  def setUp(self):
    super().setUp()
    # Translator calls `gcloud config get account` to check active account
    # using subprocess.run().
    # We don't care about this call for most of the tests below, so we are
    # simply patching this here.
    # There are separate tests to check this call is being made.
    self._subprocess_run_patcher = mock.patch.object(subprocess,
                                                     'run',
                                                     autospec=True)
    self._mock_subprocess_run = self._subprocess_run_patcher.start()
    self._mock_subprocess_run.return_value.returncode = 0

  def tearDown(self):
    if self._subprocess_run_patcher is not None:
      self._subprocess_run_patcher.stop()
    super().tearDown()
