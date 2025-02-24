# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import

from flask import make_response, Blueprint

blueprint = Blueprint('health_checks', __name__)

@blueprint.route('/', methods=['GET'])
def HealthChecksGetHandler():
  """ Required for service health check """
  return make_response('Ok')
