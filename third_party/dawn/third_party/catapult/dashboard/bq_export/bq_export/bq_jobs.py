# Copyright (c) 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Export pinpoint job data to BigQuery with Beam & Cloud Dataflow."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import json
import logging
import six

import apache_beam as beam
from apache_beam.options.pipeline_options import DebugOptions
from apache_beam.options.pipeline_options import GoogleCloudOptions
from apache_beam.options.pipeline_options import PipelineOptions
from apache_beam.metrics import Metrics

from bq_export.split_by_timestamp import ReadTimestampRangeFromDatastore
from bq_export.export_options import BqExportOptions
from bq_export.utils import (PrintCounters, WriteToPartitionedBigQuery,
                             IsoDateToYYYYMMDD)


class UnconvertibleJobError(Exception):
  pass


def _ConvertOptionalDateTime(dt):
  if dt is None:
    return None
  return dt.isoformat()


def _JobToYYYYMMDD(row_dict):
  return IsoDateToYYYYMMDD(row_dict['create_time'])


def _IfNone(value, none_result):
  """Like SQL's IF_NULL."""
  return value if value is not None else none_result


def JobEntityToRowDict(entity):
  try:
    comparison_mode = entity.get('comparison_mode')
    if isinstance(comparison_mode, (int,)):
      # Workaround for some entities in April/May 2018 with a comparison_mode of
      # 2 (the integer, not a string!).
      comparison_mode = str(comparison_mode)

    # Skia based jobs uses UUID strings as IDs. As part of the deprecation plan,
    # this is to be migrated to Skia and should be handled there. Thus, we
    # ignore any string based ids.
    entity_id = entity.key.id
    if not isinstance(entity_id, int):
      raise TypeError("Only int based IDs are exported.")

    d = {
        'id':
            entity_id,
        'batch_id':
            entity.get('batch_id'),
        'arguments':
            entity['arguments'],  # required
        'bug_id':
            entity.get('bug_id'),
        'comparison_mode':
            comparison_mode,
        'gerrit': {
            'server': entity.get('gerrit_server'),
            'change_id': entity.get('gerrit_change_id'),
        },
        'name':
            entity.get('name'),
        'tags':
            entity.get('tags'),
        'user_email':
            entity.get('user'),
        'create_time':
            entity['created'].isoformat(),  # required
        'start_time':
            _ConvertOptionalDateTime(entity.get('started_time')),
        'update_time':
            entity['updated'].isoformat(),  # required
        'started':
            _IfNone(entity.get('started', True), False),
        'cancelled':
            _IfNone(entity.get('cancelled'), False),
        'cancel_reason':
            entity.get('cancel_reason'),
        'done':
            _IfNone(entity.get('done'), False),
        'task':
            entity.get('task'),
        'exception':
            entity.get('exception'),
        'exception_details':
            entity.get('exception_details'),
        'difference_count':
            entity.get('difference_count'),
        'retry_count':
            entity.get('retry_count', 0),
        'benchmark_arguments': {
            'benchmark': entity.get('benchmark_arguments.benchmark'),
            'story': entity.get('benchmark_arguments.story'),
            'story_tags': entity.get('benchmark_arguments.story_tags'),
            'chart': entity.get('benchmark_arguments.chart'),
            'statistic': entity.get('benchmark_arguments.statistic'),
        },
        'use_execution_engine':
            _IfNone(entity.get('use_execution_engine'), False),
    }
  except KeyError as e:
    six.raise_from(UnconvertibleJobError('Missing property: ' + str(e)), e)
  # Computed properties, directly translated from the ComputedProperty
  # definitions of the ndb.Model.
  d['completed'] = bool(
      d['done']
      or (not d['use_execution_engine'] and d['started'] and not d['task']))
  d['failed'] = bool(d.get('exception_details') or d.get('exception'))
  d['running'] = bool((not d['use_execution_engine'] and
                       (d['started'] and not d['cancelled'] and d['task'])
                       or (d['started'] and not d['completed'])))
  d['configuration'] = json.loads(d['arguments']).get('configuration')
  return d


def main():
  project = 'chromeperf'
  options = PipelineOptions()
  options.view_as(DebugOptions).add_experiment('use_beam_bq_sink')
  options.view_as(GoogleCloudOptions).project = project
  bq_export_options = options.view_as(BqExportOptions)

  p = beam.Pipeline(options=options)
  entities_read = Metrics.counter('main', 'entities_read')
  failed_entity_transforms = Metrics.counter('main', 'failed_entity_transforms')

  # Read 'Job' entities from datastore.
  job_entities = (
      p
      | 'ReadFromDatastore(Job)' >> ReadTimestampRangeFromDatastore(
          {
              'project': project,
              'kind': 'Job'
          },
          time_range_provider=bq_export_options.GetTimeRangeProvider(),
          timestamp_property='created'))

  def ConvertEntity(entity):
    entities_read.inc()
    try:
      row_dict = JobEntityToRowDict(entity)
    except TypeError:
      logging.getLogger().info('Ignoring jobs without int based ids.')
      return []
    except UnconvertibleJobError:
      logging.getLogger().exception('Failed to convert Job')
      failed_entity_transforms.inc()
      return []
    return [row_dict]

  job_dicts = (
      job_entities | 'ConvertEntityToRow(Job)' >> beam.FlatMap(ConvertEntity))

  _ = """
  CREATE TABLE `chromeperf.chromeperf_dashboard_data.jobs`
  (id INT64 NOT NULL,
   arguments STRING NOT NULL,
   bug_id INT64,
   comparison_mode STRING,
   gerrit STRUCT<server STRING, change_id STRING>,
   name STRING,
   tags STRING,
   user_email STRING,
   create_time TIMESTAMP NOT NULL,
   start_time TIMESTAMP,
   update_time TIMESTAMP NOT NULL,
   started BOOLEAN NOT NULL,
   done BOOLEAN NOT NULL,
   cancelled BOOLEAN NOT NULL,
   cancel_reason STRING,
   task STRING,
   exception STRING,
   exception_details STRING,
   difference_count INT64,
   retry_count INT64 NOT NULL,
   benchmark_arguments STRUCT<benchmark STRING, story STRING,
                              story_tags STRING, chart STRING,
                              statistic STRING>,
   use_execution_engine BOOLEAN NOT NULL,
   completed BOOLEAN NOT NULL,
   failed BOOLEAN NOT NULL,
   running BOOLEAN NOT NULL,
   configuration STRING,
   batch_id STRING)
  PARTITION BY DATE(`create_time`);
  """
  bq_job_schema = {
      'fields': [
          {
              'name': 'id',
              'type': 'INT64',
              'mode': 'REQUIRED'
          },
          {
              'name': 'arguments',
              'type': 'STRING',
              'mode': 'REQUIRED'
          },
          {
              'name': 'bug_id',
              'type': 'INT64',
              'mode': 'NULLABLE'
          },
          {
              'name': 'comparison_mode',
              'type': 'STRING',
              'mode': 'NULLABLE'
          },
          {
              'name':
                  'gerrit',
              'type':
                  'RECORD',
              'mode':
                  'NULLABLE',
              'fields': [
                  {
                      'name': 'server',
                      'type': 'STRING',
                      'mode': 'NULLABLE'
                  },
                  {
                      'name': 'change_id',
                      'type': 'STRING',
                      'mode': 'NULLABLE'
                  },
              ]
          },
          {
              'name': 'name',
              'type': 'STRING',
              'mode': 'NULLABLE'
          },
          {
              'name': 'tags',
              'type': 'STRING',
              'mode': 'NULLABLE'
          },
          {
              'name': 'user_email',
              'type': 'STRING',
              'mode': 'NULLABLE'
          },
          {
              'name': 'create_time',
              'type': 'TIMESTAMP',
              'mode': 'REQUIRED'
          },
          {
              'name': 'start_time',
              'type': 'TIMESTAMP',
              'mode': 'NULLABLE'
          },
          {
              'name': 'update_time',
              'type': 'TIMESTAMP',
              'mode': 'REQUIRED'
          },
          {
              'name': 'started',
              'type': 'BOOLEAN',
              'mode': 'REQUIRED'
          },
          {
              'name': 'done',
              'type': 'BOOLEAN',
              'mode': 'REQUIRED'
          },
          {
              'name': 'cancelled',
              'type': 'BOOLEAN',
              'mode': 'REQUIRED'
          },
          {
              'name': 'cancel_reason',
              'type': 'STRING',
              'mode': 'NULLABLE'
          },
          {
              'name': 'task',
              'type': 'STRING',
              'mode': 'NULLABLE'
          },
          {
              'name': 'exception',
              'type': 'STRING',
              'mode': 'NULLABLE'
          },
          {
              'name': 'exception_details',
              'type': 'STRING',
              'mode': 'NULLABLE'
          },
          {
              'name': 'difference_count',
              'type': 'INT64',
              'mode': 'NULLABLE'
          },
          {
              'name': 'retry_count',
              'type': 'INT64',
              'mode': 'REQUIRED'
          },
          {
              'name':
                  'benchmark_arguments',
              'type':
                  'RECORD',
              'mode':
                  'NULLABLE',
              'fields': [
                  {
                      'name': 'benchmark',
                      'type': 'STRING',
                      'mode': 'NULLABLE'
                  },
                  {
                      'name': 'story',
                      'type': 'STRING',
                      'mode': 'NULLABLE'
                  },
                  {
                      'name': 'story_tags',
                      'type': 'STRING',
                      'mode': 'NULLABLE'
                  },
                  {
                      'name': 'chart',
                      'type': 'STRING',
                      'mode': 'NULLABLE'
                  },
                  {
                      'name': 'statistic',
                      'type': 'STRING',
                      'mode': 'NULLABLE'
                  },
              ]
          },
          {
              'name': 'use_execution_engine',
              'type': 'BOOLEAN',
              'mode': 'REQUIRED'
          },
          {
              'name': 'completed',
              'type': 'BOOLEAN',
              'mode': 'REQUIRED'
          },
          {
              'name': 'failed',
              'type': 'BOOLEAN',
              'mode': 'REQUIRED'
          },
          {
              'name': 'running',
              'type': 'BOOLEAN',
              'mode': 'REQUIRED'
          },
          {
              'name': 'configuration',
              'type': 'STRING',
              'mode': 'NULLABLE'
          },
          {
              'name': 'batch_id',
              'type': 'STRING',
              'mode': 'NULLABLE'
          },
      ]
  }

  # 'dataset' may be a RuntimeValueProvider, so we have to defer calculating
  # the table name until runtime.  The simplest way to do this is by passing a
  # function for the table name rather than a string.
  def TableNameFn(unused_element):
    return '{}:{}.jobs{}'.format(project, bq_export_options.dataset.get(),
                                 bq_export_options.table_suffix)

  _ = job_dicts | 'WriteToBigQuery(jobs)' >> WriteToPartitionedBigQuery(
      TableNameFn, bq_job_schema, element_to_yyyymmdd_fn=_JobToYYYYMMDD)

  result = p.run()
  result.wait_until_finish()
  PrintCounters(result)
