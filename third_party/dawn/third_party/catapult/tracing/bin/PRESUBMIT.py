# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


USE_PYTHON3 = True


import os

def CheckChange(input_api, output_api):
  init_py_path = os.path.join(input_api.PresubmitLocalPath(), '__init__.py')
  res = []
  if os.path.exists(init_py_path):
    res += [output_api.PresubmitError(
        '__init__.py is not allowed to exist in bin/')]
  return res

def CheckChangeOnUpload(input_api, output_api):
  return CheckChange(input_api, output_api)


def CheckChangeOnCommit(input_api, output_api):
  return CheckChange(input_api, output_api)
