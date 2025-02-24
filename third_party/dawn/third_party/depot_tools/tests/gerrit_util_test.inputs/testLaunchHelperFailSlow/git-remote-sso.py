#!/usr/bin/env python3
# Copyright 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
import time

time.sleep(6)
print(f"I ran too long without writing output!!", file=sys.stderr)
