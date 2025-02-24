# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Acceptable values for the Telemetry compatibility_mode flag.
"""

NO_FIELD_TRIALS = 'no-field-trials'
IGNORE_CERTIFICATE_ERROR = 'ignore-certificate-errors'
LEGACY_COMMAND_LINE_PATH = 'legacy-command-line-path'
GPU_BENCHMARKING_FALLBACKS = 'gpu-benchmarking-fallbacks'
# On Android, don't require a rooted device, and don't attempt to run adb as
# root. This restricts the kinds of operations that can be performed, for
# example setting the CPU governor, and is not suitable for performance testing.
DONT_REQUIRE_ROOTED_DEVICE = 'dont-require-rooted-device'
