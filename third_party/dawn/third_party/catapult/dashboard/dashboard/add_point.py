# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""URL endpoint to allow Buildbot slaves to post data to the dashboard."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import copy
import json
import logging
import math
import re
import six

from google.appengine.api import datastore_errors
from google.appengine.api import taskqueue
from google.appengine.ext import ndb

from dashboard.api import api_auth
from dashboard.common import request_handler
from dashboard.common import datastore_hooks
from dashboard.common import histogram_helpers
from dashboard.common import math_utils
from dashboard.models import graph_data

from flask import request, make_response

_TASK_QUEUE_NAME = 'new-points-queue'

# Number of rows to process per task queue task. This limits the task size
# and execution time (Limits: 100KB object size and 10 minutes execution time).
_TASK_QUEUE_SIZE = 32

# Max length for a Row property name.
_MAX_COLUMN_NAME_LENGTH = 25

# Maximum length of a value for a string property.
_STRING_COLUMN_MAX_LENGTH = 400

# Maximum number of properties for a Row.
_MAX_NUM_COLUMNS = 30

# Maximum length for a test path. This limit is required because the test path
# used as the string ID for TestContainer (the parent in the datastore for Row
# entities), and datastore imposes a maximum string ID length.
_MAX_TEST_PATH_LENGTH = 500


class BadRequestError(Exception):
  """An error indicating that a 400 response status should be returned."""


def AddPointPost():
  """Validates data parameter and add task to queue to process points.

  The row data comes from a "data" parameter, which is a JSON encoding of a
  list of dictionaries, each of which represents one performance result
  (one point in a graph) and associated data.

    [
      {
        "master": "ChromiumPerf",
        "bot": "xp-release-dual-core",
        "test": "dromaeo/dom/modify",
        "revision": 123456789,
        "value": 24.66,
        "error": 2.33,
        "units": "ms",
        "supplemental_columns": {
          "d_median": 24234.12,
          "d_mean": 23.553,
          "r_webkit": 423340,
          ...
        },
        ...
      },
      ...
    ]

  In general, the required fields are "master", "bot", "test" (which together
  form the test path which identifies the series that this point belongs to),
  and "revision" and "value", which are the X and Y values for the point.

  This API also supports the Dashboard JSON v1.0 format (go/telemetry-json),
  the first producer of which is Telemetry. Telemetry provides lightweight
  serialization of values it produces, as JSON. If a dashboard JSON object is
  passed, it will be a single dict rather than a list, with the test,
  value, error, and units fields replaced by a chart_data field containing a
  Chart JSON dict (see design doc, and example below). Dashboard JSON v1.0 is
  processed by converting it into rows (which can be viewed as Dashboard JSON
  v0).

  {
    "master": "ChromiumPerf",
    <other row fields>,
    "chart_data": {
      "foo": {
        "bar": {
          "type": "scalar",
          "name": "foo.bar",
          "units": "ms",
          "value": 4.2,
        },
        "summary": {
          "type": "list_of_scalar_values",
          "name": "foo",
          "units": "ms",
          "values": [4.2, 5.7, 6.8],
          "std": 1.30512,
        },
    },
  }

  Request parameters:
    data: JSON encoding of a list of dictionaries.

  Outputs:
    Empty 200 response with if successful,
    200 response with warning message if optional data is invalid,
    403 response with error message if sender IP is not white-listed,
    400 response with error message if required data is invalid.
    500 with error message otherwise.
  """
  datastore_hooks.SetPrivilegedRequest()
  try:
    api_auth.Authorize()
  except api_auth.ApiAuthException as error:
    logging.error('Auth error: %s', error)
    return request_handler.RequestHandlerReportError('User unauthorized.', 403)

  data_str = request.values.get('data')
  if not data_str:
    return request_handler.RequestHandlerReportError(
        'Missing "data" parameter.', status=400)

  return AddData(data_str)


def AddData(data_str):
  try:
    data = json.loads(data_str)
  except ValueError:
    return request_handler.RequestHandlerReportError(
        'Invalid JSON string.', status=400)

  try:
    if isinstance(data, dict):
      if data.get('chart_data'):
        data = _DashboardJsonToRawRows(data)
        if not data:
          return make_response('')  # No data to add, bail out.
      else:
        return request_handler.RequestHandlerReportError(
            'Data should be a list of rows or a Dashboard JSON v1.0 dict.',
            status=400)

    if data:
      # We only need to validate the row ID for one point, since all points
      # being handled by this upload should have the same row ID.
      last_added_entity = _GetLastAddedEntityForRow(data[0])
      _ValidateRowId(data[0], last_added_entity)

    for row_dict in data:
      ValidateRowDict(row_dict)
    _AddTasks(data)
    return make_response('')
  except BadRequestError as e:
    # If any of the data was invalid, abort immediately and return an error.
    return request_handler.RequestHandlerReportError(str(e), status=400)


def _ValidateNameString(value, name):
  if not value:
    raise BadRequestError('No %s name given.' % name)
  if not isinstance(value, six.string_types):
    raise BadRequestError('Error: %s must be a string' % name)
  if '/' in value:
    raise BadRequestError('Illegal slash in %s' % name)


def _ValidateDashboardJson(dash_json_dict):
  assert isinstance(dash_json_dict, dict)
  # A Dashboard JSON dict should at least have all charts coming from the
  # same master, bot and rev. It can contain multiple charts, however.
  _ValidateNameString(dash_json_dict.get('master'), 'master')
  _ValidateNameString(dash_json_dict.get('bot'), 'bot')

  if not dash_json_dict.get('point_id'):
    raise BadRequestError('No point_id number given.')
  if not dash_json_dict.get('chart_data'):
    raise BadRequestError('No chart data given.')

  charts = dash_json_dict.get('chart_data', {}).get('charts', {})

  for _, v in charts.items():
    if not isinstance(v, dict):
      raise BadRequestError('Expected be dict: %s' % str(v))


def _DashboardJsonToRawRows(dash_json_dict):
  """Formats a Dashboard JSON dict as a list of row dicts.

  For the dashboard to begin accepting the Telemetry Dashboard JSON format
  as per go/telemetry-json, this function chunks a Dashboard JSON literal
  into rows and passes the resulting list to _AddTasks.

  Args:
    dash_json_dict: A dashboard JSON v1.0 dict.

  Returns:
    A list of dicts, each of which represents a point.

  Raises:
    AssertionError: The given argument wasn't a dict.
    BadRequestError: The content of the input wasn't valid.
  """
  _ValidateDashboardJson(dash_json_dict)

  test_suite_name = _TestSuiteName(dash_json_dict)

  chart_data = dash_json_dict.get('chart_data', {})
  charts = chart_data.get('charts', {})
  if not charts:
    return []  # No charts implies no data to add.

  # Links to about:tracing traces are listed under 'trace'; if they
  # exist copy them to a separate dictionary and delete from the chartjson
  # so that we don't try to process them as data points.
  tracing_links = None
  if 'trace' in charts:
    tracing_links = charts['trace'].copy()
    del charts['trace']
  row_template = _MakeRowTemplate(dash_json_dict)

  benchmark_description = chart_data.get('benchmark_description', '')
  is_ref = bool(dash_json_dict.get('is_ref'))
  rows = []

  for chart in charts:
    for trace in charts[chart]:
      # Need to do a deep copy here so we don't copy a_tracing_uri data.
      row = copy.deepcopy(row_template)
      specific_vals = _FlattenTrace(test_suite_name, chart, trace,
                                    charts[chart][trace], is_ref, tracing_links,
                                    benchmark_description)
      # Telemetry may validly produce rows that represent a value of NaN. To
      # avoid getting into messy situations with alerts, we do not add such
      # rows to be processed.
      if not (math.isnan(specific_vals['value'])
              or math.isnan(specific_vals['error'])):
        if specific_vals['tracing_uri']:
          row['supplemental_columns']['a_tracing_uri'] = specific_vals[
              'tracing_uri']
        row.update(specific_vals)
        rows.append(row)

  return rows


def _TestSuiteName(dash_json_dict):
  """Extracts a test suite name from Dashboard JSON.

  The dashboard JSON may contain a field "test_suite_name". If this is not
  present or it is None, the dashboard will fall back to using "benchmark_name"
  in the "chart_data" dict.
  """
  name = None
  if dash_json_dict.get('test_suite_name'):
    name = dash_json_dict['test_suite_name']
  else:
    try:
      name = dash_json_dict['chart_data']['benchmark_name']
    except KeyError as e:
      six.raise_from(
          BadRequestError('Could not find test suite name. ' + str(e)), e)

  _ValidateNameString(name, 'test_suite_name')

  return name


def _AddTasks(data):
  """Puts tasks on queue for adding data.

  Args:
    data: A list of dictionaries, each of which represents one point.
  """
  task_list = []
  for data_sublist in _Chunk(data, _TASK_QUEUE_SIZE):
    task_list.append(
        taskqueue.Task(
            url='/add_point_queue', params={'data': json.dumps(data_sublist)}))

  queue = taskqueue.Queue(_TASK_QUEUE_NAME)
  futures = [
      queue.add_async(t) for t in _Chunk(task_list, taskqueue.MAX_TASKS_PER_ADD)
  ]
  for f in futures:
    f.get_result()


def _Chunk(items, chunk_size):
  """Breaks a long list into sub-lists of a particular size."""
  chunks = []
  for i in range(0, len(items), chunk_size):
    chunks.append(items[i:i + chunk_size])
  return chunks


def _MakeRowTemplate(dash_json_dict):
  """Produces a template for rows created from a Dashboard JSON v1.0 dict.

  _DashboardJsonToRawRows adds metadata fields to every row that it creates.
  These include things like master, bot, point ID, versions, and other
  supplementary data. This method produces a dict containing this metadata
  to which row-specific information (like value and error) can be added.
  Some metadata needs to be transformed to conform to the v0 format, and this
  method is also responsible for that transformation.

  Some validation is deferred until after the input is converted to a list
  of row dicts, since revision format correctness is checked on a per-point
  basis.

  Args:
    dash_json_dict: A dashboard JSON v1.0 dict.

  Returns:
    A dict containing data to include in each row dict that is created from
    |dash_json_dict|.
  """
  row_template = dash_json_dict.copy()

  del row_template['chart_data']
  del row_template['point_id']

  row_template['revision'] = dash_json_dict['point_id']

  annotations = row_template['supplemental']
  versions = row_template['versions']

  del row_template['supplemental']
  del row_template['versions']
  row_template['supplemental_columns'] = {}
  supplemental = row_template['supplemental_columns']

  for annotation in annotations:
    supplemental['a_' + annotation] = annotations[annotation]

  for version in versions:
    supplemental['r_' + version] = versions[version]

  return row_template


def _FlattenTrace(test_suite_name,
                  chart_name,
                  trace_name,
                  trace,
                  is_ref=False,
                  tracing_links=None,
                  benchmark_description=''):
  """Takes a trace dict from dashboard JSON and readies it for display.

  Traces can be either scalars or lists; if scalar we take the value directly;
  if list we average the values and compute their standard deviation. We also
  extract fields that are normally part of v0 row dicts that are uploaded
  using add_point but are actually part of traces in the v1.0 format.

  Args:
    test_suite_name: The name of the test suite (benchmark).
    chart_name: The name of the chart to which this trace belongs.
    trace_name: The name of the passed trace.
    trace: A trace dict extracted from a dashboard JSON chart.
    is_ref: A boolean which indicates whether this trace comes from a
        reference build.
    tracing_links: A dictionary mapping trace names to about:tracing trace
        urls in cloud storage
    benchmark_description: A string documenting the benchmark suite to which
        this trace belongs.

  Returns:
    A dict containing units, value, and error for this trace.

  Raises:
    BadRequestError: The data wasn't valid.
  """
  if '@@' in chart_name:
    grouping_label, chart_name = chart_name.split('@@')
    chart_name = chart_name + '/' + grouping_label

  value, error = _ExtractValueAndError(trace)

  # If there is a link to an about:tracing trace in cloud storage for this
  # test trace_name, cache it.
  tracing_uri = None
  if (tracing_links and trace_name in tracing_links
      and 'cloud_url' in tracing_links[trace_name]):
    tracing_uri = tracing_links[trace_name]['cloud_url'].replace('\\/', '/')

  story_name = trace_name
  trace_name = histogram_helpers.EscapeName(trace_name)
  if trace_name == 'summary':
    subtest_name = chart_name
  else:
    subtest_name = chart_name + '/' + trace_name

  name = test_suite_name + '/' + subtest_name
  if trace_name == 'summary' and is_ref:
    name += '/ref'
  elif trace_name != 'summary' and is_ref:
    name += '_ref'

  units = trace.get('units')
  if units is None:
    raise BadRequestError('Units must be specified in the chart data')

  row_dict = {
      'test': name,
      'value': value,
      'error': error,
      'units': units,
      'tracing_uri': tracing_uri,
      'benchmark_description': benchmark_description,
  }

  if 'improvement_direction' in trace:
    improvement_direction_str = trace['improvement_direction']
    if improvement_direction_str is None:
      raise BadRequestError('improvement_direction must not be None')
    row_dict['higher_is_better'] = _ImprovementDirectionToHigherIsBetter(
        improvement_direction_str)

  if story_name != trace_name:
    row_dict['unescaped_story_name'] = story_name

  return row_dict


def _ExtractValueAndError(trace):
  """Returns the value and measure of error from a chartjson trace dict.

  Args:
    trace: A dict that has one "result" from a performance test, e.g. one
        "value" in a Telemetry test, with the keys "trace_type", "value", etc.

  Returns:
    A pair (value, error) where |value| is a float and |error| is some measure
    of variance used to show error bars; |error| could be None.

  Raises:
    BadRequestError: Data format was invalid.
  """
  trace_type = trace.get('type')

  if trace_type == 'scalar':
    value = trace.get('value')
    if value is None and trace.get('none_value_reason'):
      return float('nan'), 0
    try:
      return float(value), 0
    except Exception as e:  # pylint: disable=broad-except
      six.raise_from(
          BadRequestError('Expected scalar value, got: %r' % value), e)

  if trace_type == 'list_of_scalar_values':
    values = trace.get('values')
    if not isinstance(values, list) and values is not None:
      # Something else (such as a single scalar, or string) was given.
      raise BadRequestError('Expected list of scalar values, got: %r' % values)
    if not values or None in values:
      # None was included or values is None; this is not an error if there
      # is a reason.
      if trace.get('none_value_reason'):
        return float('nan'), float('nan')
      raise BadRequestError('Expected list of scalar values, got: %r' % values)
    if not all(_IsNumber(v) for v in values):
      raise BadRequestError('Non-number found in values list: %r' % values)
    value = math_utils.Mean(values)
    std = trace.get('std')
    if std is not None:
      error = std
    else:
      error = math_utils.StandardDeviation(values)
    return value, error

  if trace_type == 'histogram':
    return _GeomMeanAndStdDevFromHistogram(trace)

  raise BadRequestError('Invalid value type in chart object: %r' % trace_type)


def _IsNumber(v):
  return isinstance(v, (float, int))


def _GeomMeanAndStdDevFromHistogram(histogram):
  """Generates the geom. mean and std. dev. for a histogram.

  A histogram is a collection of numerical buckets with associated
  counts; a bucket can either represent a number of instances of a single
  value ('low'), or from within a range of values (in which case 'high' will
  specify the upper bound). We compute the statistics by treating the
  histogram analogously to a list of individual values, where the counts tell
  us how many of each value there are.

  Args:
    histogram: A histogram dict with a list 'buckets' of buckets.

  Returns:
    The geometric mean and standard deviation of the given histogram.
  """
  # Note: This code comes originally from
  # build/scripts/common/chromium_utils.py and was used initially for
  # processing histogram results on the buildbot side previously.
  if 'buckets' not in histogram:
    return 0.0, 0.0
  count = 0
  sum_of_logs = 0
  for bucket in histogram['buckets']:
    if 'high' in bucket:
      bucket['mean'] = (bucket['low'] + bucket['high']) / 2.0
    else:
      bucket['mean'] = bucket['low']
    if bucket['mean'] > 0:
      sum_of_logs += math.log(bucket['mean']) * bucket['count']
      count += bucket['count']

  if count == 0:
    return 0.0, 0.0

  sum_of_squares = 0
  geom_mean = math.exp(sum_of_logs / count)
  for bucket in histogram['buckets']:
    if bucket['mean'] > 0:
      sum_of_squares += (bucket['mean'] - geom_mean)**2 * bucket['count']
  return geom_mean, math.sqrt(sum_of_squares / count)


def _ImprovementDirectionToHigherIsBetter(improvement_direction_str):
  """Converts an improvement direction string to a higher_is_better boolean.

  Args:
    improvement_direction_str: a string, either 'up' or 'down'.

  Returns:
    A boolean expressing the appropriate higher_is_better value.

  Raises:
    BadRequestError: if improvement_direction_str is invalid.
  """
  # We use improvement_direction if given. Otherwise, by not providing it here
  # we'll fall back to a default from dashboard.units_to_direction module.
  # TODO(eakuefner): Fail instead of falling back after fixing crbug.com/459450.
  if improvement_direction_str == 'up':
    return True
  if improvement_direction_str == 'down':
    return False
  raise BadRequestError('Invalid improvement direction string: ' +
                        improvement_direction_str)


def _GetLastAddedEntityForRow(row):
  if not ('master' in row and 'bot' in row and 'test' in row):
    return None
  path = '%s/%s/%s' % (row['master'], row['bot'], row['test'].strip('/'))
  if len(path) > _MAX_TEST_PATH_LENGTH:
    return None

  try:
    last_added_revision_entity = ndb.Key('LastAddedRevision', path).get()
  except datastore_errors.BadRequestError:
    logging.warning('Datastore BadRequestError when getting %s', path)
    return None

  return last_added_revision_entity


def ValidateRowDict(row):
  """Checks all fields in the input dictionary.

  Args:
    row: A dictionary which represents one point.

  Raises:
    BadRequestError: The input was not valid.
  """
  required_fields = ['master', 'bot', 'test']
  for field in required_fields:
    if field not in row:
      raise BadRequestError('No "%s" field in row dict.' % field)
  _ValidateMasterBotTest(row['master'], row['bot'], row['test'])
  GetAndValidateRowProperties(row)


def _ValidateMasterBotTest(master, bot, test):
  """Validates the master, bot, and test properties of a row dict."""
  # Trailing and leading slashes in the test name are ignored.
  # The test name must consist of at least a test suite plus sub-test.
  test = test.strip('/')
  if '/' not in test:
    raise BadRequestError('Test name must have more than one part.')

  if len(test.split('/')) > graph_data.MAX_TEST_ANCESTORS:
    raise BadRequestError('Invalid test name: %s' % test)

  _ValidateNameString(master, 'master')
  _ValidateNameString(bot, 'bot')
  _ValidateTestPath('%s/%s/%s' % (master, bot, test))


def _ValidateTestPath(test_path):
  """Checks whether all the parts of the test path are valid."""
  # A test with a test path length over the max key length shouldn't be
  # created, since the test path is used in TestContainer keys.
  if len(test_path) > _MAX_TEST_PATH_LENGTH:
    raise BadRequestError('Test path too long: %s' % test_path)

  # Stars are reserved for test path patterns, so they can't be used in names.
  if '*' in test_path:
    raise BadRequestError('Illegal asterisk in test name.')

  for name in test_path.split('/'):
    _ValidateTestPathPartName(name)


def _ValidateTestPathPartName(name):
  """Checks whether a Master, Bot or TestMetadata name is OK."""
  # NDB Datastore doesn't allow key names to start and with "__" and "__".
  if name.startswith('__') and name.endswith('__'):
    raise BadRequestError(
        'Invalid name: "%s". Names cannot start and end with "__".' % name)


def _ValidateRowId(row_dict, last_added_entity):
  """Checks whether the ID for a Row is OK.

  Args:
    row_dict: A dictionary with new point properties, including "revision".
    last_added_entity: The last previous added revision entity for the test.

  Raises:
    BadRequestError: The revision is not acceptable for some reason.
  """
  row_id = GetAndValidateRowId(row_dict)

  # Get the last added revision number for this test.
  last_row_id = None
  if not last_added_entity:
    master, bot, test = row_dict['master'], row_dict['bot'], row_dict['test']
    test_path = '%s/%s/%s' % (master, bot, test)
    # Could be first point in test.
    logging.warning('Test %s has no last added revision entry.', test_path)
  else:
    last_row_id = last_added_entity.revision

  if not _IsAcceptableRowId(row_id, last_row_id):
    raise BadRequestError(
        'Invalid ID (revision) %d; compared to previous ID %s, it was larger '
        'or smaller by too much and must not be <= 0.' % (row_id, last_row_id))


def _IsAcceptableRowId(row_id, last_row_id):
  """Checks whether the given row id (aka revision) is not too large or small.

  For each data series (i.e. TestMetadata entity), we assume that row IDs are
  monotonically increasing. On a given chart, points are sorted by these
  row IDs. This way, points can arrive out of order but still be shown
  correctly in the chart.

  However, sometimes a bot might start to use a different *type* of row ID;
  for example it might change from revision numbers or build numbers to
  timestamps, or from timestamps to build numbers. This causes a lot of
  problems, including points being put out of order.

  If a sender of data actually wants to switch to a different type of
  row ID, it would be much cleaner for them to start sending it under a new
  chart name.

  Args:
    row_id: The proposed Row entity id (usually sent as "revision")
    last_row_id: The previous Row id, or None if there were none previous.

  Returns:
    True if acceptable, False otherwise.
  """
  if row_id <= 0:
    return False
  if last_row_id is None:
    return True
  # Too big of a decrease.
  if row_id < 0.5 * last_row_id:
    return False
  # Too big of an increase.
  if row_id > 2 * last_row_id:
    return False
  return True


def GetAndValidateRowId(row_dict):
  """Returns the integer ID for a new Row.

  This method is also responsible for validating the input fields related
  to making the new row ID.

  Args:
    row_dict: A dictionary obtained from the input JSON.

  Returns:
    An integer row ID.

  Raises:
    BadRequestError: The input wasn't formatted properly.
  """
  if 'revision' not in row_dict:
    raise BadRequestError('Required field "revision" missing.')
  try:
    return int(row_dict['revision'])
  except (ValueError, TypeError) as e:
    raise BadRequestError(
        'Bad value for "revision", should be numerical.') from e


def GetAndValidateRowProperties(row):
  """From the object received, make a dictionary of properties for a Row.

  This includes the default "value" and "error" columns as well as all
  supplemental columns, but it doesn't include "revision", and it doesn't
  include input fields that are properties of the parent TestMetadata, such as
  "units".

  This method is responsible for validating all properties that are to be
  properties of the new Row.

  Args:
    row: A dictionary obtained from the input JSON.

  Returns:
    A dictionary of the properties and property values to set when creating
    a Row. This will include "value" and "error" as well as all supplemental
    columns.

  Raises:
    BadRequestError: The properties weren't formatted correctly.
  """
  columns = {}

  # Value and error must be floating point numbers.
  if 'value' not in row:
    raise BadRequestError('No "value" given.')
  try:
    columns['value'] = float(row['value'])
  except (ValueError, TypeError) as e:
    six.raise_from(
        BadRequestError('Bad value for "value", should be numerical.'), e)
  if 'error' in row:
    try:
      error = float(row['error'])
      columns['error'] = error
    except (ValueError, TypeError):
      logging.warning('Bad value for "error".')
  if 'swarming_bot_id' in row:
    try:
      swarming_bot_id = str(row['swarming_bot_id'])
      columns['swarming_bot_id'] = swarming_bot_id
    except (ValueError, TypeError):
      logging.warning('Bad value for "swarming_bot_id".')
  columns.update(_GetSupplementalColumns(row))
  return columns


def _GetSupplementalColumns(row):
  """Gets a dict of supplemental columns.

  If any columns are invalid, a warning is logged and they just aren't included,
  but no exception is raised.

  Individual rows may specify up to _MAX_NUM_COLUMNS extra data, revision,
  and annotation columns. These columns must follow formatting rules for
  their type. Invalid columns are dropped with an error log, but the valid
  data will still be graphed.

  Args:
    row: A dict, possibly with the key "supplemental_columns", the value of
        which should be a dict.

  Returns:
    A dict of valid supplemental columns.
  """
  columns = {}
  for (name, value) in row.get('supplemental_columns', {}).items():
    # Don't allow too many columns
    if len(columns) == _MAX_NUM_COLUMNS:
      logging.warning('Too many columns, some being dropped.')
      break
    value = _CheckSupplementalColumn(name, value)
    if value:
      columns[name] = value
  return columns


def _CheckSupplementalColumn(name, value):
  """Returns a possibly modified value for a supplemental column, or None."""
  # Check length of column name.
  name = str(name)
  if len(name) > _MAX_COLUMN_NAME_LENGTH:
    logging.warning('Supplemental column name too long.')
    return None

  # The column name has a prefix which indicates type of value.
  if name[:2] not in ('d_', 'r_', 'a_'):
    logging.warning('Bad column name "%s", invalid prefix.', name)
    return None

  # The d_ prefix means "data column", intended to hold numbers.
  if name.startswith('d_'):
    try:
      value = float(value)
    except (ValueError, TypeError):
      logging.warning('Bad value for column "%s", should be numerical.', name)
      return None

  # The r_ prefix means "revision", and the value should look like a number,
  # a version number, or a git commit hash.
  if name.startswith('r_'):
    revision_patterns = [
        r'^\d+$',
        r'^\d+\.\d+\.\d+\.\d+$',
        r'^[A-Fa-f0-9]{40}$',
    ]
    if (not value or len(str(value)) > _STRING_COLUMN_MAX_LENGTH
        or not any(re.match(p, str(value)) for p in revision_patterns)):
      logging.warning('Bad value for revision column "%s". Value: %s', name,
                      value)
      return None
    value = str(value)

  if name.startswith('a_'):
    # Annotation column, is typically a short string.
    # Bot_ID lists can be long, truncate if exceed max length
    if len(str(value)) > _STRING_COLUMN_MAX_LENGTH:
      logging.warning('Value for "%s" too long, truncated to max length %d.',
                      name,
                      _STRING_COLUMN_MAX_LENGTH)
      if isinstance(value, list):
        while len(str(value)) > _STRING_COLUMN_MAX_LENGTH:
          value.pop()
      elif isinstance(value, str):
        value = value[:_STRING_COLUMN_MAX_LENGTH]
      else:
        logging.warning('Value for "%s" is not truncatable', name)
        return None

  return value
