# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os


def InjectJavaScript(tab, js_file_name):
  with open(os.path.join(os.path.dirname(__file__), js_file_name)) as f:
    js = f.read()
    tab.ExecuteJavaScript(js)
