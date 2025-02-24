# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Dispatches requests to request handler classes."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging

from dashboard.pinpoint import handlers

from flask import Flask, request as flask_request, make_response

APP = Flask(__name__)

from google.appengine.api import wrap_wsgi_app
APP.wsgi_app = wrap_wsgi_app(APP.wsgi_app, use_deferred=True)


@APP.route('/api/jobs')
def JobsHandlerGet():
  return handlers.jobs.JobsHandlerGet()


@APP.route('/api/config', methods=['POST'])
def ConfigHandlerPost():
  return handlers.config.ConfigHandlerPost()


@APP.route('/api/commit', methods=['POST'])
def CommitHandlerPost():
  return handlers.commit.CommitHandlerPost()


@APP.route('/api/new', methods=['POST'])
def NewHandlerPost():
  return handlers.new.NewHandlerPost()


@APP.route('/api/job/<job_id>')
def JobHandlerGet(job_id):
  return handlers.job.JobHandlerGet(job_id)


@APP.route('/api/job', methods=['POST'])
def JobHandlerPost():
  return handlers.job.JobHandlerPost()


@APP.route('/api/queue-stats/<configuration>')
def QueueStatsHandlerGet(configuration):
  return handlers.queue_stats.QueueStatsHandlerGet(configuration)


@APP.route('/api/job/cancel', methods=['POST'])
def CancelHandlerPost():
  return handlers.cancel.CancelHandlerPost()


@APP.route('/api/commits', methods=['POST'])
def CommitsHandlerPost():
  return handlers.commits.CommitsHandlerPost()


@APP.route('/api/migrate', methods=['GET', 'POST'])
def MigrateHandler():
  return handlers.migrate.MigrateHandler()


# Used internally by Pinpoint. Not accessible from the public API
@APP.route('/cron/fifo-scheduler')
def FifoSchedulerHandler():
  return handlers.fifo_scheduler.FifoSchedulerHandler()


@APP.route('/cron/refresh-jobs')
def RefreshJobsHandler():
  return handlers.refresh_jobs.RefreshJobsHandler()


@APP.route('/cron/isolate-cleanup')
def IsolateCleanupHandler():
  return handlers.isolate.IsolateCleanupHandler()


@APP.route('/cron/update-culprit-verification-results')
def CulpritVerificationResultsUpdateHandler():
  return handlers.culprit.CulpritVerificationResultsUpdateHandler()


@APP.route('/api/results2/<job_id>')
def Results2Handler(job_id):
  return handlers.results2.Results2Handler(job_id)


@APP.route('/api/generate-results2/<job_id>', methods=['POST'])
def Results2GeneratorHandler(job_id):
  return handlers.results2.Results2GeneratorHandler(job_id)


@APP.route('/api/run/<job_id>', methods=['POST'])
def RunHandler(job_id):
  return handlers.run.RunHandler(job_id)


@APP.route('/api/isolate', methods=['GET', 'POST'])
def IsolateHandler():
  return handlers.isolate.IsolateHandler()


@APP.route('/api/stats')
def StatsHandler():
  return handlers.stats.StatsHandler()

@APP.route('/api/builds/<bot_config>', methods=['GET'])
def BuildsHander(bot_config):
  return handlers.builds.RecentBuildsGet(bot_config)


@APP.route('/_ah/push-handlers/task-updates', methods=['POST'])
def TaskUpdatesHandler():
  return handlers.task_updates.TaskUpdatesHandler()


# Some handlers were identified as obsolete during the python 3 migration and
# thus were deleted. Though, we want to be aware of any client calls to those
# deleted endpoints in the future by adding logs here.
@APP.route('/api/cas', endpoint='/api/cas', methods=['GET', 'POST'])
@APP.route(
    '/api/cas/<builder_name>/<git_hash>/<target>',
    endpoint='/api/cas',
    methods=['GET', 'POST'])
@APP.route(
    '/api/isolate/<builder_name>/<git_hash>/<target>', methods=['GET', 'POST'])
def ObsoleteEndpointsHandler(builder_name=None, git_hash=None, target=None):
  del builder_name, git_hash, target
  obsolete_endpoint = flask_request.endpoint
  logging.error(
      'Request on deleted endpoint: %s. '
      'It was considered obsolete in Python 3 migration.', obsolete_endpoint)

  return make_response(
      'This endpoint is obsolete: %s. '
      'Please contact browser-perf-engprod@google.com for more info.' %
      obsolete_endpoint, 404)
