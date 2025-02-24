# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import json
import os
import re
import sys
import tempfile
import platform

import tracing_project
import vinn

from tracing.mre import failure
from tracing.mre import file_handle
from tracing.mre import function_handle
from tracing.mre import mre_result
from tracing.mre import job as job_module

_MAP_SINGLE_TRACE_CMDLINE_PATH = os.path.join(
    tracing_project.TracingProject.tracing_src_path, 'mre',
    'map_single_trace_cmdline.html')

class TemporaryMapScript(object):
  def __init__(self, js):
    tempfile_kwargs = {'mode': 'w+', 'delete': False}
    if sys.version_info >= (3,):
      tempfile_kwargs['encoding'] = 'utf-8'
    temp_file = tempfile.NamedTemporaryFile(**tempfile_kwargs)

    temp_file.write("""
<!DOCTYPE html>
<script>
%s
</script>
""" % js)
    temp_file.close()
    self._filename = temp_file.name

  def __enter__(self):
    return self

  def __exit__(self, *args, **kwargs):
    os.remove(self._filename)
    self._filename = None

  @property
  def filename(self):
    return self._filename


class FunctionLoadingFailure(failure.Failure):
  pass

class FunctionNotDefinedFailure(failure.Failure):
  pass

class MapFunctionFailure(failure.Failure):
  pass

class FileLoadingFailure(failure.Failure):
  pass

class TraceImportFailure(failure.Failure):
  pass

class NoResultsAddedFailure(failure.Failure):
  pass

class InternalMapError(Exception):
  pass

_FAILURE_NAME_TO_FAILURE_CONSTRUCTOR = {
    'FileLoadingError': FileLoadingFailure,
    'FunctionLoadingError': FunctionLoadingFailure,
    'FunctionNotDefinedError': FunctionNotDefinedFailure,
    'TraceImportError': TraceImportFailure,
    'MapFunctionError': MapFunctionFailure,
    'NoResultsAddedError': NoResultsAddedFailure
}


def MapSingleTrace(trace_handle,
                   job,
                   extra_import_options=None,
                   timeout=None):
  assert (isinstance(extra_import_options, (type(None), dict))), (
      'extra_import_options should be a dict or None.')
  project = tracing_project.TracingProject()

  all_source_paths = list(project.source_paths)
  all_source_paths.append(project.trace_processor_root_path)

  result = mre_result.MreResult()

  with trace_handle.PrepareFileForProcessing() as prepared_trace_handle:
    js_args = [
        json.dumps(prepared_trace_handle.AsDict()),
        json.dumps(job.AsDict()),
    ]
    if extra_import_options:
      js_args.append(json.dumps(extra_import_options))

    # Use 8gb heap space to make sure we don't OOM'ed on big trace, but not
    # on ARM devices since we use 32-bit d8 binary.
    if platform.machine() == 'armv7l' or platform.machine() == 'aarch64':
      v8_args = None
    else:
      v8_args = ['--max-old-space-size=8192']

    try:
      res = vinn.RunFile(
          _MAP_SINGLE_TRACE_CMDLINE_PATH,
          source_paths=all_source_paths,
          js_args=js_args,
          v8_args=v8_args,
          timeout=timeout)
    except RuntimeError as e:
      result.AddFailure(failure.Failure(
          job,
          trace_handle.canonical_url,
          'Error', 'vinn runtime error while mapping trace.',
          str(e), 'Unknown stack'))
      return result

  stdout = res.stdout
  if not isinstance(stdout, str):
    stdout = stdout.decode('utf-8', errors='replace')

  if res.returncode != 0:
    sys.stderr.write(stdout)
    result.AddFailure(failure.Failure(
        job,
        trace_handle.canonical_url,
        'Error', 'vinn runtime error while mapping trace.',
        'vinn runtime error while mapping trace.', 'Unknown stack'))
    return result

  for line in stdout.split('\n'):
    m = re.match('^MRE_RESULT: (.+)', line, re.DOTALL)
    if m:
      found_dict = json.loads(m.group(1))
      failures = [
          failure.Failure.FromDict(f, job, _FAILURE_NAME_TO_FAILURE_CONSTRUCTOR)
          for f in found_dict['failures']]

      for f in failures:
        result.AddFailure(f)

      for k, v in found_dict['pairs'].items():
        result.AddPair(k, v)

    else:
      if len(line) > 0:
        sys.stderr.write(line)
        sys.stderr.write('\n')

  if not (len(result.pairs) or len(result.failures)):
    raise InternalMapError('Internal error: No results were produced!')

  return result


def ExecuteTraceMappingCode(trace_file_path, process_trace_func_code,
                            extra_import_options=None,
                            trace_canonical_url=None):
  """Execute |process_trace_func_code| on the input |trace_file_path|.

  process_trace_func_code must contain a function named 'process_trace' with
  signature as follows:

    function processTrace(results, model) {
       // call results.addPair(<key>, <value>) to add data to results object.
    }

  Whereas results is an instance of tr.mre.MreResult, and model is an instance
  of tr.model.Model which was resulted from parsing the input trace.

  Returns:
    This function returns the dictionay that represents data collected in
    |results|.

  Raises:
    RuntimeError if there is any error with execute trace mapping code.
  """

  with TemporaryMapScript("""
     //# sourceURL=processTrace
      %s;
      tr.mre.FunctionRegistry.register(processTrace);
  """ % process_trace_func_code) as map_script:
    handle = function_handle.FunctionHandle(
        [function_handle.ModuleToLoad(filename=map_script.filename)],
        function_name='processTrace')
    mapping_job = job_module.Job(handle)
    trace_file_path = os.path.abspath(trace_file_path)
    if not trace_canonical_url:
      trace_canonical_url = 'file://%s' % trace_file_path
    trace_handle = file_handle.URLFileHandle(
        trace_file_path, trace_canonical_url)
    results = MapSingleTrace(trace_handle, mapping_job, extra_import_options)
    if results.failures:
      raise RuntimeError(
          'Failures mapping trace:\n%s' %
          ('\n'.join(str(f) for f in results.failures)))
    return results.pairs
