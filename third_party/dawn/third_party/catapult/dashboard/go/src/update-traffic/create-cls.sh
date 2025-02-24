# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#!/bin/sh

# Exit early if any of the commands fails
set -e

if [[ -n $(git status -s) ]]; then
  echo 'There are uncommitted changes, please stash them before proceeding.'
  exit 1
fi

if [[ $# -ne 1 ]] ; then
  echo "specify a deployment umbrella issue ID on the command line, e.g.:"
  echo "$0 <issue id>"
  exit 1
fi

branch_suffix='-deploy'
declare -a services=("default"
                        "api"
                        "pinpoint"
                        "upload"
                        "upload-processing"
                        "skia-bridge"
                        "perf-issue-service")

git fetch origin
for service in "${services[@]}"; do
    branch_name="${service}${branch_suffix}"
    git branch -D "${branch_name}" || true
    git checkout -b "$branch_name" -t origin/main
    git pull --ff
    go run update-traffic.go -checkout-base ../../../.. -service-id $service
    if git diff-index --quiet HEAD --; then
      echo "service ($service) is already up-to-date, skipping."
    else
      git commit -am "update chromeperf deployment for $service"
      git cl upload -b $1
    fi
done;
