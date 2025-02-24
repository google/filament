# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Polls the sheriff_config service."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from dashboard.sheriff_config_client import SheriffConfigClient


from flask import Response


def SheriffConfigPollerGet():
  client = SheriffConfigClient()
  ok, err_msg = client.Update()
  if not ok:
    return Response('FAILED: %s' % err_msg)
  return Response('OK')
