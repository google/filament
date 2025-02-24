# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
import functools

from dashboard.common import report_query
from dashboard.models import report_template
from dashboard import update_test_suite_descriptors

MEMORY_METRICS = [('Java Heap', 'system_memory:java_heap'),
                  ('Native Heap', 'system_memory:native_heap'),
                  ('Android Graphics', 'gpu_memory'),
                  ('Overall PSS', 'system_memory')]

STARTUP_BY_BROWSER = {
    'chrome': {
        'testSuites': ['start_with_url.cold.startup_pages'],
        'measurement': 'foreground_tab_request_start',
        'testCases': ['http___bbc.co.uk']
    },
    'webview': {
        'testSuites': ['system_health.webview_startup'],
        'measurement': 'webview_startup_wall_time_avg',
        'testCases': ['load:chrome:blank']
    }
}


def MemoizeWithTimeout(**kwargs):
  """Memoize the value returned by a function that takes no arguments."""
  duration = datetime.timedelta(**kwargs)

  def Decorator(f):
    f._value = None
    f._expires_at = None

    @functools.wraps(f)
    def Replacement():
      if f._expires_at is None or datetime.datetime.utcnow() > f._expires_at:
        f._value = f()
        f._expires_at = datetime.datetime.utcnow() + duration
      return f._value

    return Replacement

  return Decorator


@MemoizeWithTimeout(hours=12)
def GetSystemHealthDescriptors():
  return update_test_suite_descriptors.FetchCachedTestSuiteDescriptor(
      None, 'system_health.memory_mobile')


def IterTemplateRows(browser, bot):
  descriptors = GetSystemHealthDescriptors()
  test_cases = descriptors['caseTags']['health_check']

  # Startup.
  yield dict(STARTUP_BY_BROWSER[browser], label='Startup:Time', bots=[bot])

  # Memory.
  if bot == 'ChromiumPerf:android-pixel2_webview-perf':
    # The pixel2 webview bot incorrectly reports memory as if coming from
    # chrome. TODO(crbug.com/972620): Remove this when bug is fixed.
    browser = 'chrome'
  for label, component in MEMORY_METRICS:
    yield {
        'label':
            ':'.join(['Memory', label]),
        'testSuites': ['system_health.memory_mobile'],
        'bots': [bot],
        'measurement':
            ':'.join([
                'memory', browser, 'all_processes:reported_by_os', component,
                'proportional_resident_size'
            ]),
        'testCases':
            test_cases
    }

  # CPU.
  yield {
      'label': 'CPU:Time Percentage',
      'testSuites': ['system_health.common_mobile'],
      'bots': [bot],
      'measurement': 'cpu_time_percentage',
      'testCases': test_cases
  }


def CreateSystemHealthReport(template_id, name, builder, is_fyi, modified):
  # The browser (Chrome/WebView) is always the second part of the report name,
  # and is used to build the right template.
  browser = name.split(':')[1].lower()
  master = 'ChromiumPerfFyi' if is_fyi else 'ChromiumPerf'
  bot = ':'.join([master, builder])

  @report_template.Static(
      template_id=template_id,
      name=name,
      internal_only=False,
      modified=modified)
  def Report(revisions):
    # Template is updated on each call to the handler to make sure that we use
    # an up to date set of foreground/background stories.
    template = {
        'rows': list(IterTemplateRows(browser, bot)),
        'statistics': ['avg', 'std', 'max'],
        'url': 'https://bit.ly/system-health-benchmarks'
    }
    return report_query.ReportQuery(template, revisions)

  return Report


CreateSystemHealthReport(
    template_id=2013652838,
    name='Health:Chrome:Android Go',
    builder='android-go-perf',
    is_fyi=False,
    modified=datetime.datetime(2019, 3, 22))

CreateSystemHealthReport(
    template_id=434658613,
    name='Health:Chrome:Pixel 2',
    builder='android-pixel2-perf',
    is_fyi=False,
    modified=datetime.datetime(2019, 3, 22))

CreateSystemHealthReport(
    template_id=1371943537,
    name='Health:WebView:Android Go',
    builder='android-go_webview-perf',
    is_fyi=False,
    modified=datetime.datetime(2019, 3, 22))

CreateSystemHealthReport(
    template_id=191176182,
    name='Health:WebView:Pixel 2',
    builder='android-pixel2_webview-perf',
    is_fyi=False,
    modified=datetime.datetime(2019, 3, 22))
