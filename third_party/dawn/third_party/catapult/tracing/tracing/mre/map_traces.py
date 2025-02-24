# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import argparse
import sys

from tracing.mre import corpus_driver_cmdline
from tracing.mre import map_runner
from tracing.mre import function_handle
from tracing.mre import job as job_module
from tracing.mre import json_output_formatter


def Main(argv):
  parser = argparse.ArgumentParser(
      description='Bulk trace processing')
  parser.add_argument('--map_function_handle')
  parser.add_argument('-j', '--jobs', type=int,
                      default=map_runner.AUTO_JOB_COUNT)
  parser.add_argument('-o', '--output-file')
  parser.add_argument('-s', '--stop-on-error',
                      action='store_true')

  if len(sys.argv) == 1:
    parser.print_help()
    sys.exit(0)

  args = parser.parse_args(argv[1:])

  corpus_driver = corpus_driver_cmdline.GetCorpusDriver(parser, args)

  if args.output_file:
    ofile = open(args.output_file, 'w')
  else:
    ofile = sys.stdout

  output_formatter = json_output_formatter.JSONOutputFormatter(ofile)

  try:
    map_handle = None
    if args.map_function_handle:
      map_handle = function_handle.FunctionHandle.FromUserFriendlyString(
          args.map_function_handle)
    job = job_module.Job(map_handle)
  except function_handle.UserFriendlyStringInvalidError:
    error_lines = [
        'The map_traces command-line API has changed! You must now specify the',
        'filenames to load and the map function name, separated by :. For '
        'example, a mapper in',
        'foo.html called Foo would be written as foo.html:Foo .'
    ]
    parser.error('\n'.join(error_lines))

  try:
    trace_handles = corpus_driver.GetTraceHandles()
    runner = map_runner.MapRunner(trace_handles, job,
                                  stop_on_error=args.stop_on_error,
                                  jobs=args.jobs,
                                  output_formatters=[output_formatter])
    results = runner.Run()
    if not any(result.failures for result in results):
      return 0
    return 255
  finally:
    if ofile != sys.stdout:
      ofile.close()
