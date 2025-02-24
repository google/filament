# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import six

from py_trace_event import trace_event

from telemetry.util import wpr_modes

class SharedState(six.with_metaclass(trace_event.TracedMetaClass, object)):
  """A class that manages the test state across multiple stories.
  It's styled on unittest.TestCase for handling test setup & teardown logic.

  """

  #pylint: disable=unused-argument
  def __init__(self, test, finder_options, story_set, possible_browser):
    """ This method is styled on unittest.TestCase.setUpClass.
    Override to do any action before running stories that
    share this same state.
    Args:
      test: a legacy_page_test.LegacyPageTest or story_test.StoryTest instance.
      options: a BrowserFinderOptions instance that contains command line
        options.
      story_set: a story.StorySet instance.
    """
    # TODO(crbug/404771): Move network controller options out of
    # browser_options and into finder_options.
    browser_options = finder_options.browser_options
    if finder_options.use_live_sites:
      self._wpr_mode = wpr_modes.WPR_OFF
    elif browser_options.wpr_mode == wpr_modes.WPR_RECORD:
      self._wpr_mode = wpr_modes.WPR_RECORD
    else:
      self._wpr_mode = wpr_modes.WPR_REPLAY
    self._possible_browser = possible_browser

  @property
  def platform(self):
    """ Override to return the platform which stories that share this same
    state will be run on.
    """
    raise NotImplementedError()

  @property
  def wpr_mode(self):
    return self._wpr_mode

  def WillRunStory(self, story):
    """ Override to do any action before running each one of all stories
    that share this same state.
    This method is styled on unittest.TestCase.setUp.
    """
    raise NotImplementedError()

  def DidRunStory(self, results):
    """ Override to do any action after running each of all stories that
    share this same state.
    This method is styled on unittest.TestCase.tearDown.
    """
    raise NotImplementedError()

  def CanRunStory(self, story):
    """Indicate whether the story can be run in the current configuration.
    This is called after WillRunStory and before RunStory. Return True
    if the story should be run, and False if it should be skipped.
    Most subclasses will probably want to override this to always
    return True.
    Args:
      story: a story.Story instance.
    """
    raise NotImplementedError()

  def RunStory(self, results):
    """ Override to do any action before running each one of all stories
    that share this same state.
    This method is styled on unittest.TestCase.run.
    """
    raise NotImplementedError()

  def TearDownState(self):
    """ Override to do any action after running multiple stories that
    share this same state.
    This method is styled on unittest.TestCase.tearDownClass.
    """
    raise NotImplementedError()

  def DumpStateUponStoryRunFailure(self, results):
    """ Dump state of the current story run in case of failure.

    This would usually mean recording additional artifacts (e.g. logs,
    screenshots) to help debugging the failure.

    Args:
      results: A PageTestResults object which implementations can use to record
          artifacts.
    """
    raise NotImplementedError()
