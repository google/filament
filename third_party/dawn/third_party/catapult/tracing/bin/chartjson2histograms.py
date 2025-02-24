#!/usr/bin/env python
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import os
import sys

from tracing.value import convert_chart_json  # pylint: disable=wrong-import-position
import tracing_project  # pylint: disable=wrong-import-position

TRACING_PATH = os.path.abspath(
    os.path.join(os.path.dirname(os.path.realpath(__file__)), '..'))
sys.path.append(TRACING_PATH)
tracing_project.UpdateSysPathIfNeeded()



def main():
  parser = argparse.ArgumentParser(
      description='Converts a chartjson file to HistogramSet JSON.',
      add_help=False)
  parser.add_argument('chartjson_path',
                      help='chartjson file path (input).')
  parser.add_argument('histograms_path',
                      help='HistogramSet JSON file path (output).')
  parser.add_argument('-h', '--help', action='help',
                      help='Show this help message and exit.')
  args = parser.parse_args()
  result = convert_chart_json.ConvertChartJson(args.chartjson_path)
  if result.returncode != 0:
    sys.stderr.write(result.stdout)
  else:
    open(args.histograms_path, 'w').write(result.stdout)
  return result.returncode

if __name__ == '__main__':
  sys.exit(main())
