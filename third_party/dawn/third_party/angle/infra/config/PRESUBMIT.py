# Copyright 2019 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

USE_PYTHON3 = True


def CheckChangeOnUpload(input_api, output_api):
    return input_api.RunTests(
        input_api.canned_checks.CheckLucicfgGenOutput(input_api, output_api, 'main.star'))


def CheckChangeOnCommit(input_api, output_api):
    return CheckChangeOnUpload(input_api, output_api)
