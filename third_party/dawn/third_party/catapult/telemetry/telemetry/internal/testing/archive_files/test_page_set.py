# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry import story
from telemetry.page import page
from telemetry.internal.testing.pages.external_page import ExternalPage


class InternalPage(page.Page):
  def __init__(self, story_set):
    super().__init__('file://bar.html', page_set=story_set)

class TestPageSet(story.StorySet):
  """A pageset for testing purpose"""

  def __init__(self):
    super().__init__(
        archive_data_file='data/archive_files/test.json',
        cloud_storage_bucket=story.PUBLIC_BUCKET)

    #top google property; a google tab is often open
    class Google(page.Page):
      def __init__(self, story_set):
        # pylint: disable=bad-super-call
        super().__init__('https://www.google.com',
                                     page_set=story_set)

      def RunGetActionRunner(self, action_runner):
        return action_runner

    self.AddStory(Google(self))
    self.AddStory(InternalPage(self))
    self.AddStory(ExternalPage(self))
