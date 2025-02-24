# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import time

from profile_chrome import chrome_startup_tracing_agent
from profile_chrome import chrome_tracing_agent
from profile_chrome import ui
from profile_chrome import util
from systrace import output_generator
from systrace import tracing_controller


def _GetResults(trace_results, controller, output, compress,
                interval, trace_format='html'):
  ui.PrintMessage('Downloading...')

  # Wait for the trace file to get written.
  time.sleep(1)

  for agent in controller.get_child_agents:
    if isinstance(agent, chrome_tracing_agent.ChromeTracingAgent):
      time.sleep(interval / 4)

  # Ignore the systraceController because it will not contain any results,
  # instead being in charge of collecting results.
  trace_results = [x for x in controller.all_results if not (x.source_name ==
      'systraceController')]

  if not trace_results:
    ui.PrintMessage('No results')
    return ''

  result = None
  trace_results = output_generator.MergeTraceResultsIfNeeded(trace_results)
  if trace_format == 'html':
    ui.PrintMessage('Writing trace HTML...')
    html_file = output or trace_results[0].source_name + '.html'
    result = output_generator.GenerateHTMLOutput(trace_results, html_file)
    ui.PrintMessage('\nWrote file://%s' % result)
  elif compress and len(trace_results) == 1:
    result = output or trace_results[0].source_name + '.gz'
    util.WriteDataToCompressedFile(trace_results[0].raw_data, result)
  elif len(trace_results) > 1:
    result = (output or 'chrome-combined-trace-%s.zip' %
              util.GetTraceTimestamp())
    util.ArchiveData(trace_results, result)
  else:
    if output:
      result = output
    else:
      result = trace_results[0].source_name
    mode = 'wb' if isinstance(trace_results[0].raw_data, bytes) else 'w'
    with open(result, mode) as f:
      f.write(trace_results[0].raw_data)

  return result


def CaptureProfile(options, interval, modules, output=None,
                   compress=False, trace_format='html'):
  """Records a profiling trace saves the result to a file.

  Args:
    options: Command line options.
    interval: Time interval to capture in seconds. An interval of None (or 0)
        continues tracing until stopped by the user.
    modules: The list of modules to initialize the tracing controller with.
    output: Output file name or None to use an automatically generated name.
    compress: If True, the result will be compressed either with gzip or zip
        depending on the number of captured subtraces.
    trace_format: Format of trace file html|json|proto. Default is html for
        legacy reasons.

  Returns:
    Path to saved profile.
  """
  agents_with_config = tracing_controller.CreateAgentsWithConfig(options,
                                                                 modules)
  if chrome_startup_tracing_agent in modules:
    controller_config = tracing_controller.GetChromeStartupControllerConfig(
        options)
  else:
    controller_config = tracing_controller.GetControllerConfig(options)
  controller = tracing_controller.TracingController(agents_with_config,
                                                    controller_config)
  try:
    result = controller.StartTracing()
    trace_type = controller.GetTraceType()
    if not result:
      ui.PrintMessage('Trace starting failed.')
    if interval:
      ui.PrintMessage(('Capturing %d-second %s. Press Enter to stop early...' %
                     (interval, trace_type)), eol='')
      ui.WaitForEnter(interval)
    else:
      ui.PrintMessage('Capturing %s. Press Enter to stop...' % trace_type,
                      eol='')
      # TODO(https://crbug.com/1262296): Update this after Python2 trybots retire.
      # pylint: disable=undefined-variable
      raw_input()

    ui.PrintMessage('Stopping...')
    all_results = controller.StopTracing()
  finally:
    if interval:
      ui.PrintMessage('done')

  return _GetResults(all_results, controller, output, compress,
                     interval, trace_format)
