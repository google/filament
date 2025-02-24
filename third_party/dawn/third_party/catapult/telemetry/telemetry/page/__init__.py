# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import inspect
import logging
import os
import six.moves.urllib.parse # pylint: disable=import-error

from py_utils import cloud_storage  # pylint: disable=import-error

from telemetry import story
from telemetry.page import cache_temperature as cache_temperature_module
from telemetry.page import shared_page_state
from telemetry.page import traffic_setting as traffic_setting_module
from telemetry.internal.actions import action_runner as action_runner_module


def cmp(a, b):
  return int(a > b) - int(a < b)


class Page(story.Story):

  def __init__(self, url, page_set=None, base_dir=None, name='',
               tags=None, make_javascript_deterministic=True,
               shared_page_state_class=shared_page_state.SharedPageState,
               grouping_keys=None,
               cache_temperature=cache_temperature_module.ANY,
               traffic_setting=traffic_setting_module.NONE,
               platform_specific=False,
               extra_browser_args=None,
               perform_final_navigation=False):
    self._url = url
    self._SchemeErrorCheck()

    super().__init__(
        shared_page_state_class, name=name, tags=tags,
        is_local=self._scheme in ['file', 'chrome', 'about'],
        make_javascript_deterministic=make_javascript_deterministic,
        grouping_keys=grouping_keys, platform_specific=platform_specific)

    self._page_set = page_set
    # Default value of base_dir is the directory of the file that defines the
    # class of this page instance.
    if base_dir is None:
      base_dir = os.path.dirname(inspect.getfile(self.__class__))
    self._base_dir = base_dir
    self._name = name
    self._cache_temperature = cache_temperature
    # A "final navigation" is a navigation to about:blank after the
    # test in order to flush metrics. Some UMA metrics are not output to traces
    # until the page is navigated away from.
    self._perform_final_navigation = perform_final_navigation

    assert traffic_setting in traffic_setting_module.NETWORK_CONFIGS, (
        'Invalid traffic setting: %s' % traffic_setting)
    self._traffic_setting = traffic_setting

    # These attributes can be set dynamically by the page.
    self.synthetic_delays = dict()
    self.skip_waits = False
    self.script_to_evaluate_on_commit = None
    self._extra_browser_args = extra_browser_args or []

  @property
  def cache_temperature(self):
    return self._cache_temperature

  @property
  def traffic_setting(self):
    return self._traffic_setting

  @property
  def extra_browser_args(self):
    return self._extra_browser_args

  def _SchemeErrorCheck(self):
    if not self._scheme:
      raise ValueError('Must prepend the URL with scheme (e.g. file://)')

  def Run(self, shared_state):
    current_tab = shared_state.current_tab
    # Collect garbage from previous run several times to make the results more
    # stable if needed.
    for _ in range(0, 5):
      current_tab.CollectGarbage()
    action_runner = action_runner_module.ActionRunner(
        current_tab, skip_waits=self.skip_waits)
    with shared_state.interval_profiling_controller.SamplePeriod(
        'story_run', action_runner):
      shared_state.NavigateToPage(action_runner, self)
      shared_state.RunPageInteractions(action_runner, self)
    # Navigate to about:blank in order to force previous page's metrics to
    # flush. Needed for many UMA metrics reported from PageLoadMetricsObserver.
    if self._perform_final_navigation:
      action_runner.Navigate('about:blank')

  def RunNavigateSteps(self, action_runner):
    url = self.file_path_url_with_scheme if self.is_file else self.url
    action_runner.Navigate(
        url, script_to_evaluate_on_commit=self.script_to_evaluate_on_commit)

  def RunPageInteractions(self, action_runner):
    """Override this to define custom interactions with the page.
    e.g:
      def RunPageInteractions(self, action_runner):
        action_runner.ScrollPage()
        action_runner.TapElement(text='Next')
    """

  def AsDict(self):
    """Converts a page object to a dict suitable for JSON output."""
    d = {
        'id': self._id,
        'url': self._url,
    }
    if self._name:
      d['name'] = self._name
    return d

  @property
  def story_set(self):
    return self._page_set

  @property
  def url(self):
    return self._url

  def GetSyntheticDelayCategories(self):
    result = []
    for delay, options in self.synthetic_delays.items():
      options = '%f;%s' % (options.get('target_duration', 0),
                           options.get('mode', 'static'))
      result.append('DELAY(%s;%s)' % (delay, options))
    return result

  def __lt__(self, other):
    return self.url < other.url

  def __cmp__(self, other):
    x = cmp(self.name, other.name)
    if x != 0:
      return x
    return cmp(self.url, other.url)

  def __eq__(self, other):
    return self.name == other.name and self.url == other.url

  def __hash__(self):
    return hash((self.name, self.url))

  def __str__(self):
    return self.url

  @property
  def _scheme(self):
    return six.moves.urllib.parse.urlparse(self.url).scheme

  @property
  def is_file(self):
    """Returns True iff this URL points to a file."""
    return self._scheme == 'file'

  @property
  def file_path(self):
    """Returns the path of the file, stripping the scheme and query string."""
    assert self.is_file
    # Because ? is a valid character in a filename,
    # we have to treat the URL as a non-file by removing the scheme.
    parsed_url = six.moves.urllib.parse.urlparse(self.url[7:])
    return os.path.normpath(os.path.join(
        self._base_dir, parsed_url.netloc + parsed_url.path))

  @property
  def base_dir(self):
    return self._base_dir

  @property
  def file_path_url(self):
    """Returns the file path, including the params, query, and fragment."""
    assert self.is_file
    file_path_url = os.path.normpath(
        os.path.join(self._base_dir, self.url[7:]))
    # Preserve trailing slash or backslash.
    # It doesn't matter in a file path, but it does matter in a URL.
    if self.url.endswith('/'):
      file_path_url += os.sep
    return file_path_url

  @property
  def file_path_url_with_scheme(self):
    return 'file://' + self.file_path_url

  @property
  def serving_dir(self):
    if not self.is_file:
      return None
    file_path = os.path.realpath(self.file_path)
    if os.path.isdir(file_path):
      return file_path
    return os.path.dirname(file_path)
