# Copyright (c) 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
r"""Export chromeperf rows to BigQuery with Beam & Cloud Dataflow.

Example command line to start a Dataflow job to backfill 10 days (with
reasonable defaults for other values)::

  $ SVC_ACCT=bigquery-exporter@chromeperf.iam.gserviceaccount.com
  $ python bq_export/export_rows.py \
        --service_account_email=$SVC_ACCT \
        --runner=DataflowRunner \
        --region=us-central1 \
        --temp_location=gs://chromeperf-dataflow-temp/ \
        --experiments=use_beam_bq_sink \
        --experiments=shuffle_mode=service \
        --setup_file=bq_export/setup.py \
        --max_num_workers=60 \
        --num_workers=10 \
        --num_days=10 \
        --dataset=chromeperf_dashboard_rows

Unlike the other exporters in this directory, this one writes multiple tables as
a simple form of clustering by master.  E.g. if an entity read from Datastore
has a master of 'foo.bar', this will write it to a table named 'foo_bar' in the
specified dataset.
"""
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import logging

from bq_export import bq_rows

if __name__ == '__main__':
  logging.getLogger().setLevel(logging.INFO)
  bq_rows.main()
