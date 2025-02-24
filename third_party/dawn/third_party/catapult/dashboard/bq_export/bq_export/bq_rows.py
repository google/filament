# Copyright (c) 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Export chromeperf Row data to BigQuery with Beam & Cloud Dataflow."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import datetime
import itertools
import json
import logging
import zlib

import apache_beam as beam
from apache_beam.options.pipeline_options import DebugOptions
from apache_beam.options.pipeline_options import GoogleCloudOptions
from apache_beam.options.pipeline_options import PipelineOptions
from apache_beam.metrics import Metrics
from apache_beam.transforms.core import FlatMap

from bq_export.split_by_timestamp import ReadTimestampRangeFromDatastore
from bq_export.export_options import BqExportOptions
from bq_export.utils import (FloatHack, PrintCounters,
                             WriteToPartitionedBigQuery)


def main():
  project = 'chromeperf'
  options = PipelineOptions()
  options.view_as(DebugOptions).add_experiment('use_beam_bq_sink')
  options.view_as(GoogleCloudOptions).project = project
  bq_export_options = options.view_as(BqExportOptions)

  p = beam.Pipeline(options=options)
  entities_read = Metrics.counter('main', 'entities_read')
  failed_entity_transforms = Metrics.counter('main', 'failed_entity_transforms')
  row_conflicts = Metrics.counter('main', 'row_conflicts')
  multiple_histograms_for_row = Metrics.counter(
      'main', 'multiple_histograms_for_row')
  orphaned_histogram = Metrics.counter('main', 'orphaned_histogram')

  _ = """
  CREATE TABLE `chromeperf.chromeperf_dashboard_rows.<MASTER>`
  (revision INT64 NOT NULL,
   value FLOAT64 NOT NULL,
   std_error FLOAT64,
   `timestamp` TIMESTAMP NOT NULL,
   master STRING NOT NULL,
   bot STRING NOT NULL,
   measurement STRING,
   test STRING NOT NULL,
   properties STRING,
   sample_values ARRAY<FLOAT64>)
  PARTITION BY DATE(`timestamp`)
  CLUSTER BY master, bot, measurement;
  """
  bq_row_schema = {
      'fields': [
          {
              'name': 'revision',
              'type': 'INT64',
              'mode': 'REQUIRED'
          },
          {
              'name': 'value',
              'type': 'FLOAT',
              'mode': 'REQUIRED'
          },
          {
              'name': 'std_error',
              'type': 'FLOAT',
              'mode': 'NULLABLE'
          },
          {
              'name': 'timestamp',
              'type': 'TIMESTAMP',
              'mode': 'REQUIRED'
          },
          {
              'name': 'master',
              'type': 'STRING',
              'mode': 'REQUIRED'
          },
          {
              'name': 'bot',
              'type': 'STRING',
              'mode': 'REQUIRED'
          },
          {
              'name': 'measurement',
              'type': 'STRING',
              'mode': 'NULLABLE'
          },
          {
              'name': 'test',
              'type': 'STRING',
              'mode': 'REQUIRED'
          },
          {
              'name': 'properties',
              'type': 'STRING',
              'mode': 'NULLABLE'
          },
          {
              'name': 'sample_values',
              'type': 'FLOAT',
              'mode': 'REPEATED'
          },
      ]
  }

  def RowEntityToRowDict(entity):
    entities_read.inc()
    try:
      d = {
          'revision': entity.key.id,
          'value': FloatHack(entity['value']),
          'std_error': FloatHack(entity.get('error')),
          'timestamp': entity['timestamp'].isoformat(),
          'test': entity.key.parent.name,
      }
      # Add the expando properties as a JSON-encoded dict.
      properties = {}
      for key, value in entity.items():
        if key in d or key in ['parent_test', 'error']:
          # skip properties with dedicated columns.
          continue
        if isinstance(value, datetime.date):
          value = value.isoformat()
        if isinstance(value, float):
          value = FloatHack(value)
        properties[key] = value
      d['properties'] = json.dumps(properties) if properties else None
      # Add columns derived from test: master, bot.
      test_path_parts = d['test'].split('/', 2)
      if len(test_path_parts) >= 3:
        d['master'] = test_path_parts[0]
        d['bot'] = test_path_parts[1]
        d['measurement'] = '/'.join(test_path_parts[2:])
      return [d]
    except KeyError:
      logging.getLogger().exception('Failed to convert Row')
      failed_entity_transforms.inc()
      return []

  row_query_params = dict(project=project, kind='Row')
  row_entities = (
      p
      | 'ReadFromDatastore(Row)' >> ReadTimestampRangeFromDatastore(
          row_query_params,
          time_range_provider=bq_export_options.GetTimeRangeProvider(),
          step=datetime.timedelta(minutes=5)))

  row_dicts = (
      row_entities | 'ConvertEntityToDict(Row)' >> FlatMap(RowEntityToRowDict))

  # The sample_values are not found in the Row entity.  So we have to fetch all
  # the corresponding Histogram entities and join them with our collection of
  # Rows (by using test + revision as the join key).  We also need to unpack the
  # sample values arrays out of the zlib-compressed JSON stored in the
  # Histogram's "data" property.
  def HistogramEntityToDict(entity):
    """Returns dicts with keys: 'test', 'revision', 'sample_values'."""
    entities_read.inc()
    try:
      data = entity['data']
    except KeyError:
      logging.getLogger().exception('Histogram missing "data" field')
      failed_entity_transforms.inc()
      return []
    try:
      json_str = zlib.decompress(data)
    except zlib.error:
      logging.getLogger().exception('Histogram data not valid zlib: %r', data)
      failed_entity_transforms.inc()
      return []
    try:
      data_dict = json.loads(json_str)
    except json.JSONDecodeError:
      logging.getLogger().exception('Histogram data not valid json.')
      failed_entity_transforms.inc()
      return []
    sample_values = data_dict.get('sampleValues', [])
    if not isinstance(sample_values, list):
      logging.getLogger().exception(
          'Histogram data.sampleValues not valid list.')
      failed_entity_transforms.inc()
      return []
    count = len(sample_values)
    sample_values = [v for v in sample_values if v is not None]
    if len(sample_values) != count:
      logging.getLogger().warning(
          'Histogram data.sampleValues contains null: %r', entity.key)
    for v in sample_values:
      if not isinstance(v, (int, float)):
        logging.getLogger().exception(
            'Histogram data.sampleValues contains non-numeric: %r', v)
        failed_entity_transforms.inc()
        return []
    try:
      return [{
          'test': entity['test'].name,
          'revision': entity['revision'],
          'sample_values': sample_values,
      }]
    except KeyError:
      logging.getLogger().exception(
          'Histogram missing test or revision field/s')
      failed_entity_transforms.inc()
      return []

  histogram_query_params = dict(project=project, kind='Histogram')
  histogram_entities = (
      p
      | 'ReadFromDatastore(Histogram)' >> ReadTimestampRangeFromDatastore(
          histogram_query_params,
          time_range_provider=bq_export_options.GetTimeRangeProvider(),
          step=datetime.timedelta(minutes=5)))

  histogram_dicts = (
      histogram_entities
      | 'ConvertEntityToDict(Histogram)' >> FlatMap(HistogramEntityToDict))

  def TestRevision(element):
    return (element['test'], element['revision'])

  rows_with_key = (
      row_dicts | 'WithKeys(Row)' >> beam.WithKeys(TestRevision)
  )
  histograms_with_key = (
      histogram_dicts | 'WithKeys(Histogram)' >> beam.WithKeys(TestRevision)
  )

  def MergeRowAndSampleValues(element):
    group_key, join_values = element
    rows, histograms = join_values
    if len(rows) == 0:
      orphaned_histogram.inc()
      logging.getLogger().error("No Row for Histogram(s) (%r)", group_key)
      return []
    if len(rows) > 1:
      row_conflicts.inc()
      logging.getLogger().error("Multiple rows (%d) for %r", len(rows),
                                group_key)
      return rows
    row = rows[0]
    if len(histograms) > 1:
      # We'll merge these, so this isn't an error.
      multiple_histograms_for_row.inc()
    elif len(histograms) == 0:
      # No sample values to annotate the row with.  This is common.
      return [row]
    # Merge multiple histogram's values into a single row.
    row['sample_values'] = list(itertools.chain.from_iterable(
        h['sample_values'] for h in histograms))
    return [row]

  joined_and_annotated = (
      (rows_with_key, histograms_with_key)
      | beam.CoGroupByKey()
      | beam.FlatMap(MergeRowAndSampleValues)
  )

  def TableNameFn(unused_element):
    return '{project}:{dataset}.rows{suffix}'.format(
        project=project,
        dataset=bq_export_options.dataset.get(),
        suffix=bq_export_options.table_suffix)

  _ = (
      joined_and_annotated
      | 'WriteToBigQuery(rows)' >> WriteToPartitionedBigQuery(
          TableNameFn, bq_row_schema, additional_bq_parameters={
              'clustering': {'fields': ['master', 'bot', 'measurement']}})
  )

  result = p.run()
  result.wait_until_finish()
  PrintCounters(result)
