# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Derived from telemetry OutputFormatter. Should stay close in architecture
# to telemetry OutputFormatter.


class OutputFormatter(object):

  def __init__(self, output_stream):
    self._output_stream = output_stream

  def Format(self, results):
    raise NotImplementedError()

  @property
  def output_stream(self):
    return self._output_stream
