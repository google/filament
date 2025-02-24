# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry.command_line.parser import ParseArgs
from telemetry.command_line.parser import RunCommand
from telemetry.command_line.parser import main

# Expose the list of output formats still supported by Telemetry.
from telemetry.internal.results.results_options import LEGACY_OUTPUT_FORMATS
