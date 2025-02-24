# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import print_function
import datetime
import json
import re
import six.moves.urllib.request
import six.moves.urllib.parse
import six.moves.urllib.error

# pylint: disable=import-error
import httplib2
from oauth2client import service_account  # pylint: disable=no-name-in-module
from six.moves import range
# pylint: enable=import-error


# Update this to the location you downloaded the keyfile to.
_PATH_TO_JSON_KEYFILE = 'PATH/TO/keyfile.json'

def GetAlerts(num_days, benchmark=None):
  min_timestamp = datetime.datetime.now() - datetime.timedelta(days=num_days)
  options = {'min_timestamp': min_timestamp.isoformat()}
  if benchmark:
    options['test_suite'] = benchmark
  url = '/api/alerts?' + six.moves.urllib.parse.urlencode(options)
  return _MakeDashboardApiRequest(url)['anomalies']

def GetBug(bug_id):
  url = '/api/bugs/%d' % bug_id
  print('Fetching bug %s' % bug_id)
  bug = _MakeDashboardApiRequest(url)['bug']
  _ParseBisectsFromBugComments(bug)
  return bug

def GetTimeseriesList(benchmark):
  url = '/api/list_timeseries/%s' % benchmark
  return _MakeDashboardApiRequest(url)

def GetTimeseries(test_path, num_days):
  url = '/api/timeseries/%s' % test_path
  return  _MakeDashboardApiRequest(url, 'num_days=%s' % num_days)

def _ParseBisectsFromBugComments(bug):
  """Parse the bisect data out of bug comments.

  Some bisects are currently missing, see
  https://github.com/catapult-project/catapult/issues/3702
  For now, we can scrub bug comments to ensure we have the
  full list of bisects.
  """

  # First make a dict keyed by the unique bisect try job links,
  # Mapping to known status and command used.
  bisect_statuses = {}
  bisect_commands = {}
  bisect_metrics = {}
  for bisect in bug['legacy_bisects']:
    key = bisect['buildbucket_link']
    if bisect['culprit']:
      bisect_statuses[key] = 'success'
    elif bisect['status'] == 'started':
      bisect_statuses[key] = 'started'
    else:
      # Below, we will go through bug comments to update the 'no-repro' ones.
      bisect_statuses[key] = 'failed'

  # Now go through the bug comments to find bisects that weren't
  # in the list, and also update statuses to no-repro if needed.
  for comment in bug['comments']:
    comment = comment['content']
    url = _GetBuildbucketLinkFromBugComment(comment)
    if not url:
      continue
    command = _GetBisectCommandFromBugComment(comment)
    if command:
      bisect_commands[url] = command
    metric = _GetBisectMetricFromBugComment(comment)
    if metric:
      bisect_metrics[url] = metric
    if _IsNoReproComment(comment):
      bisect_statuses[url] = 'no-repro'
    elif url not in bisect_statuses and 'Started bisect job' in comment:
      # Found a bisect job start not seen above, set it to 'started', and its
      # status will be updated if there is another comment about it failing.
      bisect_statuses[url] = 'started'
    elif bisect_statuses.get(url) != 'success':
      # Bisects that weren't reported as having culprits above are failed.
      # This overwrites some 'started' bisects that the dashboard never
      # updated on bug, as well as weird statuses like 'staled'.
      bisect_statuses[url] = 'failed'

  # Now go through all the bisects and ensure they are in the bug's list and
  # the status is set correctly.
  for url in bisect_statuses:
    index = _GetIndexOfBisectUrl(bug, url)
    if index == -1:
      bug['legacy_bisects'].append({
          'status': bisect_statuses[url],
          'buildbucket_link': url,
          'command': bisect_commands.get(url, 'unknown'),
          'bug_id': bug['id']
      })
    else:
      bug['legacy_bisects'][index]['status'] = bisect_statuses[url]
  return bug

def _GetBuildbucketLinkFromBugComment(comment):
  match = re.search(
      r'https://chromeperf.appspot.com/buildbucket_job_status/[\d]+', comment)
  if not match:
    return None
  return match.group(0)

def _GetBisectCommandFromBugComment(comment):
  match = re.search(r'To Run This Test\n+\s+(.*)\n', comment)
  if not match:
    return None
  return match.group(1)

def _GetBisectMetricFromBugComment(comment):
  match = re.search(r'Metric       : (.*)\n', comment)
  if not match:
    return None
  return match.group(1)

def _IsNoReproComment(comment):
  if ('NO Perf regression found' in comment and
      not 'tests failed to produce values' in comment):
    return True
  if ('Perf regression found but unable to narrow commit range' in comment and
      not 'build failures' in comment):
    return True
  if 'NO Test failure found' in comment:
    return True
  if ('Perf regression found but unable to continue' in comment and
      'Bisect was stopped because a commit' in comment):
    return True
  return False

def _GetIndexOfBisectUrl(bug, url):
  for i in range(0, len(bug['legacy_bisects'])):
    if bug['legacy_bisects'][i]['buildbucket_link'] == url:
      return i
  return -1

def _MakeDashboardApiRequest(path, postdata=None):
  print('Requesting: %s' % path)
  url = 'https://chromeperf.appspot.com' + path
  scopes = ['https://www.googleapis.com/auth/userinfo.email']

  # Need to update the path here.
  creds = service_account.ServiceAccountCredentials.from_json_keyfile_name(
      _PATH_TO_JSON_KEYFILE, scopes)

  http_auth = creds.authorize(httplib2.Http())
  response, content = http_auth.request(
      url,
      'POST',
      postdata,
      headers={'Content-length': len(postdata or '')})
  if str(response['status']) != '200':
    print('Received %s response from %s' % (response['status'], url))
  return json.loads(content)
