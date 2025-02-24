# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from dashboard.api import api_request_handler
from dashboard.common import bot_configurations


def _CheckUser():
  pass


@api_request_handler.RequestHandlerDecoratorFactory(_CheckUser)
def ConfigHandlerPost():
  return {'configurations': bot_configurations.List()}
