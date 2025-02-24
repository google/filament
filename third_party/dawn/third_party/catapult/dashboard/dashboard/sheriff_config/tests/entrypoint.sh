#!/bin/bash
# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

wait-port() {
  timeout $1 bash -c \
    'until printf "" 2>>/dev/null >>/dev/tcp/$0; do sleep 1; done' \
    $(echo $2 | tr : /)
}

# waiting datastore
wait-port 30 ${DATASTORE_EMULATOR_HOST}
"$@"