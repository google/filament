# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from flask import make_response, Blueprint

dummy = Blueprint('dummy', __name__)

@dummy.route('/')
def DummyHandler():
  return make_response('welcome')