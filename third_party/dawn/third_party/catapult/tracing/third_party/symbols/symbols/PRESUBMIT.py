# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


USE_PYTHON3 = True


def CommonChecks(input_api, output_api):
  output = []
  output.extend(input_api.RunTests(input_api.canned_checks.GetPylint(
      input_api,
      output_api,
      version='2.7')))
  output.extend(input_api.canned_checks.RunUnitTestsInDirectory(
      input_api,
      output_api,
      input_api.PresubmitLocalPath(),
      files_to_check=[r'^.+_unittest\.py$'],
      run_on_python2=False,
      run_on_python3=True,
      skip_shebang_check=True))
  return output


def CheckChangeOnUpload(input_api, output_api):
  return CommonChecks(input_api, output_api)


def CheckChangeOnCommit(input_api, output_api):
  return CommonChecks(input_api, output_api)