# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Lint as: python3
"""Common exit codes for catapult tools."""

SUCCESS = 0
TEST_FAILURE = 1
FATAL_ERROR = 2
# See crbug.com/1019139#c8 for history on this exit code.
# Note that some test runners (for example: typ) may continue to return SUCCESS
# for cases where all tests are skipped.
ALL_TESTS_SKIPPED = 111
