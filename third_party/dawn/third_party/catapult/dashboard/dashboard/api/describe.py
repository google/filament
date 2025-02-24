# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from dashboard import update_test_suite_descriptors
from dashboard.api import api_request_handler

from flask import request


def _CheckUser():
  pass


@api_request_handler.RequestHandlerDecoratorFactory(_CheckUser)
def DescribePost():
  master = request.values.get('master')
  suite = request.values.get('test_suite')
  return update_test_suite_descriptors.FetchCachedTestSuiteDescriptor(
          master, suite)
