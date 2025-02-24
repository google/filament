# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Dispatches requests to request handler classes."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from flask import Flask, request as flask_request, make_response
import logging

from google.appengine.api import wrap_wsgi_app
import google.cloud.logging
try:
  import googleclouddebugger
  googleclouddebugger.enable(breakpoint_enable_canary=True)
except ImportError:
  pass

from dashboard import add_histograms
from dashboard import add_histograms_queue
from dashboard import add_point
from dashboard import add_point_queue
from dashboard import alert_groups
from dashboard import alerts
from dashboard import associate_alerts
from dashboard import buildbucket_job_status
from dashboard import dump_graph_json
from dashboard import edit_anomalies
from dashboard import edit_site_config
from dashboard import file_bug
from dashboard import graph_csv
from dashboard import graph_json
from dashboard import graph_revisions
from dashboard import group_report
from dashboard import layered_cache_delete_expired
from dashboard import list_tests
from dashboard import load_from_prod
from dashboard import main
from dashboard import migrate_test_names
from dashboard import migrate_test_names_tasks
from dashboard import mark_recovered_alerts
from dashboard import navbar
from dashboard import pinpoint_request
from dashboard import report
from dashboard import sheriff_config_poller
from dashboard import short_uri
from dashboard import update_dashboard_stats
from dashboard import update_test_suites
from dashboard import update_test_suite_descriptors
from dashboard import uploads_info
from dashboard.api import alerts as api_alerts
from dashboard.api import config
from dashboard.api import describe
from dashboard.api import test_suites
from dashboard.api import timeseries2
from dashboard.common import datastore_hooks

google.cloud.logging.Client().setup_logging(log_level=logging.DEBUG)
logging.getLogger("urllib3").setLevel(logging.INFO)

datastore_hooks.InstallHooks()

flask_app = Flask(__name__)

flask_app.wsgi_app = wrap_wsgi_app(flask_app.wsgi_app, use_deferred=True)


@flask_app.route('/')
def MainHandlerGet():
  return main.MainHandlerGet()


@flask_app.route('/add_histograms', methods=['POST'])
def AddHistogramsPost():
  return add_histograms.AddHistogramsPost()


@flask_app.route('/add_histograms/process', methods=['POST'])
def AddHistogramsProcessPost():
  return add_histograms.AddHistogramsProcessPost()


@flask_app.route('/add_histograms_queue', methods=['GET', 'POST'])
def AddHistogramsQueuePost():
  return add_histograms_queue.AddHistogramsQueuePost()


@flask_app.route('/add_point', methods=['POST'])
def AddPointPost():
  return add_point.AddPointPost()


@flask_app.route('/add_point_queue', methods=['GET', 'POST'])
def AddPointQueuePost():
  return add_point_queue.AddPointQueuePost()


@flask_app.route('/alert_groups_update')
def AlertGroupsGet():
  return alert_groups.AlertGroupsGet()


@flask_app.route('/alerts', methods=['GET'])
def AlertsHandlerGet():
  return alerts.AlertsHandlerGet()


@flask_app.route('/alerts', methods=['POST'])
def AlertsHandlerPost():
  return alerts.AlertsHandlerPost()


@flask_app.route('/associate_alerts', methods=['GET', 'POST'])
def AssociateAlertsHandlerPost():
  return associate_alerts.AssociateAlertsHandlerPost()


@flask_app.route('/api/alerts', methods=['POST', 'OPTIONS'])
def AlertsPost():
  return api_alerts.AlertsPost()


@flask_app.route('/api/config', methods=['POST'])
def ConfigHandlerPost():
  return config.ConfigHandlerPost()


@flask_app.route('/api/describe', methods=['POST', 'OPTIONS'])
def DescribePost():
  return describe.DescribePost()


@flask_app.route('/api/test_suites', methods=['POST', 'OPTIONS'])
def TestSuitesPost():
  return test_suites.TestSuitesPost()


@flask_app.route('/api/timeseries2', methods=['POST'])
def TimeSeries2Post():
  return timeseries2.TimeSeries2Post()


@flask_app.route('/buildbucket_job_status/<job_id>')
def BuildbucketJobStatusGet(job_id):
  return buildbucket_job_status.BuildbucketJobStatusGet(job_id)


@flask_app.route('/delete_expired_entities')
def LayeredCacheDeleteExpiredGet():
  return layered_cache_delete_expired.LayeredCacheDeleteExpiredGet()


@flask_app.route('/dump_graph_json', methods=['GET'])
def DumpGraphJsonHandler():
  return dump_graph_json.DumpGraphJsonHandlerGet()


@flask_app.route('/edit_anomalies', methods=['POST'])
def EditAnomaliesPost():
  return edit_anomalies.EditAnomaliesPost()


@flask_app.route('/edit_site_config', methods=['GET'])
def EditSiteConfigHandlerGet():
  return edit_site_config.EditSiteConfigHandlerGet()


@flask_app.route('/edit_site_config', methods=['POST'])
def EditSiteConfigHandlerPost():
  return edit_site_config.EditSiteConfigHandlerPost()


@flask_app.route('/file_bug', methods=['GET', 'POST'])
def FileBugHandlerGet():
  return file_bug.FileBugHandlerGet()


@flask_app.route('/graph_csv', methods=['GET'])
def GraphCSVHandlerGet():
  return graph_csv.GraphCSVGet()


@flask_app.route('/graph_csv', methods=['POST'])
def GraphCSVHandlerPost():
  return graph_csv.GraphCSVPost()


@flask_app.route('/graph_json', methods=['POST'])
def GraphJsonPost():
  return graph_json.GraphJsonPost()


@flask_app.route('/graph_revisions', methods=['POST'])
def GraphRevisionsPost():
  return graph_revisions.GraphRevisionsPost()


@flask_app.route('/group_report', methods=['GET'])
def GroupReportGet():
  return group_report.GroupReportGet()


@flask_app.route('/group_report', methods=['POST'])
def GroupReportPost():
  return group_report.GroupReportPost()


@flask_app.route('/list_tests', methods=['POST'])
def ListTestsHandlerPost():
  return list_tests.ListTestsHandlerPost()


@flask_app.route('/load_from_prod', methods=['GET', 'POST'])
def LoadFromProdHandler():
  return load_from_prod.LoadFromProdHandlerGetPost()


@flask_app.route('/mark_recovered_alerts', methods=['GET', 'POST'])
def MarkRecoveredAlertsPost():
  return mark_recovered_alerts.MarkRecoveredAlertsPost()


@flask_app.route('/migrate_test_names', methods=['GET'])
def MigrateTestNamesGet():
  return migrate_test_names.MigrateTestNamesGet()


@flask_app.route('/migrate_test_names', methods=['POST'])
def MigrateTestNamesPost():
  return migrate_test_names.MigrateTestNamesPost()


@flask_app.route('/migrate_test_names_tasks', methods=['POST'])
def MigrateTestNamesTasksPost():
  return migrate_test_names_tasks.MigrateTestNamesTasksPost()


@flask_app.route('/navbar', methods=['POST'])
def NavbarHandlerPost():
  return navbar.NavbarHandlerPost()


@flask_app.route('/pinpoint/new/bisect', methods=['POST'])
def PinpointNewBisectPost():
  return pinpoint_request.PinpointNewBisectPost()


@flask_app.route('/pinpoint/new/perf_try', methods=['POST'])
def PinpointNewPerfTryPost():
  return pinpoint_request.PinpointNewPerfTryPost()


@flask_app.route('/pinpoint/new/prefill', methods=['POST'])
def PinpointNewPrefillPost():
  return pinpoint_request.PinpointNewPrefillPost()


@flask_app.route('/configs/update')
def SheriffConfigPollerGet():
  return sheriff_config_poller.SheriffConfigPollerGet()


@flask_app.route('/report', methods=['GET'])
def ReportHandlerGet():
  return report.ReportHandlerGet()


@flask_app.route('/report', methods=['POST'])
def ReportHandlerPost():
  return report.ReportHandlerPost()


@flask_app.route('/short_uri', methods=['GET'])
def ShortUriHandlerGet():
  return short_uri.ShortUriHandlerGet()


@flask_app.route('/short_uri', methods=['POST'])
def ShortUriHandlerPost():
  return short_uri.ShortUriHandlerPost()


@flask_app.route('/update_dashboard_stats')
def UpdateDashboardStatsGet():
  return update_dashboard_stats.UpdateDashboardStatsGet()


@flask_app.route('/update_test_suites', methods=['GET','POST'])
def UpdateTestSuitesPost():
  return update_test_suites.UpdateTestSuitesPost()


@flask_app.route('/update_test_suite_descriptors', methods=['GET', 'POST'])
def UpdateTestSuitesDescriptorsPost():
  return update_test_suite_descriptors.UpdateTestSuiteDescriptorsPost()


@flask_app.route('/uploads/<token_id>')
def UploadsInfoGet(token_id):
  return uploads_info.UploadsInfoGet(token_id)


# Some handlers were identified as obsolete during the python 3 migration and
# thus were deleted. Though, we want to be aware of any client calls to those
# deleted endpoints in the future by adding logs here.
@flask_app.route(
    '/bug_details', endpoint='/bug_details', methods=['GET', 'POST'])
@flask_app.route(
    '/create_health_report',
    endpoint='/create_health_report',
    methods=['GET', 'POST'])
@flask_app.route(
    '/get_diagnostics', endpoint='/get_diagnostics', methods=['POST'])
@flask_app.route('/get_histogram', endpoint='/get_histogram', methods=['POST'])
@flask_app.route(
    '/put_entities_task', endpoint='/put_entities_task', methods=['POST'])
@flask_app.route(
    '/speed_releasing', endpoint='/speed_releasing', methods=['GET', 'POST'])
@flask_app.route('/api/bugs/<bug_id>', endpoint='/api/bugs', methods=['POST'])
@flask_app.route(
    '/api/bugs/p/<bug_id>/<project_id>',
    endpoint='/api/bugs/p',
    methods=['POST'])
@flask_app.route(
    '/api/existing_bug', endpoint='/api/existing_bug', methods=['POST'])
@flask_app.route(
    '/api/list_timeseries/', endpoint='/api/list_timeseries', methods=['POST'])
@flask_app.route('/api/new_bug', endpoint='/api/new_bug', methods=['POST'])
@flask_app.route(
    '/api/new_pinpoint', endpoint='/api/new_pinpoint', methods=['POST'])
@flask_app.route(
    '/api/nudge_alert', endpoint='/api/nudge_alert', methods=['POST'])
@flask_app.route(
    '/api/report/generate', endpoint='/api/report/generate', methods=['POST'])
@flask_app.route(
    '/api/report/names', endpoint='/api/report/names', methods=['POST'])
@flask_app.route(
    '/api/report/template', endpoint='/api/report/template', methods=['POST'])
@flask_app.route(
    '/api/timeseries/', endpoint='/api/timeseries', methods=['POST'])
def ObsoleteEndpointsHandler(bug_id=None, project_id=None):
  del bug_id, project_id
  obsolete_endpoint = flask_request.endpoint
  logging.error(
      'Request on deleted endpoint: %s. '
      'It was considered obsolete in Python 3 migration.', obsolete_endpoint)

  return make_response(
      'This endpoint is obsolete: %s. '
      'Please contact browser-perf-engprod@google.com for more info.' %
      obsolete_endpoint, 404)


def APP(environ, request):
  return flask_app(environ, request)
