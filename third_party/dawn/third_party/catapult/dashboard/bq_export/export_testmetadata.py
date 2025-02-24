# Copyright (c) 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
r"""Export chromeperf TestMetadata to BigQuery with Beam & Cloud Dataflow.

Example command line to start a Dataflow job::

  $ SVC_ACCT=bigquery-exporter@chromeperf.iam.gserviceaccount.com
  $ python bq_export/export_testmetadata.py \
        --service_account_email=$SVC_ACCT \
        --runner=DataflowRunner \
        --region=us-central1 \
        --temp_location=gs://chromeperf-dataflow-temp/ \
        --experiments=use_beam_bq_sink \
        --experiments=shuffle_mode=service \
        --setup_file=bq_export/setup.py \
        --autoscaling_algorithm=NONE \
        --num_workers=40
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import logging

from bq_export import bq_testmetadata

if __name__ == '__main__':
  logging.getLogger().setLevel(logging.INFO)
  bq_testmetadata.main()
