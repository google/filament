# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import re


def RunChecks(input_api, output_api, excluded_paths=()):
  excluded_paths = list(excluded_paths) + [r'.*PRESUBMIT\.py']
  root = input_api.change.RepositoryRoot()

  def ShouldCheck(filepath):
    if os.path.split(os.path.dirname(filepath))[1] != 'bin':
      return False
    if any(re.match(pattern, filepath) for pattern in excluded_paths):
      return False
    return True

  results = []

  for f in input_api.AffectedFiles():
    filepath = os.path.join(root, f.LocalPath())
    if (ShouldCheck(filepath) and os.path.exists(filepath)
        and not os.access(filepath, os.X_OK)):
      results += [output_api.PresubmitError(
          '%r must be executable.' % filepath)]

  return results
