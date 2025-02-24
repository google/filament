# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
import logging

from flask import request

from google.appengine.api import datastore_errors
from google.appengine.datastore import datastore_query
from google.appengine.ext import deferred

from dashboard.api import api_auth
from dashboard.api import api_request_handler
from dashboard.common import stored_object
from dashboard.common import utils
from dashboard.pinpoint.models import job

_BATCH_SIZE = 50
_STATUS_KEY = 'job_migration_status'


def _CheckUser():
  if utils.IsDevAppserver():
    return
  api_auth.Authorize()
  if not utils.IsAdministrator():
    raise api_request_handler.ForbiddenError()


@api_request_handler.RequestHandlerDecoratorFactory(_CheckUser)
def MigrateHandler():
  if request.method == 'GET':
    return stored_object.Get(_STATUS_KEY) or {}

  if request.method == 'POST':
    status = stored_object.Get(_STATUS_KEY)

    if not status:
      _Start()
    return stored_object.Get(_STATUS_KEY) or {}
  return {}


def _Start():
  query = job.Job.query(job.Job.task == None)
  status = {
      'count': 0,
      'started': datetime.datetime.now().isoformat(),
      'total': query.count(),
      'errors': 0,
  }
  stored_object.Set(_STATUS_KEY, status)
  deferred.defer(_Migrate, status, None)


def _Migrate(status, cursor=None):
  if cursor:
    cursor = datastore_query.Cursor(urlsafe=cursor)
  query = job.Job.query(job.Job.task == None)
  jobs, next_cursor, more = query.fetch_page(_BATCH_SIZE, start_cursor=cursor)

  # Because individual job instances might fail to be persisted for some reason
  # (e.g. entities exceeding the entity size limit) we'll perform the updates
  # one at a time. This is not an ideal state, since we'll want to be able to
  # migrate all jobs to an alternative structure in the future, but we recognise
  # that partial success is better than total failure.
  for j in jobs:
    try:
      j.put()
      status['count'] += 1
    except datastore_errors.Error as e:
      logging.error('Failed migrating job %s: %s', j.job_id, e)
      status['errors'] += 1

  if more:
    stored_object.Set(_STATUS_KEY, status)
    deferred.defer(_Migrate, status, next_cursor.urlsafe())
  else:
    stored_object.Set(_STATUS_KEY, None)
