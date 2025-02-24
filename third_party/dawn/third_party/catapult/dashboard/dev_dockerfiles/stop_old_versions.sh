#!/bin/env bash
# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Usage: service_name1 service_name2 ...
# Args:
#   service_name: the service name whose older versions are to be stopped
#
# This script is to stop versions that have been deployed before the currently
# serving one and are not taking up any traffic. This frees up instances and
# addresses to be used by newer versions.
#
# This script should be only used with GAE Flex services.
#
for SERVICE in "$@"; do
  cutoffDate=$(gcloud app versions list --service=${SERVICE} \
    --filter="traffic_split=1.0" --format="table[no-heading](LAST_DEPLOYED)")
  echo "Deployment time of current serving version is $cutoffDate"
  gcloud app versions list --service=${SERVICE} \
    --format="table[no-heading](VERSION.ID)" \
    --filter="version.servingStatus='SERVING' AND \
              traffic_split=0.0 AND \
              version.createTime<${cutoffDate}" \
  | xargs --no-run-if-empty gcloud app versions stop -s ${SERVICE}
done