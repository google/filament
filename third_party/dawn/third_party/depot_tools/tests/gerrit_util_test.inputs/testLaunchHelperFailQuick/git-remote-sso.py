#!/usr/bin/env python3
# Copyright 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

print("SSO Failure Message!!!", file=sys.stderr)
os.close(1)  # signal that we've written all config
