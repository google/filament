# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Provides a layer of abstraction for the chromeperf API."""

from http import client
import json
import logging

from application import utils

if utils.IsStagingEnvironment():
  CHROMEPERF_HOST = 'https://chromeperf-stage.uc.r.appspot.com/'
else:
  CHROMEPERF_HOST = 'https://chromeperf.appspot.com/'


class RequestError(client.HTTPException):

  def __init__(self, msg, headers, content):
    super().__init__(msg)
    self.headers = headers
    self.content = content


class NotFoundError(RequestError):
  """Raised when a request gives a HTTP 404 error."""


def GetBuganizerProjects():
  url = CHROMEPERF_HOST + 'edit_site_config?key=buganizer_projects&format=json'

  http = utils.ServiceAccountHttp()
  response, content = http.request(url)

  status = response.get('status', 200)
  if status == '404':
    logging.debug('Response headers: %s, body: %s', response, content)
    raise NotFoundError(
        'HTTP status code %s: %s' % (status, repr(content[0:200])),
        response, content)
  if not status.startswith('2'):
    logging.debug('Response headers: %s, body: %s', response, content)
    raise RequestError(
        'Failure in request for `%s`; HTTP status code %s: %s' %
        (url, status, repr(content[0:200])), response, content)

  all_values = json.loads(content)

  return json.loads(all_values['value'])
