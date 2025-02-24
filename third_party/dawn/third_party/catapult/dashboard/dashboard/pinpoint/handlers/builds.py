# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging

from dashboard.api import api_request_handler
from dashboard.api import api_auth
from dashboard.common import bot_configurations
from dashboard.common import cloud_metric
from dashboard.common import utils
from dashboard.services import buildbucket_service


def _CheckUser():
  if utils.IsDevAppserver():
    return
  api_auth.Authorize()
  if not utils.IsTryjobUser():
    raise api_request_handler.ForbiddenError()

BUILDER_MAPPING = {
    'Android Compile Perf': 'android-builder-perf',
    'Android Compile Perf PGO': 'android-builder-perf-pgo',
    'Android arm64 Compile Perf': 'android_arm64-builder-perf',
    'Android arm64 Compile Perf PGO': 'android_arm64-builder-perf-pgo',
    'Android arm64 High End Compile Perf': 'android_arm64_high_end-builder-perf',
    'Android arm64 High End Compile Perf PGO': 'android_arm64_high_end-builder-perf-pgo',
    'Chromecast Linux Builder Perf': 'chromecast-linux-builder-perf',
    'Chromeos Amd64 Generic Lacros Builder Perf': 'chromeos-amd64-generic-lacros-builder-perf',
    'Fuchsia Builder Perf': 'fuchsia-builder-perf-arm64',
    'Linux Builder Perf': 'linux-builder-perf',
    'Linux Builder Perf PGO': 'linux-builder-perf-pgo',
    'Mac Builder Perf': 'mac-builder-perf',
    'Mac Builder Perf PGO': 'mac-builder-perf-pgo',
    'Mac arm Builder Perf': 'mac-arm-builder-perf',
    'Mac arm Builder Perf PGO': 'mac-arm-builder-perf-pgo',
    'mac-laptop_high_end-perf': 'mac-laptop_high_end-perf',
    'Win x64 Builder Perf': 'win64-builder-perf',
    'Win x64 Builder Perf PGO': 'win64-builder-perf-pgo',
}

@api_request_handler.RequestHandlerDecoratorFactory(_CheckUser)
@cloud_metric.APIMetric("pinpoint", "/api/builds/get")
def RecentBuildsGet(bot_configuration: str):
  logging.info('Trying to get most recent builds for %s', bot_configuration)
  bot_config = bot_configurations.Get(bot_configuration)
  builder_name = BUILDER_MAPPING[bot_config['builder']]
  builds_response = buildbucket_service.GetBuilds(
    'chrome', 'ci', builder_name)
  return builds_response
