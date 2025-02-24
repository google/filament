#!/bin/bash
# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e

init_catapult() {
  # The current catapult deploy process & unit testing will generate
  # files in the directory. To prevent making any changes to the mounted
  # directory, we can just make a copy instead.
  if [[ -e /image/workspace ]]; then
    cp -r /image/workspace /workspace
  fi
  # In some case we want change the content on disk. So /workspace may be
  # direcetly mounted.
  if [[ -e /workspace ]]; then
    # TODO(dberris): This is a hack, which really shouldn't be required if we
    # remove the requirement that the deployment script be running in a git
    # repository.
    mkdir -p /workspace/.git/hooks
    pushd /workspace/dashboard
    make clean && make
    popd
  fi
}

set_user_email() {
  # We only need to set the user email when both /image/workspace exist and
  # gcloud account configured. Because in other cases we don't neet to deploy
  # the service (gcloud not authed or without code)
  email=$(gcloud config get-value account 2>/dev/null)
  if [[ -e /workspace ]] && ! [[ -z "${email}" ]]; then
    pushd /workspace
    git config --add user.email "${email}"
    popd
  fi
}

init_catapult
set_user_email
exec "$@"