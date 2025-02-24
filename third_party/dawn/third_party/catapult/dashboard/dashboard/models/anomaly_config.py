# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""The database models for anomaly alerting threshold configs."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging

from google.appengine.ext import ndb

# The parameters to use from anomaly threshold config dict.
# Any parameters in such a dict that aren't in this list will be ignored.
_VALID_ANOMALY_CONFIG_PARAMETERS = {
    'max_window_size',
    'min_segment_size',
    'min_absolute_change',
    'min_relative_change',
    'min_steppiness',
    'multiple_of_std_dev',
}


class AnomalyConfig(ndb.Model):
  """Represents a set of parameters for the anomaly detection function.

  The anomaly detection module uses set of parameters to determine the
  thresholds for what is considered an anomaly.
  """
  # A dictionary mapping parameter names to values.
  config = ndb.JsonProperty(required=True, indexed=False)

  # A list of test path patterns. Each pattern is a string which can match parts
  # of the test path either exactly, or use * as a wildcard.
  # Note: TestMetadata entities contain a key property called
  # overridden_anomaly_config, which is set in the pre-put hook for TestMetadata
  # in graph_data.py.
  patterns = ndb.StringProperty(repeated=True, indexed=False)


def CleanConfigDict(config_dict):
  """Removes invalid parameters from a config dictionary.

  In the config dict there may be extra "comment" parameters which
  should be ignored. These are removed so that the parameters can
  be passed to FindChangePoints using ** notation.
  """
  return {
      key: value
      for key, value in config_dict.items()
      if key in _VALID_ANOMALY_CONFIG_PARAMETERS
  }


@ndb.synctasklet
def GetAnomalyConfigDict(test):
  """Gets the anomaly threshold config for the given test.

  Args:
    test: TestMetadata entity to get the config for.

  Returns:
    A dictionary with threshold parameters for the given test.
  """
  result = yield GetAnomalyConfigDictAsync(test)
  raise ndb.Return(result)


@ndb.tasklet
def GetAnomalyConfigDictAsync(test):
  if not test.overridden_anomaly_config:
    raise ndb.Return({})
  anomaly_config = yield test.overridden_anomaly_config.get_async()
  if not anomaly_config:
    logging.warning('No AnomalyConfig fetched from key %s for test %s',
                    test.overridden_anomaly_config, test.test_path)
    # The the overridden_anomaly_config property should be reset
    # in the pre-put hook of the TestMetadata entity.
    test.UpdateSheriff()
    test.put()
    raise ndb.Return({})
  raise ndb.Return(CleanConfigDict(anomaly_config.config))
