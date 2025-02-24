# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Defines common functionality used for generating e-mail to sheriffs."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging
import re
import six
import six.moves.urllib.parse

from google.appengine.api import urlfetch

from dashboard.common import utils

_SINGLE_EMAIL_SUBJECT = ('%(percent_changed)s %(change_type)s in %(test_name)s '
                         'on %(bot)s at %(start)d:%(end)d')

_EMAIL_HTML_TABLE = """
A <b>%(percent_changed)s</b> %(change_type)s.<br>
<table cellpadding='4'>
  <tr><td>Master:</td><td><b>%(master)s</b></td>
  <tr><td>Bot:</td><td><b>%(bot)s</b></td>
  <tr><td>Test:</td><td><b>%(test_name)s</b></td>
  <tr><td>Revision Range:</td><td><b>%(start)d - %(end)d</b></td>
</table><p>
<a href="%(triage_url)s">View the graph</a> to triage.<br>
<br>
"""

_SUMMARY_EMAIL_TEXT_BODY = """
A %(percent_changed)s %(change_type)s:

Master: %(master)s
Bot: %(bot)s
Test: %(test_name)s
Revision Range:%(start)d - %(end)d

View the graph: %(triage_url)s
+++++++++++++++++++++++++++++++
"""

_PERF_TRY_EMAIL_SUBJECT = (
    'Perf Try %(status)s on %(bot)s at %(start)s:%(end)s.')

_PERF_TRY_EMAIL_HTML_BODY = """
Perf Try Job %(status)s
<br><br>
%(warnings)s
A Perf Try Job was submitted on %(bot)s at
<a href="%(perf_url)s">%(perf_url)s</a>.<br>
<table cellpadding='4'>
  <tr><td>Bot:</td><td><b>%(bot)s</b></td>
  <tr><td>Test:</td><td><b>%(command)s</b></td>
  <tr><td>Revision Range:</td><td><b>%(start)s - %(end)s</b></td>
  <tr><td>HTML Results:</td><td><b>%(html_results)s</b></td>
  %(profiler_results)s
</table><p>
"""

_PERF_TRY_EMAIL_TEXT_BODY = """
Perf Try Job %(status)s
%(warnings)s
Bot: %(bot)s
Test: %(command)s
Revision Range:%(start)s - %(end)s

HTML Results: %(html_results)s
%(profiler_results)s
+++++++++++++++++++++++++++++++
"""

_PERF_PROFILER_HTML_ROW = '<tr><td>%(title)s:</td><td><b>%(link)s</b></td>\n'
_PERF_PROFILER_TEXT_ROW = '%(title)s: %(link)s\n'


def GetPerfTryJobEmailReport(try_job_entity):
  """Gets the contents of the email to send once a perf try job completes."""
  results_data = try_job_entity.results_data
  config = try_job_entity.GetConfigDict()
  if results_data['status'] == 'completed':
    profiler_html_links = ''
    profiler_text_links = ''
    for link_dict in results_data['profiler_links']:
      profiler_html_links += _PERF_PROFILER_HTML_ROW % link_dict
      profiler_text_links += _PERF_PROFILER_TEXT_ROW % link_dict
    subject_dict = {
        'status': 'Success',
        'bot': results_data['bisect_bot'],
        'start': config['good_revision'],
        'end': config['bad_revision']
    }
    html_dict = {
        'status': 'SUCCESS',
        'bot': results_data['bisect_bot'],
        'perf_url': results_data['buildbot_log_url'],
        'command': config['command'],
        'start': config['good_revision'],
        'end': config['bad_revision'],
        'html_results': results_data['cloud_link'],
        'profiler_results': profiler_html_links,
    }
    if results_data.get('warnings'):
      html_dict['warnings'] = ','.join(results_data['warnings'])
    text_dict = html_dict.copy()
    text_dict['profiler_results'] = profiler_text_links
  elif results_data['status'] == 'failed':
    if not config:
      config = {
          'good_revision': '?',
          'bad_revision': '?',
          'command': '?',
      }
    subject_dict = {
        'status': 'Failure',
        'bot': results_data['bisect_bot'],
        'start': config['good_revision'],
        'end': config['bad_revision']
    }
    html_dict = {
        'status': 'FAILURE',
        'bot': results_data['bisect_bot'],
        'perf_url': results_data['buildbot_log_url'],
        'command': config['command'],
        'start': config['good_revision'],
        'end': config['bad_revision'],
        'html_results': '',
        'profiler_results': '',
    }
    text_dict = html_dict
  else:
    return None

  html = _PERF_TRY_EMAIL_HTML_BODY % html_dict
  text = _PERF_TRY_EMAIL_TEXT_BODY % text_dict
  subject = _PERF_TRY_EMAIL_SUBJECT % subject_dict
  return {'subject': subject, 'html': html, 'body': text}


def GetSubscriptionEmails(subscriptions):
  """Gets all of the email addresses to send mail to for a Sheriff.

  This includes both the general email address of the subscription rotation,
  which is often a mailing list, and the email address of the particular
  subscription on duty, if applicable.

  Args:
    subscriptions: Subscription entities.

  Returns:
    A comma-separated list of email addresses; this will be an empty string
    if there are no email addresses.
  """
  receivers = [
      s.notification_email for s in subscriptions if s.notification_email
  ]
  for s in subscriptions:
    emails = _GetSheriffOnDutyEmail(s)
    if emails:
      receivers.append(emails)
  return ','.join(receivers)


def _GetSheriffOnDutyEmail(subscription):
  """Gets the email address of the subscription on duty for a rotation.

  Args:
    subscription: A Subscription entity.

  Returns:
    A comma-separated list of email addresses, or None.
  """
  if not subscription.rotation_url:
    return None
  response = urlfetch.fetch(subscription.rotation_url)
  if response.status_code != 200:
    logging.error('Response %d from %s for %s.', response.status_code,
                  subscription.rotation_url, subscription.name)
    return None
  match = re.match(r'document\.write\(\'(.*)\'\)',
                   six.ensure_str(response.content))
  if not match:
    logging.error('Could not parse response from subscription URL %s: %s',
                  subscription.rotation_url, response.content)
    return None
  addresses = match.groups()[0].split(', ')
  return ','.join('%s@google.com' % a for a in addresses)


def GetReportPageLink(test_path, rev=None, add_protocol_and_host=True):
  """Gets a URL for viewing a single graph."""
  path_parts = test_path.split('/')
  if len(path_parts) < 4:
    logging.error('Could not make link, invalid test path: %s', test_path)
  master, bot = path_parts[0], path_parts[1]
  test_name = '/'.join(path_parts[2:])
  trace_name = path_parts[-1]
  trace_names = ','.join([trace_name, trace_name + '_ref', 'ref'])
  if add_protocol_and_host:
    link_template = 'https://chromeperf.appspot.com/report?%s'
  else:
    link_template = '/report?%s'
  uri = link_template % six.moves.urllib.parse.urlencode([
      ('masters', master),
      ('bots', bot),
      ('tests', test_name),
      ('checked', trace_names),
  ])
  if rev:
    uri += '&rev=%s' % rev
  return uri


def GetGroupReportPageLink(alert):
  """Gets a URL for viewing a graph for an alert and possibly related alerts."""
  # Entities only have a key if they have already been put().
  if alert and alert.key:
    link_template = 'https://chromeperf.appspot.com/group_report?keys=%s'
    return link_template % six.ensure_str(alert.key.urlsafe())
  # If we can't make the above link, fall back to the /report page.
  test_path = utils.TestPath(alert.GetTestMetadataKey())
  return GetReportPageLink(test_path, rev=alert.end_revision)


def GetAlertInfo(alert, test):
  """Gets the alert info formatted for the given alert and test.

  Args:
    alert: An Anomaly entity.
    test: The TestMetadata entity for the given alert.

  Returns:
    A dictionary of string keys to values. Keys are 'email_subject',
    'email_text', 'email_html', 'dashboard_link', 'bug_link'.
  """
  percent_changed = alert.GetDisplayPercentChanged()
  change_type = 'improvement' if alert.is_improvement else 'regression'
  test_name = '/'.join(test.test_path.split('/')[2:])
  subscription_names = ','.join(alert.subscription_names)
  master = test.master_name
  bot = test.bot_name

  triage_url = GetGroupReportPageLink(alert)

  # Parameters to interpolate into strings below.
  interpolation_parameters = {
      'percent_changed': percent_changed,
      'change_type': change_type,
      'master': master,
      'bot': bot,
      'test_name': test_name,
      'sheriff_name': subscription_names,
      'start': alert.start_revision,
      'end': alert.end_revision,
      'triage_url': triage_url,
  }

  results = {
      'email_subject': _SINGLE_EMAIL_SUBJECT % interpolation_parameters,
      'email_text': _SUMMARY_EMAIL_TEXT_BODY % interpolation_parameters,
      'email_html': _EMAIL_HTML_TABLE % interpolation_parameters,
      'dashboard_link': triage_url,
  }
  return results
