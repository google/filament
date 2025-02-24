# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import os
import tempfile
import unittest

from tracing.value import histogram
from tracing.value import histogram_set
from tracing.value import merge_histograms


class MergeHistogramsUnittest(unittest.TestCase):

  def testSingularHistogramsGetMergedFrom(self):
    hist0 = histogram.Histogram('foo', 'count')
    hist1 = histogram.Histogram('bar', 'count')
    histograms = histogram_set.HistogramSet([hist0, hist1])
    histograms_file = tempfile.NamedTemporaryFile(delete=False)
    histograms_file.write(json.dumps(histograms.AsDicts()).encode('utf-8'))
    histograms_file.close()

    merged_dicts = merge_histograms.MergeHistograms(histograms_file.name,
                                                    ('name',))
    merged_histograms = histogram_set.HistogramSet()
    merged_histograms.ImportDicts(merged_dicts)
    self.assertEqual(len(list(merged_histograms.shared_diagnostics)), 0)
    self.assertEqual(len(merged_histograms), 2)
    os.remove(histograms_file.name)
