# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


class RunReporter(object):

  def __init__(self, canonical_url):
    self.canonical_url = canonical_url

  def DidAddFailure(self, failure):
    pass

  def DidRun(self, run_failed):
    pass


# Derived from telemetry ProgressReporter. Should stay close in architecture
# to telemetry ProgressReporter.
class ProgressReporter(object):

  def WillRun(self, canonical_url):
    return RunReporter(canonical_url)

  # TODO(eakuefner): Implement reduction, make this not take a result list.
  def DidFinishAllRuns(self, result_list):
    pass
