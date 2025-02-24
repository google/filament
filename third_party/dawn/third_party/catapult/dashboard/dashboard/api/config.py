# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from dashboard.api import api_request_handler
from dashboard.common import namespaced_stored_object
from dashboard import revision_info_client

ALLOWLIST = [
    revision_info_client.REVISION_INFO_KEY,
]

from flask import request

def _CheckUser():
  pass

@api_request_handler.RequestHandlerDecoratorFactory(_CheckUser)
def ConfigHandlerPost():
  key = request.values.get('key')
  if key not in ALLOWLIST:
    return None
  return namespaced_stored_object.Get(key)
