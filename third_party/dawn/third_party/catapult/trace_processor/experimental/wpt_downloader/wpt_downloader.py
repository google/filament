#!/usr/bin/python
# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import json
import os
import urllib
import urlparse
import zlib


WPT_TEST_URL = 'http://www.webpagetest.org/jsonResult.php?test={wpt_job}'
ERROR_MISSING_WPT_JOBS = """WPT Job Ids not specified!

Use --wpt_jobs to specify a list of comma separated ids.

ie. python download_from_wpt.py --wpt_jobs 1,2,3 output_dir/
"""

def WriteMetadataAndTraceToFile(
    output_path, file_name, metadata, trace_contents):
  file_name = os.path.join(output_path, file_name)

  with open(os.path.join(output_path, file_name), 'wb') as f:
    f.write(trace_contents)

  with open(os.path.join(output_path, '%s.meta' % file_name), 'w') as f:
    json.dump(metadata, f)


def DownloadFromWPT(wpt_job, output_path):
  url = WPT_TEST_URL.format(wpt_job=wpt_job)
  job_response = urllib.urlopen(url)
  job_data = json.load(job_response)

  blacklist = ['runs', 'median', 'average', 'standardDeviation']
  metadata = dict(
      [(k, v) for k, v in job_data['data'].iteritems() if not k in blacklist])

  for k,v in job_data['data']['runs'].iteritems():
    for a,b in v.iteritems():
      if not 'trace' in b['rawData']:
        continue
      trace_url = b['rawData']['trace']
      parsed_url = urlparse.urlparse(trace_url)
      query = urlparse.parse_qsl(parsed_url.query)
      file_name = '%s_%s' % (wpt_job, query[2][1])
      print 'Downloading %s to %s ...' % (trace_url, file_name)

      file_response = urllib.urlopen(trace_url)

      WriteMetadataAndTraceToFile(
          output_path, file_name, metadata, file_response.read())


def Main():
  parser = argparse.ArgumentParser(description='Process traces')
  parser.add_argument('output_path', help='Output path')
  parser.add_argument('--wpt_jobs', help='WebPageTest Job IDs, comma separated')
  args = parser.parse_args()

  output_path = os.path.abspath(args.output_path)

  if not args.wpt_jobs:
    parser.exit(1, ERROR_MISSING_WPT_JOBS)

  wpt_jobs =  args.wpt_jobs.split(',')

  for wpt_job in wpt_jobs:
    DownloadFromWPT(wpt_job, output_path)
