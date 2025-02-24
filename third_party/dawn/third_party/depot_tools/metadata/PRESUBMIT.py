#!/usr/bin/env vpython3
# Copyright (c) 2023 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PRESUBMIT_VERSION = '2.0.0'


def CheckPythonUnitTests(input_api, output_api):
    tests = input_api.canned_checks.GetUnitTestsInDirectory(
        input_api, output_api, "tests", files_to_check=[r'.+_test\.py$'])

    return input_api.RunTests(tests)
