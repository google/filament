# Copyright 2023 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Utility for handling ANGLE perf metrics, separate file as it's
# called by both the test runner and the post-processing script.

import collections
import json
import logging
import statistics


def ConvertToSkiaPerf(angle_metrics_json_files):
    grouped_results = collections.defaultdict(list)
    for fn in angle_metrics_json_files:
        with open(fn) as f:
            metrics = json.load(f)
            for group in metrics:
                for d in group:
                    k = (('suite', d['name']), ('renderer', d['backend'].lstrip('_')),
                         ('test', d['story']), ('metric', d['metric'].lstrip('.')), ('units',
                                                                                     d['units']))
                    grouped_results[k].append(float(d['value']))

    results = []
    for k, v in grouped_results.items():
        results.append({
            'key': dict(k),
            'measurements': {
                'stat': [{
                    'value': 'mean',
                    'measurement': statistics.mean(v),
                }, {
                    'value': 'stdev',
                    'measurement': statistics.stdev(v) if len(v) > 1 else 0,
                }],
            },
        })

    logging.info('angle_metrics to skia perf: %d entries' % len(results))

    return results
