# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import logging
import os
import re
import webapp2
import uuid

from . import trace_info
from . import cloud_config

import cloudstorage as gcs

from google.appengine.api import datastore_errors

default_retry_params = gcs.RetryParams(initial_delay=0.2,
                                       max_delay=5.0,
                                       backoff_factor=2,
                                       max_retry_period=15)
gcs.set_default_retry_params(default_retry_params)


class UploadPage(webapp2.RequestHandler):

  def get(self):
    self.response.out.write("""
          <html><body>
            <head><title>Performance Insights - Trace Uploader</title></head>
            <form action="/upload" enctype="multipart/form-data" method="post">
              <div><input type="file" name="trace"/></div>
              <div><input type="submit" value="Upload"></div>
            </form><hr>
          </body></html>""")

  def post(self):
    trace_uuid = str(uuid.uuid4())

    gcs_path = '/%s/%s.gz' % (
        cloud_config.Get().trace_upload_bucket, trace_uuid)
    gcs_file = gcs.open(gcs_path,
                        'w',
                        content_type='application/octet-stream',
                        options={},
                        retry_params=default_retry_params)
    gcs_file.write(self.request.get('trace'))
    gcs_file.close()

    trace_object = trace_info.TraceInfo(id=trace_uuid)
    trace_object.remote_addr = os.environ["REMOTE_ADDR"]

    for arg in self.request.arguments():
      arg_key = arg.replace('-', '_').lower()
      if arg_key in trace_object._properties:
        try:
          setattr(trace_object, arg_key, self.request.get(arg))
        except datastore_errors.BadValueError:
          pass

    scenario_config = self.request.get('config')
    if scenario_config:
      config_json = json.loads(scenario_config)
      if 'scenario_name' in config_json:
        trace_object.scenario_name = config_json['scenario_name']

    tags_string = self.request.get('tags')
    if tags_string:
      # Tags are comma separated and should only include alphanumeric + '-'.
      if re.match('^[a-zA-Z0-9-,]+$', tags_string):
        trace_object.tags = tags_string.split(',')
      else:
        logging.warning('The provided tags string includes one or more invalid'
                        ' characters and will be ignored')

    trace_object.ver = self.request.get('product-version')
    trace_object.put()

    self.response.write(trace_uuid)

app = webapp2.WSGIApplication([('/upload', UploadPage)])
