# Copyright (c) 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Export chromeperf dashboard data to BigQuery with Beam & Cloud Dataflow."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import apache_beam as beam
from apache_beam.options.pipeline_options import DebugOptions
from apache_beam.options.pipeline_options import GoogleCloudOptions
from apache_beam.options.pipeline_options import PipelineOptions
from apache_beam.metrics import Metrics

from bq_export.split_by_timestamp import ReadTimestampRangeFromDatastore
from bq_export.export_options import BqExportOptions
from bq_export.utils import (TestPath, FloatHack, PrintCounters,
                             WriteToPartitionedBigQuery)


class UnconvertibleAnomalyError(Exception):
  pass


def main():
  project = 'chromeperf'
  options = PipelineOptions()
  options.view_as(DebugOptions).add_experiment('use_beam_bq_sink')
  options.view_as(GoogleCloudOptions).project = project
  bq_export_options = options.view_as(BqExportOptions)

  p = beam.Pipeline(options=options)
  entities_read = Metrics.counter('main', 'entities_read')
  failed_entity_transforms = Metrics.counter('main', 'failed_entity_transforms')

  # Read 'Anomaly' entities from datastore.
  entities = (
      p
      | 'ReadFromDatastore(Anomaly)' >> ReadTimestampRangeFromDatastore(
          {
              'project': project,
              'kind': 'Anomaly'
          },
          time_range_provider=bq_export_options.GetTimeRangeProvider()))

  def AnomalyEntityToRowDict(entity):
    entities_read.inc()
    try:
      # We do the iso conversion of the nullable timestamps in isolation.
      earliest_input_timestamp = entity.get('earliest_input_timestamp')
      if earliest_input_timestamp:
        earliest_input_timestamp = earliest_input_timestamp.isoformat()
      latest_input_timestamp = entity.get('latest_input_timestamp')
      if latest_input_timestamp:
        latest_input_timestamp = latest_input_timestamp.isoformat()
      d = {
          'id': entity.key.id,
          # TODO: 'sheriff'
          # 'subscriptions' omitted; subscription_names is sufficient
          'subscription_names': entity.get('subscription_names', []),
          'test': TestPath(entity['test']),
          'start_revision': entity['start_revision'],
          'end_revision': entity['end_revision'],
          'display_start': entity.get('display_start'),
          'display_end': entity.get('display_end'),
          # TODO: 'ownership'
          'statistic': entity['statistic'],
          'bug_id': entity['bug_id'],
          'internal_only': entity['internal_only'],
          'timestamp': entity['timestamp'].isoformat(),
          'segment_size_before': entity.get('segment_size_before'),
          'segment_size_after': entity.get('segment_size_after'),
          'median_before_anomaly': entity.get('median_before_anomaly'),
          'median_after_anomaly': entity.get('median_after_anomaly'),
          'std_dev_before_anomaly': entity.get('std_dev_before_anomaly'),
          'window_end_revision': entity.get('window_end_revision'),
          't_statistic': FloatHack(entity.get('t_statistic')),
          'degrees_of_freedom': entity.get('degrees_of_freedom'),
          'p_value': entity.get('p_value'),
          'is_improvement': entity.get('is_improvement', False),
          'recovered': entity.get('recovered', False),
          # TODO: 'ref_test'
          'units': entity.get('units'),
          # TODO: 'recipe_bisects'
          'pinpoint_bisects': entity.get('pinpoint_bisects', []),
          # These are critical to "time-to-culprit" calculations.
          'earliest_input_timestamp': earliest_input_timestamp,
          'latest_input_timestamp': latest_input_timestamp,
      }
      if d['statistic'] is None:
        # Some years-old anomalies lack this.
        raise UnconvertibleAnomalyError()
      return [d]
    except (KeyError, UnconvertibleAnomalyError):
      failed_entity_transforms.inc()
      return []

  anomaly_dicts = (
      entities
      | 'ConvertEntityToRow(Anomaly)' >> beam.FlatMap(AnomalyEntityToRowDict))

  _ = """
  CREATE TABLE `chromeperf.chromeperf_dashboard_data.anomalies`
  (id INT64 NOT NULL,
   `timestamp` TIMESTAMP NOT NULL,
   subscription_names ARRAY<STRING>,
   `test` STRING NOT NULL,
   start_revision INT64 NOT NULL,
   end_revision INT64 NOT NULL,
   display_start INT64,
   display_end INT64,
   statistic STRING NOT NULL,
   bug_id INT64,
   internal_only BOOLEAN NOT NULL,
   segment_size_before INT64,
   segment_size_after INT64,
   median_before_anomaly FLOAT64,
   median_after_anomaly FLOAT64,
   std_dev_before_anomaly FLOAT64,
   window_end_revision INT64,
   t_statistic FLOAT64,
   degrees_of_freedom FLOAT64,
   p_value FLOAT64,
   is_improvement BOOLEAN NOT NULL,
   recovered BOOLEAN NOT NULL,
   units STRING,
   pinpoint_bisects ARRAY<STRING>,
   earliest_input_timestamp TIMESTAMP,
   latest_input_timestamp TIMESTAMP)
  PARTITION BY DATE(`timestamp`);
  """
  bq_anomaly_schema = {
      'fields': [
          {
              'name': 'id',
              'type': 'INT64',
              'mode': 'REQUIRED'
          },
          {
              'name': 'subscription_names',
              'type': 'STRING',
              'mode': 'REPEATED'
          },
          {
              'name': 'test',
              'type': 'STRING',
              'mode': 'REQUIRED'
          },
          {
              'name': 'start_revision',
              'type': 'INT64',
              'mode': 'REQUIRED'
          },
          {
              'name': 'end_revision',
              'type': 'INT64',
              'mode': 'REQUIRED'
          },
          {
              'name': 'display_start',
              'type': 'INT64',
              'mode': 'NULLABLE'
          },
          {
              'name': 'display_end',
              'type': 'INT64',
              'mode': 'NULLABLE'
          },
          {
              'name': 'statistic',
              'type': 'STRING',
              'mode': 'REQUIRED'
          },
          {
              'name': 'bug_id',
              'type': 'INT64',
              'mode': 'NULLABLE'
          },
          {
              'name': 'internal_only',
              'type': 'BOOLEAN',
              'mode': 'REQUIRED'
          },
          {
              'name': 'timestamp',
              'type': 'TIMESTAMP',
              'mode': 'REQUIRED'
          },
          {
              'name': 'segment_size_before',
              'type': 'INT64',
              'mode': 'NULLABLE'
          },
          {
              'name': 'segment_size_after',
              'type': 'INT64',
              'mode': 'NULLABLE'
          },
          {
              'name': 'median_before_anomaly',
              'type': 'FLOAT',
              'mode': 'NULLABLE'
          },
          {
              'name': 'median_after_anomaly',
              'type': 'FLOAT',
              'mode': 'NULLABLE'
          },
          {
              'name': 'std_dev_before_anomaly',
              'type': 'FLOAT',
              'mode': 'NULLABLE'
          },
          {
              'name': 'window_end_revision',
              'type': 'INT64',
              'mode': 'NULLABLE'
          },
          {
              'name': 't_statistic',
              'type': 'FLOAT',
              'mode': 'NULLABLE'
          },
          {
              'name': 'degrees_of_freedom',
              'type': 'FLOAT',
              'mode': 'NULLABLE'
          },
          {
              'name': 'p_value',
              'type': 'FLOAT',
              'mode': 'NULLABLE'
          },
          {
              'name': 'is_improvement',
              'type': 'BOOLEAN',
              'mode': 'REQUIRED'
          },
          {
              'name': 'recovered',
              'type': 'BOOLEAN',
              'mode': 'REQUIRED'
          },
          {
              'name': 'units',
              'type': 'STRING',
              'mode': 'NULLABLE'
          },
          {
              'name': 'pinpoint_bisects',
              'type': 'STRING',
              'mode': 'REPEATED'
          },
          {
              'name': 'earliest_input_timestamp',
              'type': 'TIMESTAMP',
              'mode': 'NULLABLE'
          },
          {
              'name': 'latest_input_timestamp',
              'type': 'TIMESTAMP',
              'mode': 'NULLABLE'
          },
      ]
  }

  # 'dataset' may be a RuntimeValueProvider, so we have to defer calculating
  # the table name until runtime.  The simplest way to do this is by passing a
  # function for the table name rather than a string.
  def TableNameFn(unused_element):
    return '{}:{}.anomalies{}'.format(project, bq_export_options.dataset.get(),
                                      bq_export_options.table_suffix)

  _ = (
      anomaly_dicts | 'WriteToBigQuery(anomalies)' >>
      WriteToPartitionedBigQuery(TableNameFn, bq_anomaly_schema))

  result = p.run()
  result.wait_until_finish()
  PrintCounters(result)
