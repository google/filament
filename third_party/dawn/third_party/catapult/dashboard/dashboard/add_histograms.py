# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""URL endpoint for adding new histograms to the dashboard."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import decimal
import ijson
import json
import logging
import six
import sys
import uuid
import zlib
try:
  import cloudstorage.cloudstorage as cloudstorage
except ImportError:
  # This is a work around to fix the discrepency on file tree in tests.
  import cloudstorage

from google.appengine.api import taskqueue
from google.appengine.ext import ndb

from dashboard import sheriff_config_client
from dashboard.api import api_auth
from dashboard.api import api_request_handler
from dashboard.common import datastore_hooks
from dashboard.common import histogram_helpers
from dashboard.common import timing
from dashboard.common import utils
from dashboard.models import graph_data
from dashboard.models import histogram
from dashboard.models import upload_completion_token
from tracing.value import histogram_set
from tracing.value.diagnostics import diagnostic
from tracing.value.diagnostics import reserved_infos

from flask import make_response, request

TASK_QUEUE_NAME = 'histograms-queue'

_RETRY_PARAMS = cloudstorage.RetryParams(backoff_factor=1.1)
_TASK_RETRY_LIMIT = 4
_ZLIB_BUFFER_SIZE = 4096


def _CheckUser():
  if utils.IsDevAppserver():
    return
  api_auth.Authorize()
  if not utils.IsInternalUser():
    raise api_request_handler.ForbiddenError()


def AddHistogramsProcessPost():
  datastore_hooks.SetPrivilegedRequest()
  token = None

  try:
    params = json.loads(request.get_data())
    gcs_file_path = params['gcs_file_path']

    token_id = params.get('upload_completion_token')
    if token_id is not None:
      token = upload_completion_token.Token.get_by_id(token_id)
      upload_completion_token.Token.UpdateObjectState(
          token, upload_completion_token.State.PROCESSING)

    try:
      logging.debug('Loading %s', gcs_file_path)
      gcs_file = cloudstorage.open(
          gcs_file_path, 'r', retry_params=_RETRY_PARAMS)
      with DecompressFileWrapper(gcs_file) as decompressing_file:
        histogram_dicts = _LoadHistogramList(decompressing_file)

      gcs_file.close()

      ProcessHistogramSet(histogram_dicts, token)
    finally:
      cloudstorage.delete(gcs_file_path, retry_params=_RETRY_PARAMS)

    upload_completion_token.Token.UpdateObjectState(
        token, upload_completion_token.State.COMPLETED)
    return make_response('{}')

  except Exception as e:  # pylint: disable=broad-except
    logging.error('Error processing histograms: %s', str(e), exc_info=1)
    upload_completion_token.Token.UpdateObjectState(
        token, upload_completion_token.State.FAILED, str(e))
    return make_response(json.dumps({'error': str(e)}))


def _GetCloudStorageBucket():
  if utils.IsStagingEnvironment():
    return 'chromeperf-staging-add-histograms-cache'
  return 'add-histograms-cache'


@api_request_handler.RequestHandlerDecoratorFactory(_CheckUser)
def AddHistogramsPost():
  if utils.IsDevAppserver():
    # Don't require developers to zip the body.
    # In prod, the data will be written to cloud storage and processed on the
    # taskqueue, so the caller will not see any errors. In dev_appserver,
    # process the data immediately so the caller will see errors.
    # Also always create upload completion token for such requests.
    token, token_info = _CreateUploadCompletionToken()
    ProcessHistogramSet(
        _LoadHistogramList(six.StringIO(request.get_data())), token)
    token.UpdateState(upload_completion_token.State.COMPLETED)
    return token_info

  with timing.WallTimeLogger('decompress'):
    try:
      data_str = request.get_data()
      # Try to decompress at most 100 bytes from the data, only to determine
      # if we've been given compressed payload.
      zlib.decompressobj().decompress(data_str, 100)
      logging.info('Received compressed data.')
    except zlib.error as e:
      data_str = request.form['data']
      if not data_str:
        six.raise_from(
            api_request_handler.BadRequestError(
                'Missing or uncompressed data.'), e)
      data_str = zlib.compress(six.ensure_binary(data_str))
      logging.info('Received uncompressed data.')

  if not data_str:
    raise api_request_handler.BadRequestError('Missing "data" parameter')

  filename = uuid.uuid4()
  params = {'gcs_file_path': '/%s/%s' % (_GetCloudStorageBucket(), filename)}

  gcs_file = cloudstorage.open(
      params['gcs_file_path'],
      'w',
      content_type='application/octet-stream',
      retry_params=_RETRY_PARAMS)
  gcs_file.write(data_str)
  gcs_file.close()

  _, token_info = _CreateUploadCompletionToken(params['gcs_file_path'])
  params['upload_completion_token'] = token_info['token']

  retry_options = taskqueue.TaskRetryOptions(
      task_retry_limit=_TASK_RETRY_LIMIT)
  queue = taskqueue.Queue('default')

  queue.add(
      taskqueue.Task(
          url='/add_histograms/process',
          payload=json.dumps(params),
          retry_options=retry_options))
  return token_info


def _LoadHistogramList(input_file):
  """Incremental file decoding and JSON parsing when handling new histograms.

  This helper function takes an input file which yields fragments of JSON
  encoded histograms then incrementally builds the list of histograms to return
  the fully formed list in the end.

  Returns
    This function returns an instance of a list() containing dict()s decoded
    from the input_file.

  Raises
    This function may raise ValueError instances if we end up not finding valid
    JSON fragments inside the file.
  """
  try:
    with timing.WallTimeLogger('json.load'):

      def NormalizeDecimals(obj):
        # Traverse every object in obj to turn Decimal objects into floats.
        if isinstance(obj, decimal.Decimal):
          return float(obj)
        if isinstance(obj, dict):
          for k, v in obj.items():
            obj[k] = NormalizeDecimals(v)
        if isinstance(obj, list):
          obj = [NormalizeDecimals(x) for x in obj]
        return obj

      objects = [NormalizeDecimals(x) for x in ijson.items(input_file, 'item')]

  except ijson.JSONError as e:
    # Wrap exception in a ValueError
    six.raise_from(ValueError('Failed to parse JSON: %s' % (e)), e)

  return objects


def _CreateUploadCompletionToken(temporary_staging_file_path=None):
  token_info = {
      'token': str(uuid.uuid4()),
      'file': temporary_staging_file_path,
  }
  token = upload_completion_token.Token(
      id=token_info['token'],
      temporary_staging_file_path=temporary_staging_file_path,
  )
  token.put()
  logging.info('Upload completion token created. Token id: %s',
               token_info['token'])
  return token, token_info


def _LogDebugInfo(histograms):
  hist = histograms.GetFirstHistogram()
  if not hist:
    logging.info('No histograms in data.')
    return

  log_urls = hist.diagnostics.get(reserved_infos.LOG_URLS.name)
  if log_urls:
    log_urls = list(log_urls)
    msg = 'Buildbot URL: %s' % str(log_urls)
    logging.info(msg)
  else:
    logging.info('No LOG_URLS in data.')

  build_urls = hist.diagnostics.get(reserved_infos.BUILD_URLS.name)
  if build_urls and hasattr(build_urls, '__iter__'):
    build_urls = list(build_urls)
    msg = 'Build URL: %s' % str(build_urls)
    logging.info(msg)
  else:
    logging.info('No BUILD_URLS in data.')


def ProcessHistogramSet(histogram_dicts, completion_token=None):
  if not isinstance(histogram_dicts, list):
    raise api_request_handler.BadRequestError(
        'HistogramSet JSON must be a list of dicts')

  histograms = histogram_set.HistogramSet()

  with timing.WallTimeLogger('hs.ImportDicts'):
    histograms.ImportDicts(histogram_dicts)

  with timing.WallTimeLogger('hs.DeduplicateDiagnostics'):
    histograms.DeduplicateDiagnostics()

  if len(histograms) == 0:
    raise api_request_handler.BadRequestError(
        'HistogramSet JSON must contain at least one histogram.')

  with timing.WallTimeLogger('hs._LogDebugInfo'):
    _LogDebugInfo(histograms)

  with timing.WallTimeLogger('InlineDenseSharedDiagnostics'):
    InlineDenseSharedDiagnostics(histograms)

  # TODO(#4242): Get rid of this.
  # https://github.com/catapult-project/catapult/issues/4242
  with timing.WallTimeLogger('_PurgeHistogramBinData'):
    _PurgeHistogramBinData(histograms)

  with timing.WallTimeLogger('_GetDiagnosticValue calls'):
    master = _GetDiagnosticValue(reserved_infos.MASTERS.name,
                                 histograms.GetFirstHistogram())
    bot = _GetDiagnosticValue(reserved_infos.BOTS.name,
                              histograms.GetFirstHistogram())
    benchmark = _GetDiagnosticValue(reserved_infos.BENCHMARKS.name,
                                    histograms.GetFirstHistogram())
    benchmark_description = _GetDiagnosticValue(
        reserved_infos.BENCHMARK_DESCRIPTIONS.name,
        histograms.GetFirstHistogram(),
        optional=True)

  with timing.WallTimeLogger('_ValidateMasterBotBenchmarkName'):
    _ValidateMasterBotBenchmarkName(master, bot, benchmark)

  with timing.WallTimeLogger('ComputeRevision'):
    suite_key = utils.TestKey('%s/%s/%s' % (master, bot, benchmark))
    logging.info('Suite: %s', suite_key.id())

    revision = ComputeRevision(histograms)
    logging.info('Revision: %s', revision)

    internal_only = graph_data.Bot.GetInternalOnlySync(master, bot)

  revision_record = histogram.HistogramRevisionRecord.GetOrCreate(
      suite_key, revision)
  revision_record.put()

  last_added = histogram.HistogramRevisionRecord.GetLatest(
      suite_key).get_result()

  # On first upload, a query immediately following a put may return nothing.
  if not last_added:
    last_added = revision_record

  _CheckRequest(last_added, 'No last revision')

  # We'll skip the histogram-level sparse diagnostics because we need to
  # handle those with the histograms, below, so that we can properly assign
  # test paths.
  with timing.WallTimeLogger('FindSuiteLevelSparseDiagnostics'):
    suite_level_sparse_diagnostic_entities = FindSuiteLevelSparseDiagnostics(
        histograms, suite_key, revision, internal_only)

  # TODO(896856): Refactor master/bot computation to happen above this line
  # so that we can replace with a DiagnosticRef rather than a full diagnostic.
  with timing.WallTimeLogger('DeduplicateAndPut'):
    new_guids_to_old_diagnostics = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            suite_level_sparse_diagnostic_entities, suite_key, revision,
            last_added.revision).get_result())

  with timing.WallTimeLogger('ReplaceSharedDiagnostic calls'):
    for new_guid, old_diagnostic in new_guids_to_old_diagnostics.items():
      histograms.ReplaceSharedDiagnostic(
          new_guid, diagnostic.Diagnostic.FromDict(old_diagnostic))

  with timing.WallTimeLogger('_CreateHistogramTasks'):
    tasks = _CreateHistogramTasks(suite_key.id(), histograms, revision,
                                  benchmark_description, completion_token)

  with timing.WallTimeLogger('_QueueHistogramTasks'):
    _QueueHistogramTasks(tasks)


def _ValidateMasterBotBenchmarkName(master, bot, benchmark):
  for n in (master, bot, benchmark):
    if '/' in n:
      raise api_request_handler.BadRequestError('Illegal slash in %s' % n)


def _QueueHistogramTasks(tasks):
  queue = taskqueue.Queue(TASK_QUEUE_NAME)
  futures = []
  for i in range(0, len(tasks), taskqueue.MAX_TASKS_PER_ADD):
    f = queue.add_async(tasks[i:i + taskqueue.MAX_TASKS_PER_ADD])
    futures.append(f)
  for f in futures:
    f.get_result()


def _MakeTask(params):
  return taskqueue.Task(
      url='/add_histograms_queue',
      payload=json.dumps(params),
      _size_check=False)


def _CreateHistogramTasks(suite_path,
                          histograms,
                          revision,
                          benchmark_description,
                          completion_token=None):
  tasks = []
  duplicate_check = set()
  measurement_add_futures = []
  sheriff_client = sheriff_config_client.GetSheriffConfigClient()

  for hist in histograms:
    diagnostics = FindHistogramLevelSparseDiagnostics(hist)
    test_path = '%s/%s' % (suite_path, histogram_helpers.ComputeTestPath(hist))

    # Log the information here so we can see which histograms are being queued.
    logging.debug('Queueing: %s', test_path)

    if test_path in duplicate_check:
      raise api_request_handler.BadRequestError(
          'Duplicate histogram detected: %s' % test_path)

    duplicate_check.add(test_path)

    # We create one task per histogram, so that we can get as much time as we
    # need for processing each histogram per task.
    task_dict = _MakeTaskDict(hist, test_path, revision, benchmark_description,
                              diagnostics, completion_token)
    tasks.append(_MakeTask([task_dict]))

    if completion_token is not None:
      measurement_add_futures.append(
          completion_token.AddMeasurement(
              test_path, utils.IsMonitored(sheriff_client, test_path)))
  ndb.Future.wait_all(measurement_add_futures)

  return tasks


def _MakeTaskDict(hist,
                  test_path,
                  revision,
                  benchmark_description,
                  diagnostics,
                  completion_token=None):
  # TODO(simonhatch): "revision" is common to all tasks, as is the majority of
  # the test path
  params = {
      'test_path': test_path,
      'revision': revision,
      'benchmark_description': benchmark_description
  }

  # By changing the GUID just before serializing the task, we're making it
  # unique for each histogram. This avoids each histogram trying to write the
  # same diagnostic out (datastore contention), at the cost of copyin the
  # data. These are sparsely written to datastore anyway, so the extra
  # storage should be minimal.
  for d in diagnostics.values():
    d.ResetGuid()

  diagnostics = {k: d.AsDict() for k, d in diagnostics.items()}

  params['diagnostics'] = diagnostics
  params['data'] = hist.AsDict()

  if completion_token is not None:
    params['token'] = completion_token.key.id()

  return params


def FindSuiteLevelSparseDiagnostics(histograms, suite_key, revision,
                                    internal_only):
  diagnostics = {}
  for hist in histograms:
    for name, diag in hist.diagnostics.items():
      if name in histogram_helpers.SUITE_LEVEL_SPARSE_DIAGNOSTIC_NAMES:
        existing_entity = diagnostics.get(name)
        if existing_entity is None:
          diagnostics[name] = histogram.SparseDiagnostic(
              id=diag.guid,
              data=diag.AsDict(),
              test=suite_key,
              start_revision=revision,
              end_revision=sys.maxsize,
              name=name,
              internal_only=internal_only)
        elif existing_entity.key.id() != diag.guid:
          raise ValueError(name +
                           ' diagnostics must be the same for all histograms')
  return list(diagnostics.values())


def FindHistogramLevelSparseDiagnostics(hist):
  diagnostics = {}
  for name, diag in hist.diagnostics.items():
    if name in histogram_helpers.HISTOGRAM_LEVEL_SPARSE_DIAGNOSTIC_NAMES:
      diagnostics[name] = diag
  return diagnostics


def _GetDiagnosticValue(name, hist, optional=False):
  if optional:
    if name not in hist.diagnostics:
      return None

  _CheckRequest(name in hist.diagnostics,
                'Histogram [%s] missing "%s" diagnostic' % (hist.name, name))
  value = hist.diagnostics[name]
  _CheckRequest(len(value) == 1, 'Histograms must have exactly 1 "%s"' % name)
  return value.GetOnlyElement()


def ComputeRevision(histograms):
  _CheckRequest(len(histograms) > 0, 'Must upload at least one histogram')
  rev = _GetDiagnosticValue(
      reserved_infos.POINT_ID.name,
      histograms.GetFirstHistogram(),
      optional=True)

  if rev is None:
    rev = _GetDiagnosticValue(
        reserved_infos.CHROMIUM_COMMIT_POSITIONS.name,
        histograms.GetFirstHistogram(),
        optional=True)

  if rev is None:
    revision_timestamps = histograms.GetFirstHistogram().diagnostics.get(
        reserved_infos.REVISION_TIMESTAMPS.name)
    _CheckRequest(
        revision_timestamps is not None,
        'Must specify REVISION_TIMESTAMPS, CHROMIUM_COMMIT_POSITIONS,'
        ' or POINT_ID')
    rev = revision_timestamps.max_timestamp

  if not isinstance(rev, int):
    raise api_request_handler.BadRequestError('Point ID must be an integer.')

  return rev


def InlineDenseSharedDiagnostics(histograms):
  # TODO(896856): Delete inlined diagnostics from the set
  for hist in histograms:
    diagnostics = hist.diagnostics
    for name, diag in diagnostics.items():
      if name not in histogram_helpers.SPARSE_DIAGNOSTIC_NAMES:
        diag.Inline()


def _PurgeHistogramBinData(histograms):
  # We do this because RelatedEventSet and Breakdown data in bins is
  # enormous in their current implementation.
  for cur_hist in histograms:
    for cur_bin in cur_hist.bins:
      for dm in cur_bin.diagnostic_maps:
        keys = list(dm.keys())
        for k in keys:
          del dm[k]


def _CheckRequest(condition, msg):
  if not condition:
    raise api_request_handler.BadRequestError(msg)


class DecompressFileWrapper:
  """A file-like object implementing inline decompression.

  This class wraps a file-like object and does chunk-based decoding of the data.
  We only implement the read() function supporting fixed-chunk reading, capped
  to a predefined constant buffer length.

  Example

    with open('filename', 'r') as input:
      decompressor = DecompressFileWrapper(input)
      while True:
        chunk = decompressor.read(4096)
        if len(chunk) == 0:
          break
        // handle the chunk with size <= 4096
  """

  def __init__(self, source_file, buffer_size=_ZLIB_BUFFER_SIZE):
    self.source_file = source_file
    self.decompressor = zlib.decompressobj()
    self.buffer_size = buffer_size

  def __enter__(self):
    return self

  def read(self, size=None):  # pylint: disable=invalid-name
    if size is None or size < 0:
      size = self.buffer_size

    # We want to read chunks of data from the buffer, chunks at a time.
    temporary_buffer = self.decompressor.unconsumed_tail
    if len(temporary_buffer) < self.buffer_size / 2:
      raw_buffer = self.source_file.read(size)
      if raw_buffer:
        temporary_buffer += raw_buffer

    if len(temporary_buffer) == 0:
      return b''

    decompressed_data = self.decompressor.decompress(temporary_buffer, size)
    return decompressed_data

  def close(self):  # pylint: disable=invalid-name
    self.decompressor.flush()

  def __exit__(self, exception_type, exception_value, execution_traceback):
    self.close()
    return False
