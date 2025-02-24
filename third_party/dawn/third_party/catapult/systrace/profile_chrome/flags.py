# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# TODO(https://crbug.com/1262296): Update this after Python2 trybots retire.
# pylint: disable=deprecated-module
import optparse


def OutputOptions(parser):
  # TODO(https://crbug.com/1262296): Update this after Python2 trybots retire.
  # pylint: disable=deprecated-module
  output_options = optparse.OptionGroup(parser, 'Output options')
  output_options.add_option('-o', '--output', dest='output_file',
                            help='Save trace output to file.')
  output_options.add_option('--json', help='Save trace as raw JSON instead of '
                            'HTML.', dest='write_json')
  output_options.add_option('--trace_format', help='Save trace as chosen '
                              'format instead of the default HTML.',
                              dest='trace_format')
  output_options.add_option('--view', help='Open resulting trace file in a '
                            'browser.', action='store_true')
  return output_options

def ParseFormatFlags(options):
  """Parses trace format through trace_format and write_json flags.

  This function ensures that the user sets valid flag values and updates
  the value of trace_format if necessary. For backwards compatibility,
  we maintain the write_json flag. However, we also want to ensure that
  we modify the successor trace_format flag accordingly. At most one of
  the trace_format and write_json flags should be specified.

  Args:
    options: An object with specfied command line flag value attributes

  Raises:
    ValueError: An error due to both the trace_format and write_json
      flags being specified
  """
  # Assume that at most one of options.write_json or options.trace_format
  # have defaulted values (no flag input).
  supported_flags = ['html', 'json', 'proto']
  if options.write_json and options.trace_format:
    raise ValueError("At most one parameter of --json or " +
                      "--trace_format should be provided")
  if not options.write_json and not options.trace_format:
    print("Using default format: html. Choose another format by specifying: " +
            "--trace_format [html|json|proto]")
  if options.trace_format:
    if options.trace_format not in supported_flags:
      raise ValueError("Format '{}' is not supported." \
                            .format(options.trace_format))
  else:
    options.trace_format = 'json' if options.write_json else 'html'
