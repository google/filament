# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""XHR endpoint to fill in navbar fields."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json

from dashboard.common import request_handler

from flask import make_response, request


def NavbarHandlerPost():
  """XHR endpoint to fill in navbar fields."""
  template_values = {}
  request_handler.RequestHandlerGetDynamicVariables(template_values,
                                                   request.values.get('path'))
  res = make_response(
      json.dumps({
          'login_url': template_values['login_url'],
          'is_admin': template_values['is_admin'],
          'display_username': template_values['display_username'],
          'is_internal_user': template_values['is_internal_user']
      }))
  return res
