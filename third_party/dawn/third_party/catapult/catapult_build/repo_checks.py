# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Checks to use in PRESUBMIT.py for general repository violations."""


def RunChecks(input_api, output_api):
  orig_files = [f.LocalPath()
                for f in input_api.AffectedFiles(include_deletes=False)
                if f.LocalPath().endswith('.orig')]
  if orig_files:
    return [output_api.PresubmitError(
        'Files with ".orig" suffix must not be checked into the '
        'repository:\n  ' + '\n  '.join(orig_files))]
  return []
