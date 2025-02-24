# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Simple customizable stories and story sets to use in tests.

There are two main kinds of stories defined:
- TestPage, a Page subclass using the default SharedPageState. Whether an
  actual browser is involved in tests using these depends on the options object
  built with options_for_unittests.GetRunOptions() and passed to the relevant
  story running functions.
- DummyStory, a Story using a TestSharedState and a mock platform. Tests using
  these never involve a real browser.

This module also provides helpers to easily create story sets and other related
classes to work with these kinds of stories.
"""

from __future__ import absolute_import
import posixpath
from unittest import mock
import six
import six.moves.urllib.parse # pylint: disable=import-error,wrong-import-order

from telemetry.core import platform as platform_module
from telemetry.core import util
from telemetry import page
from telemetry import story as story_module
from telemetry.web_perf import story_test


class DummyStoryTest(story_test.StoryTest):
  """A dummy no-op StoryTest.

  Does nothing in addition to whatever the shared state, as determined by the
  stories used in the tests, do.
  """
  def __init__(self, options=None):
    del options  # Unused.

  def WillRunStory(self, platform, story=None):
    del platform, story  # Unused.

  def Measure(self, platform, results):
    del platform, results  # Unused.

  def DidRunStory(self, platform, results):
    del platform, results  # Unused.


class TestPage(page.Page):
  def __init__(self, story_set, url, name=None, run_side_effect=None):
    """A simple customizable page.

    Note that this uses the default shared_page_state.SharedPageState, as most
    stories do, which includes method calls to interact with a browser and its
    platform. Whether a real browser is actually used depends on the options
    object built with the help of options_for_unittests.GetRunOptions().

    Args:
      story_set: An instance of the StorySet object this page belongs to.
      url: A URL for the page to load, in tests usually a local 'file://' URI.
      name: A name for the story. If not given a reasonable default is built
        from the url.
      run_side_effect: Side effect of the story's RunPageInteractions method.
        It should be a callable taking an action_runner, or an instance of
        an exception to be raised.
    """
    if name is None:
      name = _StoryNameFromUrl(url)
    super().__init__(
        url, story_set, name=name, base_dir=story_set.base_dir)
    self._run_side_effect = run_side_effect

  def RunPageInteractions(self, action_runner):
    if self._run_side_effect is not None:
      if isinstance(self._run_side_effect, Exception):
        raise self._run_side_effect  # pylint: disable=raising-bad-type
      self._run_side_effect(action_runner)


def SinglePageStorySet(url=None, name=None, base_dir=None,
                       story_run_side_effect=None):
  """Create a simple StorySet with a single TestPage.

  Args:
    url: An optional URL for the page to load, in tests usually a local
      'file://' URI. Defaults to 'file://blank.html' which, if using the
      default base_dir, points to a simple 'Hello World' html page.
    name: An optional name for the story. If omitted a reasonable default is
      built from the url.
    base_dir: A path on the local file system from which file URIs are served.
      Defaults to serving pages from telemetry/internal/testing.
    story_run_side_effect: Side effect of running the story. See TestPage
      docstring for details.
  """
  if url is None:
    url = 'file://blank.html'
  if base_dir is None:
    base_dir = util.GetUnittestDataDir()
  story_set = story_module.StorySet(base_dir=base_dir)
  story_set.AddStory(TestPage(story_set, url, name, story_run_side_effect))
  return story_set


class DummyStory(story_module.Story):
  def __init__(self, name, serving_dir=None, run_side_effect=None, **kwargs):
    """A customizable dummy story.

    It uses the TestSharedState, defined below with a mock platform, so tests
    using these never actually involve a real browser.

    Args:
      name: A string with the name of the story.
      serving_dir: Optional path from which (in a real local story) contents
        are served. Used in some tests but no local servers are actually set up.
      run_side_effect: Optional side effect of the story's Run method.
        It can be either an exception instance to raise, or a callable
        with no arguments.
      Extra kwargs are passed to the constructor of the base class.
    """
    super().__init__(TestSharedState, name=name, **kwargs)
    self._serving_dir = serving_dir
    self._run_side_effect = run_side_effect

  def Run(self, _):
    if self._run_side_effect is not None:
      if isinstance(self._run_side_effect, BaseException):
        raise self._run_side_effect  # pylint: disable=raising-bad-type
      self._run_side_effect()

  @property
  def serving_dir(self):
    return self._serving_dir


class DummyStorySet(story_module.StorySet):
  def __init__(self, stories, cloud_bucket=None, abridging_tag=None, **kwargs):
    """A customizable dummy story set.

    Args:
      stories: A list of either story names or objects to add to the set.
        Instances of DummyStory are useful here.
      cloud_bucket: Optional cloud storage bucket where (in a real story set)
        data for WPR recordings is stored.
      abridging_tag: Optional story tag used to define a subset of stories
        to be run in abridged mode.
      Additional kwargs are passed to the StorySet base class.
    """
    super().__init__(
        cloud_storage_bucket=cloud_bucket, **kwargs)
    self._abridging_tag = abridging_tag
    assert stories, 'There should be at least one story.'
    for story in stories:
      if isinstance(story, six.string_types):
        story = DummyStory(story)
      self.AddStory(story)

  def GetAbridgedStorySetTagFilter(self):
    return self._abridging_tag


def MockPlatform():
  """Create a mock platform to be used by tests."""
  mock_platform = mock.Mock(spec=platform_module.Platform)
  mock_platform.CanMonitorThermalThrottling.return_value = False
  mock_platform.GetArchName.return_value = None
  mock_platform.GetOSName.return_value = None
  mock_platform.GetOSVersionName.return_value = None
  mock_platform.GetDeviceId.return_value = None
  return mock_platform


class TestSharedState(story_module.SharedState):
  # Using a mock platform so there are no real actions done on the actual
  # host platform; and allows callers to inspect or configure methods called.
  mock_platform = MockPlatform()

  def __init__(self, test, options, story_set, possible_browser):
    super().__init__(
        test, options, story_set, possible_browser)
    self._current_story = None

  @property
  def platform(self):
    return self.mock_platform

  def WillRunStory(self, story):
    self._current_story = story

  def CanRunStory(self, story):
    return True

  def RunStory(self, results):
    self._current_story.Run(self)

  def DidRunStory(self, results):
    self._current_story = None

  def TearDownState(self):
    pass

  def DumpStateUponStoryRunFailure(self, results):
    pass


def _StoryNameFromUrl(url):
  """Turns e.g. 'file://path/to/name.html' into just 'name'."""
  # Strip off URI scheme, params and query; keep only netloc and path.
  uri = six.moves.urllib.parse.urlparse(url)
  filepath = posixpath.basename(uri.netloc + uri.path)
  return posixpath.splitext(posixpath.basename(filepath))[0]
