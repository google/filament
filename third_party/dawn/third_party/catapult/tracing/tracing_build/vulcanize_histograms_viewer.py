# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import tracing_project
from py_vulcanize import generate

from tracing_build import render_histograms_viewer


def VulcanizeHistogramsViewer():
  """Vulcanizes Histograms viewer with its dependencies.

  Args:
    path: destination to write the vulcanized viewer HTML.
  """
  vulcanizer = tracing_project.TracingProject().CreateVulcanizer()
  load_sequence = vulcanizer.CalcLoadSequenceForModuleNames(
      ['tracing_build.histograms_viewer'])
  return generate.GenerateStandaloneHTMLAsString(load_sequence)


def VulcanizeAndRenderHistogramsViewer(
    histogram_dicts, output_stream, reset_results=False):
  render_histograms_viewer.RenderHistogramsViewer(
      histogram_dicts, output_stream, reset_results,
      VulcanizeHistogramsViewer())
