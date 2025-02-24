# Copyright (c) 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Calculate statistics for chromeperf sample values.

This pipeline fetches sample_values from the BigQuery rows export, calculates
statistics about them, writes the output to separate BigQuery table.

We calculate statistics like kurtosis, skewness, etc, per measurement that we
can use to better understand the signal quality (or noisiness) of each
measurement.
"""

from __future__ import absolute_import


import datetime
import math
from typing import Dict, Any, TypeVar, List

import apache_beam as beam
from apache_beam.io.gcp.datastore.v1new import datastoreio
import apache_beam.io.gcp.datastore.v1new.types as ds_types
from apache_beam.options.pipeline_options import (GoogleCloudOptions,
                                                  PipelineOptions)
from apache_beam.options.value_provider import (NestedValueProvider,
                                                ValueProvider)
from apache_beam.typehints import (with_input_types, with_output_types)
import numpy as np
import scipy.stats
import scipy.stats.mstats

from bq_export.utils import WriteToPartitionedBigQuery, FloatHack

# Pylint doesn't understand @beam.ptransform_fn decorators, so we have to
# disable a few warnings.

# pylint: disable=no-value-for-parameter

@beam.ptransform_fn
@with_input_types(Dict[str, Any])
def SQLDictToRow(pcoll):
  """Convert a dict (from ReadFromBigQuery) to a beam.Row."""
  return pcoll | beam.Map(lambda d: beam.Row(**d))


T = TypeVar('T')

@with_input_types(List[T])
@with_output_types(List[T])
class ConcatListCombineFn(beam.CombineFn):
  """CombineFn for concatentating lists.

  Like ARRAY_CONCAT_AGG in SQL.

  E.g. [[1, 2], [3], [4,5,6]] -> [1, 2, 3, 4, 5, 6]
  """
  def create_accumulator(self):  # pylint: disable=invalid-name
    return []

  def add_input(self, accumulator, element):  # pylint: disable=invalid-name
    accumulator.extend(element)
    return accumulator

  def merge_accumulators(self, accumulators):  # pylint: disable=invalid-name
    merged = []
    for acc in accumulators:
      merged.extend(acc)
      del acc[:]
    return merged

  def extract_output(self, accumulator):  # pylint: disable=invalid-name
    return accumulator


@beam.ptransform_fn
def SampleValuesBy(pcoll, key_columns):
  """Group sample_values by key_columns.

  E.g. with this pcollection:
    col1=A col2=B col3=C sample_values=[1]
    col1=A col2=B col3=Z sample_values=[2, 3]

  p | SampleValuesBy(['col1', 'col2']) emits:
    col1=A col2=B all_sample_values=[1, 2, 3]
  """
  return (pcoll
          | beam.GroupBy(*key_columns).force_tuple_keys().aggregate_field(
              'sample_values', ConcatListCombineFn(), 'all_sample_values'))


@beam.ptransform_fn
def SampleValuesStatsBy(pcoll, *key_columns):
  """Emits 2-tuple of ((key_columns...), stats_row)."""
  return (
      pcoll
      | SQLDictToRow()
      | SampleValuesBy(key_columns)
      | SummariseSamples(key_columns)
  )


@beam.ptransform_fn
def SummariseSamples(pcoll, key_columns):
  def KeyCols(elem):
    return beam.Row(**{k: getattr(elem, k) for k in key_columns})
  def Stats(samples):
    # First, convert the Python list of floats to a numpy array, to save
    # scipy from repeatedly converting it implicitly.
    arr = np.array(samples, dtype=np.float64)

    # Immediately clear the Python list to minimise memory pressure.
    samples.clear()

    # Calculate the various summary statistics.
    stats = scipy.stats.describe(arr)
    quartiles = scipy.stats.mstats.mquantiles(arr)
    return beam.Row(
        num_samples=stats.nobs,
        kurtosis=stats.kurtosis,
        skewness=stats.skewness,
        iqr=scipy.stats.iqr(arr),
        variance=stats.variance,
        min=stats.minmax[0],
        max=stats.minmax[1],
        mean=stats.mean,
        first_quartile=quartiles[0],
        median=quartiles[1],
        third_quartile=quartiles[2],
        cv=scipy.stats.mstats.variation(arr),
        std_dev=math.sqrt(stats.variance),
        std_err=scipy.stats.sem(arr),
    )

  return (
      pcoll
      | 'SciPy.stats.*' >> beam.Map(
          lambda elem: (KeyCols(elem), Stats(elem.all_sample_values)))
  )


@beam.ptransform_fn
@with_output_types(Dict[str, Any])
def FlattenForBQ(pcoll, fixed_cols_provider):
  """Flatten KV element, and add fixed columns.

  Prepares for Big
  Flattens (or unpacks) a 2-tuple of (key_row, value_row) into a single
  dict.

  E.g. with this input:
    Fixed cols: fix1='f1', fix2='f2'
    Element #1: ((key1='A', key2='B'), (x=1.1, y=1.2))
    Element #2: ((key1='A', key2='C'), (x=2.1, y=2.2))

  Emits 2 elements:
    fix1='f1', fix2='f2', key1='A', key2='B', x=1.1, y=1.2
    fix1='f1', fix2='f2', key1='A', key2='C', x=2.1, y=2.2

  Inputs are 2-tuple of (beam.Row, beam.Row), and fixed_cols_provider is also a
  a beam.Row.

  Output is a single dict (suitable for writing to BigQuery).
  """

  def FlattenElement(elem):
    d = dict(fixed_cols_provider.get().as_dict())
    d.update(elem[0].as_dict())
    d.update(elem[1].as_dict())
    # TODO: try temp_file_format=AVRO on the BQ writer instead of needing
    # FloatHack?
    d['variance'] = FloatHack(d['variance'])
    d['cv'] = FloatHack(d['cv'])
    d['std_dev'] = FloatHack(d['std_dev'])
    d['std_err'] = FloatHack(d['std_err'])
    return d
  return pcoll | beam.Map(FlattenElement)


def CreateEntity(elem):
  """Make a SignalQualitySampleStats entity from summarised stats."""
  key = ds_types.Key(
      [
          'SignalQuality',
          '{bot_group}/{bot}/{measurement}'.format(**elem[0].as_dict()),
          'SignalQualitySampleStats',
          '0',  # version
      ],
      project='chromeperf',
  )
  entity = ds_types.Entity(key)
  properties = elem[1].as_dict()
  properties['updated_time'] = datetime.datetime.now()
  entity.set_properties(properties)
  return entity


class CalcStatsOptions(PipelineOptions):

  @classmethod
  def _add_argparse_args(cls, parser):  # pylint: disable=invalid-name
    parser.add_value_provider_argument(
        '--end_date',
        help=('Last day of data to export in YYYYMMDD format, or special value '
              '"yesterday".  Default is yesterday.  Timezone is always UTC.'),
        default="yesterday")
    parser.add_value_provider_argument(
        '--window_in_days',
        help='Number of days data summarize.', type=int, default=1)
    parser.add_value_provider_argument(
        '--dataset',
        help='BigQuery dataset name.  Overrideable for testing/dev purposes.',
        default='chromeperf_dashboard_data')
    parser.add_argument(
        '--export_to_datastore',
        help=('Export stats to DataStore as well as BigQuery.  This is off by '
              'default (so that backfills of historical data do not '
              'accidentally override current stats).'), action='store_true')

  def GetFixedColumnsProvider(self):
    def DateTransform(yyyymmdd):
      if yyyymmdd == 'yesterday':
        d = (datetime.datetime.utcnow() - datetime.timedelta(days=1)).date()
      else:
        d = datetime.date(
            int(yyyymmdd[:4]),
            int(yyyymmdd[4:6]),
            int(yyyymmdd[6:8]))
      return beam.Row(date=d)
    return NestedValueProvider(self.end_date, DateTransform)

  def GetSQLQueryProvider(self):
    query_template = '''
        SELECT
          master as bot_group,
          bot,
          measurement,
          sample_values
        FROM
          `chromeperf.chromeperf_dashboard_data.rows`
        WHERE
          (DATE(timestamp) BETWEEN DATE_SUB(DATE('{date}'), INTERVAL {days} DAY)
                           AND DATE('{date}'))
          AND ARRAY_LENGTH(sample_values) > 0
        ORDER BY master, bot, measurement;
        '''

    class SQLProvider(ValueProvider):
      def __init__(self, end_date, window_in_days):
        self.end_date = end_date
        self.window_in_days = window_in_days
      def is_accessible(self):  # pylint: disable=invalid-name
        return (self.end_date.is_accessible()
                and self.window_in_days.is_accessible())
      def get(self):
        end_date = self.end_date.get()
        if end_date == 'yesterday':
          end_date = (datetime.datetime.utcnow() -
                      datetime.timedelta(days=1)).strftime('%Y%m%d')
        yyyy_mm_dd = '{}-{}-{}'.format(
            end_date[:4], end_date[4:6], end_date[6:8])
        return query_template.format(date=yyyy_mm_dd,
                                     days=self.window_in_days.get() - 1)

    return SQLProvider(self.end_date, self.window_in_days)



def main():
  options = PipelineOptions()
  p = beam.Pipeline(options=options)
  options.view_as(GoogleCloudOptions).project = 'chromeperf'
  stats_options = options.view_as(CalcStatsOptions)
  query_provider = stats_options.GetSQLQueryProvider()

  # Query 'rows' table for sample_values
  rows = p | 'QueryTable' >> beam.io.ReadFromBigQuery(query=query_provider,
                                                      use_standard_sql=True,
                                                      validate=True,
                                                      flatten_results=False)

  # Group the sample values (by measurement, and by measurement+bot+bot_group),
  # and calculate noisiness stats.
  stats_by_m = (
      rows | 'CalcStats(measurement)' >> SampleValuesStatsBy('measurement'))
  stats_by_bg_b_m = (
      rows | 'CalcStats(bot_group,bot,measurement)' >> SampleValuesStatsBy(
          'bot_group', 'bot', 'measurement'))

  # Emit results to stats_by_* tables in BigQuery.
  _ = """
  CREATE TABLE `chromeperf.chromeperf_dashboard_data.stats_by_measurement_7d`
  (`date` DATE NOT NULL,

   measurement STRING NOT NULL,

   num_samples INT64 NOT NULL,
   kurtosis FLOAT64,
   skewness FLOAT64,
   iqr FLOAT64,
   variance FLOAT64,
   `min` FLOAT64,
   `max` FLOAT64,
   mean FLOAT64,
   first_quartile FLOAT64,
   median FLOAT64,
   third_quartile FLOAT64,
   cv FLOAT64,
   std_dev FLOAT64,
   std_err FLOAT64,
   )
  PARTITION BY `date`
  CLUSTER BY measurement;

  CREATE TABLE `chromeperf.chromeperf_dashboard_data.stats_by_botgroup_7d`
  (`date` DATE NOT NULL,

   bot_group STRING NOT NULL,
   bot STRING NOT NULL,
   measurement STRING NOT NULL,

   num_samples INT64 NOT NULL,
   kurtosis FLOAT64,
   skewness FLOAT64,
   iqr FLOAT64,
   variance FLOAT64,
   `min` FLOAT64,
   `max` FLOAT64,
   mean FLOAT64,
   first_quartile FLOAT64,
   median FLOAT64,
   third_quartile FLOAT64,
   cv FLOAT64,
   std_dev FLOAT64,
   std_err FLOAT64,
   )
  PARTITION BY `date`
  CLUSTER BY bot_group, bot, measurement;
  """
  bq_stats_by_measurement_schema = {
      'fields': [
          {'name': 'date', 'type': 'DATE', 'mode': 'REQUIRED'},

          {'name': 'measurement', 'type': 'STRING', 'mode': 'REQUIRED'},

          {'name': 'num_samples', 'type': 'INT64', 'mode': 'REQUIRED'},
          {'name': 'kurtosis', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'skewness', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'iqr', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'variance', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'min', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'max', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'mean', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'first_quartile', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'median', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'third_quartile', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'cv', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'std_dev', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'std_err', 'type': 'FLOAT', 'mode': 'NULLABLE'},
      ],
  }

  bq_stats_by_bg_b_m_schema = {
      'fields': [
          {'name': 'date', 'type': 'DATE', 'mode': 'REQUIRED'},

          {'name': 'bot_group', 'type': 'STRING', 'mode': 'REQUIRED'},
          {'name': 'bot', 'type': 'STRING', 'mode': 'REQUIRED'},
          {'name': 'measurement', 'type': 'STRING', 'mode': 'REQUIRED'},

          {'name': 'num_samples', 'type': 'INT64', 'mode': 'REQUIRED'},
          {'name': 'kurtosis', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'skewness', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'iqr', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'variance', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'min', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'max', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'mean', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'first_quartile', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'median', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'third_quartile', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'cv', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'std_dev', 'type': 'FLOAT', 'mode': 'NULLABLE'},
          {'name': 'std_err', 'type': 'FLOAT', 'mode': 'NULLABLE'},
      ],
  }

  def GetTableNameFn(table_name):
    def TableNameFn(unused_element):
      # The tables are partitioned by end date (only), so we have to manually
      # partition by window size so that pipelines for e.g. 7d vs. 28d
      # windows don't overwrite each other.  Thus we include days in the table
      # name (rather than as an extra column).
      return '{project}:{dataset}.{table_name}_{days}d'.format(
          project=options.view_as(GoogleCloudOptions).project,
          dataset=stats_options.dataset.get(),
          table_name=table_name,
          days=stats_options.window_in_days.get())
    return TableNameFn

  def DateToYYYYMMDD(elem):
    return elem['date'].strftime('%Y%m%d')

  _ = (stats_by_m
       | 'ConvertToBigQueryRow(by_measurement)' >> FlattenForBQ(
           stats_options.GetFixedColumnsProvider())
       | 'WriteToPartitionedBigQuery(stats_by_measurement)'
       >> WriteToPartitionedBigQuery(
           GetTableNameFn('stats_by_measurement'),
           schema=bq_stats_by_measurement_schema,
           element_to_yyyymmdd_fn=DateToYYYYMMDD,
           additional_bq_parameters={
               'clustering': {'fields': ['measurement']}}))
  _ = (stats_by_bg_b_m
       | 'ConvertToBigQueryRow(by_botgroup)' >> FlattenForBQ(
           stats_options.GetFixedColumnsProvider())
       | 'WriteToPartitionedBigQuery(stats_by_botgroup)'
       >> WriteToPartitionedBigQuery(
           GetTableNameFn('stats_by_botgroup'),
           schema=bq_stats_by_bg_b_m_schema,
           element_to_yyyymmdd_fn=DateToYYYYMMDD,
           additional_bq_parameters={
               'clustering': {'fields': ['bot_group', 'bot', 'measurement']}}))

  if stats_options.export_to_datastore:
    _ = (
        stats_by_bg_b_m
        | 'CreateEntity(SignalQuality)' >> beam.Map(CreateEntity)
        | beam.Reshuffle()
        | datastoreio.WriteToDatastore('chromeperf')
    )


  result = p.run()
  result.wait_until_finish()
