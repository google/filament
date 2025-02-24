# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import json

from tracing.mre import output_formatter


class JSONOutputFormatter(output_formatter.OutputFormatter):

  def __init__(self, output_file):
    # TODO(nduca): Resolve output_file here vs output_stream in base class.
    super(JSONOutputFormatter, self).__init__(output_file)
    self.output_file = output_file

  def Format(self, results):
    d = [result.AsDict() for result in results]
    json.dump(d, self.output_file, indent=2)
    if hasattr(self.output_file, 'flush'):
      self.output_file.flush()
