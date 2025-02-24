# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from flask import Flask

from application.api.dummy import dummy
from application.api.issues import issues
from application.api.alert_groups import alert_groups

def create_app():
    app = Flask(__name__)
    app.register_blueprint(dummy, url_prefix='/')
    app.register_blueprint(issues, url_prefix='/issues')
    app.register_blueprint(alert_groups, url_prefix='/alert_groups')
    return app