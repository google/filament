# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import logging
import os
import uuid
import six
import sys
import json
import statistics

from google.appengine.api import taskqueue
from google.appengine.ext import ndb

try:
  import cloudstorage.cloudstorage as cloudstorage
except ImportError:
  # This is a work around to fix the discrepency on file tree in tests.
  import cloudstorage
from apiclient.discovery import build
from dashboard.common import utils
from dashboard.common import oauth2_utils
from dashboard.pinpoint.models import job_state
from dashboard.pinpoint.models.quest import read_value
from dashboard.pinpoint.models.quest import run_test
from dashboard.services import swarming
from tracing_build import render_histograms_viewer
from tracing.value import gtest_json_converter
from tracing.value.diagnostics import generic_set
from tracing.value.diagnostics import reserved_infos

# Maps metric name -> position in the measures tree of the BQ export
_METRIC_MAP = {
    # CWV
    "largestContentfulPaint": ("core_web_vitals", "largestContentfulPaint"),
    "timeToFirstContentfulPaint":
        ("core_web_vitals", "timeToFirstContentfulPaint"),
    "overallCumulativeLayoutShift":
        ("core_web_vitals", "overallCumulativeLayoutShift"),
    "totalBlockingTime": ("core_web_vitals", "totalBlockingTime"),

    # Speedometer2
    "Angular2-TypeScript-TodoMVC":
        ("speedometer2", "Angular2_TypeScript_TodoMVC"),
    "AngularJS-TodoMVC": ("speedometer2", "AngularJS_TodoMVC"),
    "BackboneJS-TodoMVC": ("speedometer2", "BackboneJS_TodoMVC"),
    "Elm-TodoMVC": ("speedometer2", "Elm_TodoMVC"),
    "EmberJS-Debug-TodoMVC": ("speedometer2", "EmberJS_Debug_TodoMVC"),
    "EmberJS-TodoMVC": ("speedometer2", "EmberJS_TodoMVC"),
    "Flight-TodoMVC": ("speedometer2", "Flight_TodoMVC"),
    "Inferno-TodoMVC": ("speedometer2", "Inferno_TodoMVC"),
    "jQuery-TodoMVC": ("speedometer2", "jQuery_TodoMVC"),
    "Preact-TodoMVC": ("speedometer2", "Preact_TodoMVC"),
    "React-Redux-TodoMVC": ("speedometer2", "React_Redux_TodoMVC"),
    "React-TodoMVC": ("speedometer2", "React_TodoMVC"),
    "RunsPerMinute": ("speedometer2", "RunsPerMinute"),
    "Vanilla-ES2015-Babel-Webpack-TodoMVC":
        ("speedometer2", "Vanilla_ES2015_Babel_Webpack_TodoMVC"),
    "Vanilla-ES2015-TodoMVC": ("speedometer2", "Vanilla_ES2015_TodoMVC"),
    "VanillaJS-TodoMVC": ("speedometer2", "VanillaJS_TodoMVC"),
    "VueJS-TodoMVC": ("speedometer2", "VueJS_TodoMVC"),

    # MotionMark
    "motionmark": ("motionmark", "motionmark"),
    "Canvas Arcs": ("motionmark", "canvas_arcs"),
    "Canvas Lines": ("motionmark", "canvas_lines"),
    "Design": ("motionmark", "design"),
    "Images": ("motionmark", "images"),
    "Leaves": ("motionmark", "leaves"),
    "Multiply": ("motionmark", "multiply"),
    "Paths": ("motionmark", "paths"),
    "Suits": ("motionmark", "suits"),

    # Jetstream2
    # BQ unhappy if name starts with numeric
    "3d-cube-SP.Average": ("jetstream2", "cube_3d_SP_Average"),
    "3d-raytrace-SP.Average": ("jetstream2", "raytrace_3d_SP_Average"),
    "Air.Average": ("jetstream2", "Air_Average"),
    "Babylon.Average": ("jetstream2", "Babylon_Average"),
    "Basic.Average": ("jetstream2", "Basic_Average"),
    "Box2D.Average": ("jetstream2", "Box2D_Average"),
    "FlightPlanner.Average": ("jetstream2", "FlightPlanner_Average"),
    "HashSet-wasm.Runtime": ("jetstream2", "HashSet_wasm_Runtime"),
    "ML.Average": ("jetstream2", "ML_Average"),
    "OfflineAssembler.Average": ("jetstream2", "OfflineAssembler_Average"),
    "Score": ("jetstream2", "Score"),
    "UniPoker.Average": ("jetstream2", "UniPoker_Average"),
    "WSL.MainRun": ("jetstream2", "WSL_MainRun"),
    "acorn-wtb.Average": ("jetstream2", "acorn_wtb_Average"),
    "ai-astar.Average": ("jetstream2", "ai_astar_Average"),
    "async-fs.Average": ("jetstream2", "async_fs_Average"),
    "babylon-wtb.Average": ("jetstream2", "babylon_wtb_Average"),
    "base64-SP.Average": ("jetstream2", "base64_SP_Average"),
    "bomb-workers.Average": ("jetstream2", "bomb_workers_Average"),
    "cdjs.Average": ("jetstream2", "cdjs_Average"),
    "chai-wtb.Average": ("jetstream2", "chai_wtb_Average"),
    "coffeescript-wtb.Average": ("jetstream2", "coffeescript_wtb_Average"),
    "crypto-aes-SP.Average": ("jetstream2", "crypto_aes_SP_Average"),
    "crypto-md5-SP.Average": ("jetstream2", "crypto_md5_SP_Average"),
    "crypto-shal-SP.Average": ("jetstream2", "crypto_shal_SP_Average"),
    "crypto.Average": ("jetstream2", "crypto_Average"),
    "date-format-tofte-SP.Average":
        ("jetstream2", "date_format_tofte_SP_Average"),
    "date-format-xparb-SP.Average":
        ("jetstream2", "date_format_xparb_SP_Average"),
    "delta-blue.Average": ("jetstream2", "delta_blue_Average"),
    "earley-boyer.Average": ("jetstream2", "earley_boyer_Average"),
    "espree-wtb.Average": ("jetstream2", "espree_wtb_Average"),
    "first-inspector-code-load.Average":
        ("jetstream2", "first_inspector_code_load_Average"),
    "float-mm_c.Average": ("jetstream2", "float_mm_c_Average"),
    "gaussian-blur.Average": ("jetstream2", "gaussian_blur_Average"),
    "gbemu.Average": ("jetstream2", "gbemu_Average"),
    "gcc-loops-wasm.Runtime": ("jetstream2", "gcc_loops_wasm_Runtime"),
    "hash-map.Average": ("jetstream2", "hash_map_Average"),
    "jshint-wtb.Average": ("jetstream2", "jshint_wtb_Average"),
    "json-parse-inspector.Average":
        ("jetstream2", "json_parse_inspector_Average"),
    "json-stringify-inspector.Average":
        ("jetstream2", "json_stringify_inspector_Average"),
    "lebab-wtb.Average": ("jetstream2", "lebab_wtb_Average"),
    "mandreel.Average": ("jetstream2", "mandreel_Average"),
    "multi-inspector-code-load.Average":
        ("jetstream2", "multi_inspector_code_load_Average"),
    "n-body-SP.Average": ("jetstream2", "n_body_SP_Average"),
    "navier-stokes.Average": ("jetstream2", "navier_stokes_Average"),
    "octane-code-load.Average": ("jetstream2", "octane_code_load_Average"),
    "octane-zlib.Average": ("jetstream2", "octane_zlib_Average"),
    "pdfjs.Average": ("jetstream2", "pdfjs_Average"),
    "prepack-wtb.Average": ("jetstream2", "prepack_wtb_Average"),
    "quicksort-wasm.Runtime": ("jetstream2", "quicksort_wasm_Runtime"),
    "raytrace.Average": ("jetstream2", "raytrace_Average"),
    "regex-dna-SP.Average": ("jetstream2", "regex_dna_SP_Average"),
    "regexp.Average": ("jetstream2", "regexp_Average"),
    "richards-wasm.Runtime": ("jetstream2", "richards_wasm_Runtime"),
    "richards.Average": ("jetstream2", "richards_Average"),
    "segmentation.Average": ("jetstream2", "segmentation_Average"),
    "splay.Average": ("jetstream2", "splay_Average"),
    "stanford-crypto-aes.Average":
        ("jetstream2", "stanford_crypto_aes_Average"),
    "stanford-crypto-pbkdf2.Average":
        ("jetstream2", "stanford_crypto_pbkdf2_Average"),
    "stanford-crypto-sha256.Average":
        ("jetstream2", "stanford_crypto_sha256_Average"),
    "string-unpack-code-SP.Average":
        ("jetstream2", "string_unpack_code_SP_Average"),
    "tagcloud-SP.Average": ("jetstream2", "tagcloud_SP_Average"),
    "tsf-wasm.Runtime": ("jetstream2", "tsf_wasm_Runtime"),
    "typescript.Average": ("jetstream2", "typescript_Average"),
    "uglify-js-wtb.Average": ("jetstream2", "uglify_js_wtb_Average"),
}

_PROJECT_ID = 'chromeperf'

_DATASET_CHROME_HEALTH = 'pinpoint_export_test'
_TABLE_CHROME_HEALTH = 'pinpoint_results'

_DATASET_GENERAL = 'pinpoint_export'
_TABLE_GENERAL = 'results'


class Results2Error(Exception):

  pass


class CachedResults2(ndb.Model):
  """Stores data on when a results2 was generated."""

  updated = ndb.DateTimeProperty(required=True, auto_now_add=True)
  job_id = ndb.StringProperty()


class _GcsFileStream:
  """Wraps a gcs file providing a FileStream like api."""

  # pylint: disable=invalid-name

  def __init__(self, *args, **kwargs):
    self._gcs_file = cloudstorage.open(*args, **kwargs)

  def seek(self, _):
    pass

  def truncate(self):
    pass

  def write(self, data):
    self._gcs_file.write(six.ensure_binary(data))

  def close(self):
    self._gcs_file.close()


def _GetCloudStorageName(job_id):
  if utils.IsStagingEnvironment():
    return '/chromeperf-staging-results2-public/%s.html' % job_id
  return '/results2-public/%s.html' % job_id


def GetCachedResults2(job):
  logging.info('JobQueueDebug: Running result2. ID: %s', job.job_id)
  filename = _GetCloudStorageName(job.job_id)
  results = cloudstorage.listbucket(filename)
  logging.info('JobQueueDebug: Finished result2. ID: %s', job.job_id)

  for _ in results:
    return 'https://storage.cloud.google.com' + filename

  return None


def ScheduleResults2Generation(job):
  logging.debug('Job [%s]: ScheduleResults2Generation', job.job_id)
  try:
    # Don't want several tasks creating results2, so create task with specific
    # name to deduplicate.
    logging.info('JobQueueDebug: Adding result2. ID: %s', job.job_id)
    task_name = 'results2-public-%s' % job.job_id
    taskqueue.add(
        queue_name='job-queue',
        url='/api/generate-results2/' + job.job_id,
        name=task_name)
  except taskqueue.TombstonedTaskError:
    logging.info(
        'A task with the same name has executed in the queue. Job [%s]',
        job.job_id)
    return False
  except taskqueue.TaskAlreadyExistsError:
    logging.info('The task already exists. It has not yet run. Job [%s]',
                 job.job_id)
  return True


def GenerateResults2(job):
  """ Generates a results2.html and also adds results to BigQuery. """
  logging.debug('Job [%s]: GenerateResults2', job.job_id)

  vulcanized_html = _ReadVulcanizedHistogramsViewer()

  CachedResults2(job_id=job.job_id).put()

  filename = _GetCloudStorageName(job.job_id)
  gcs_file = _GcsFileStream(
      filename,
      'w',
      content_type='text/html',
      retry_params=cloudstorage.RetryParams(backoff_factor=1.1))

  render_histograms_viewer.RenderHistogramsViewer(
      [h.histogram for h in _FetchHistograms(job)],
      gcs_file,
      reset_results=True,
      vulcanized_html=vulcanized_html)

  gcs_file.close()
  logging.debug('Generated %s; see https://storage.cloud.google.com%s',
                filename, filename)

  # Only save A/B tests to the Chrome Health BigQuery
  if job.comparison_mode != job_state.FUNCTIONAL and \
      job.comparison_mode != job_state.PERFORMANCE:
    try:
      _SaveJobToChromeHealthBigQuery(job)
    except Exception as e:  # pylint: disable=broad-except
      logging.error(e)

  # Export every job to the General BigQuery
  try:
    _SaveJobToGeneralBigQuery(job)
  except Exception as e:  # pylint: disable=broad-except
    logging.error(e)


def _ReadVulcanizedHistogramsViewer():
  viewer_path = os.path.join(
      os.path.dirname(__file__), '..', '..', '..',
      'vulcanized_histograms_viewer', 'vulcanized_histograms_viewer.html')
  with open(viewer_path, 'rb') as f:
    return f.read()

HistogramMetadata = collections.namedtuple(
    'HistogramMetadata', ['attempt_number', "change", "swarming_result"])
HistogramData = collections.namedtuple('HistogramMetadata',
                                       ["metadata", "histogram"])


def _FetchHistograms(job):
  for change in _ChangeList(job):
    for attempt_number, attempt in enumerate(job.state._attempts[change]):
      swarming_result = None
      for execution in attempt.executions:
        # Attempt to extract taskID if this is a run_test._RunTestExecution
        if isinstance(execution, run_test._RunTestExecution):
          # crbug/1352875. task_id can be None if the pinpoint job is cancelled.
          if execution._task_id is None:
            logging.warning('Swarming task id is None for the execution. Skip.')
            continue
          # Query Swarming
          try:
            swarming_task = swarming.Swarming(execution._swarming_server).Task(
                execution._task_id)
            swarming_result = swarming_task.Result()
            logging.debug('Task result from Swarming V2: %s', swarming_result)
          except Exception as e:  # pylint: disable=broad-except
            logging.error("_FetchHistograms swarming query failed: %s", str(e))
          continue

        # Attempt to extract Histograms if this is a read_value.*
        mode = None
        if isinstance(execution, read_value._ReadHistogramsJsonValueExecution):
          mode = 'histograms'
        elif isinstance(execution, read_value._ReadGraphJsonValueExecution):
          mode = 'graphjson'
        elif isinstance(execution, read_value.ReadValueExecution):
          mode = execution.mode or 'histograms'

        if mode is None:
          continue

        histogram_sets = None
        if mode == 'graphjson':
          histograms = gtest_json_converter.ConvertGtestJson(
              _JsonFromExecution(execution))
          histograms.AddSharedDiagnosticToAllHistograms(
              reserved_infos.LABELS.name, generic_set.GenericSet([str(change)]))
          histogram_sets = histograms.AsDicts()
        else:
          histogram_sets = _JsonFromExecution(execution)

        logging.debug('Found %s histograms for %s', len(histogram_sets), change)

        metadata = HistogramMetadata(attempt_number, change, swarming_result)
        for histogram in histogram_sets:
          yield HistogramData(metadata, histogram)

        # Force deletion of histogram_set objects which can be O(100MB).
        del histogram_sets


def _ChangeList(job):
  # If there are differences, only include Changes with differences.
  changes = set()

  for change_a, change_b in job.state.Differences():
    changes.add(change_a)
    changes.add(change_b)

  if changes:
    return list(changes)

  return job.state._changes


def _JsonFromExecution(execution):
  if hasattr(execution, '_cas_root_ref') and execution._cas_root_ref:
    return read_value.RetrieveOutputJsonFromCAS(
        execution._cas_root_ref,
        execution._results_path,
    )

  if hasattr(execution, '_results_filename'):
    results_filename = execution._results_filename
  else:
    results_filename = 'chartjson-output.json'

  if hasattr(execution, '_isolate_server'):
    isolate_server = execution._isolate_server
  else:
    isolate_server = 'https://isolateserver.appspot.com'
  isolate_hash = execution._isolate_hash
  return read_value.RetrieveOutputJson(
      isolate_server,
      isolate_hash,
      results_filename,
  )


def _SaveJobToGeneralBigQuery(job):
  rows = []
  for h in _FetchHistograms(job):
    if "sampleValues" not in h.histogram:
      continue
    row = _PopulateMetadata(job, h)
    row["metric"] = h.histogram["name"]
    row["values"] = h.histogram["sampleValues"]
    rows.append(row)
  if rows:
    _InsertBQRows(_PROJECT_ID, _DATASET_GENERAL, _TABLE_GENERAL, rows)

RowKey = collections.namedtuple('RowKey', ['change', 'iteration'])
def _SaveJobToChromeHealthBigQuery(job):
  rows = {}  # Key is a RowKey
  for h in _FetchHistograms(job):
    if "sampleValues" not in h.histogram:
      continue
    rk = RowKey(h.metadata.change, h.metadata.attempt_number)
    if rk not in rows:
      rows[rk] = _PopulateMetadata(job, h)
      rows[rk]["measures"] = _GetEmptyMeasures()
    rows[rk] = _PopulateMetric(rows[rk], h.histogram["name"],
                               h.histogram["sampleValues"])
  empty_measures = _GetEmptyMeasures()
  rows_with_measures = [
      r for r in rows.values() if r["measures"] != empty_measures
  ]
  if len(rows_with_measures):
    _InsertBQRows(_PROJECT_ID, _DATASET_CHROME_HEALTH, _TABLE_CHROME_HEALTH,
                  rows_with_measures)


def _GetEmptyMeasures():
  measures = {}
  measures["core_web_vitals"] = {}
  measures["speedometer2"] = {}
  measures["motionmark"] = {}
  measures["jetstream2"] = {}
  return measures


def _PopulateMetric(data, name, values):
  if name in _METRIC_MAP:
    loc = _METRIC_MAP[name]
    if len(values) == 1 or loc[0] == 'speedometer2':
      data["measures"][loc[0]][loc[1]] = statistics.fmean(values)
    else:
      logging.warning('Benchmark %s and story %s have array values', loc[0],
                      name)

  return data


def _PopulateMetadata(job, h):
  md = {}
  md["job_start_time"] = _ConvertDatetimeToBQ(job.started_time)
  md["batch_id"] = job.batch_id
  md["run_id"] = job.job_id
  md["attempt_count"] = job.state.attempt_count
  md["dims"] = {}
  md["dims"]["device"] = {}
  md["dims"]["device"]["cfg"] = job.configuration
  if h.metadata.swarming_result:
    md["dims"]["start_time"] = _ConvertTsToBQ(
        h.metadata.swarming_result["started_ts"])
    md["dims"]["swarming_task_id"] = h.metadata.swarming_result["run_id"]
    # bot_dimensions is a list of dicts with "key" and "value" entries.
    for kv in h.metadata.swarming_result["bot_dimensions"]:
      if kv["key"] == "device_os":
        md["dims"]["device"]["os"] = kv["value"]
      # device_os should take precedence over os
      # if both are present, os is more generic.
      if kv["key"] == "os" and "os" not in md["dims"]["device"]:
        md["dims"]["device"]["os"] = kv["value"]
      if kv["key"] == "id" and len(kv["value"]) > 0:
        md["dims"]["device"]["swarming_bot_id"] = kv["value"][0]
  md["dims"]["test_info"] = {}
  md["dims"]["test_info"]["benchmark"] = job.benchmark_arguments.benchmark
  md["dims"]["test_info"]["story"] = job.benchmark_arguments.story
  # TODO: flags
  md["dims"]["checkout"] = {}
  # TODO: gitiles_host
  md["dims"]["checkout"]["repo"] = h.metadata.change.commits[0].repository
  md["dims"]["checkout"]["git_hash"] = h.metadata.change.commits[0].git_hash
  commit_dict = h.metadata.change.commits[0].AsDict()
  if "commit_position" in commit_dict:
    md["dims"]["checkout"]["commit_created"] = _ConvertIsotimeToBQ(
        commit_dict["created"])
    md["dims"]["checkout"]["branch"] = commit_dict["commit_branch"]
    md["dims"]["checkout"]["commit_position"] = commit_dict["commit_position"]
  if h.metadata.change.patch is not None:
    patch_params = h.metadata.change.patch.BuildParameters()
    md["dims"]["checkout"]["patch_gerrit_change"] = patch_params["patch_issue"]
    md["dims"]["checkout"]["patch_gerrit_revision"] = patch_params["patch_set"]
  md["dims"]["pairing"] = {}
  md["dims"]["pairing"]["variant"] = h.metadata.change.variant
  md["dims"]["pairing"]["replica"] = h.metadata.attempt_number
  # TODO: order (not implemented yet)

  return md

def _ConvertIsotimeToBQ(it):
  return it.replace('T', ' ') + ".000000"


def _ConvertTsToBQ(it):
  return it.replace('T', ' ')


def _ConvertDatetimeToBQ(dt):
  return dt.strftime('%Y-%m-%d %H:%M:%S.%f')


def _InsertBQRows(project_id, dataset_id, table_id, rows, num_retries=5):
  service = _BQService()
  rows = [{'insertId': str(uuid.uuid4()), 'json': row} for row in rows]
  logging.info("Saving %d rows to BQ. Total size: %d.", len(rows),
               sys.getsizeof(json.dumps(rows)))
  row_data_list = _CreateRowDataLists(rows)
  for i, row_list in enumerate(row_data_list):
    logging.info('Saving rows to BQ. Batch #%d with size %d.', i,
                 sys.getsizeof(json.dumps(row_list)))
    response = _RequestInsertBQRows(service, project_id, dataset_id, table_id,
                                    row_list, num_retries)
    if 'insertErrors' in response:
      logging.error("Insert failed: %s", str(response))


def _RequestInsertBQRows(service,
                         project_id,
                         dataset_id,
                         table_id,
                         row_list,
                         num_retries=5):
  return service.tabledata().insertAll(
      projectId=project_id,
      datasetId=dataset_id,
      tableId=table_id,
      body={
          'rows': row_list
      }).execute(num_retries=num_retries)


def _BQService():
  """Returns an initialized and authorized BigQuery client."""
  # pylint: disable=no-member
  credentials = oauth2_utils.GetAppDefaultCredentials(
      scope='https://www.googleapis.com/auth/bigquery')
  return build('bigquery', 'v2', credentials=credentials)


def _CreateRowDataLists(rows):
  # crbug/1340323, crbug/1352869
  # We saw complains on query size in Python 3 runtime. In order to limit the
  # size, we will separate the rows into multiple batches. Without a good
  # estimate, we are using 1000 rows as a conservative guess.
  batch_size = 1000

  return [rows[i:i + batch_size] for i in range(0, len(rows), batch_size)]
