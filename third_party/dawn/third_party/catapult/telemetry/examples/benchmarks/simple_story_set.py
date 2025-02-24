# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry import story
from telemetry import page


class ExamplePage(page.Page):

  def __init__(self, page_set):
    url = 'https://example.com'
    super().__init__(
        url=url,
        name=url,
        page_set=page_set)

  def RunPageInteractions(self, action_runner):
    # To see all the web APIs that action_runner supports, see:
    # telemetry.page.action_runner module.

    action_runner.Wait(0.5)
    action_runner.TapElement(text='More information...')
    action_runner.Wait(2)
    action_runner.ScrollPage()


class SimpleStorySet(story.StorySet):
  def __init__(self):
    super().__init__(
        archive_data_file='data/simple_story_set.json',
        cloud_storage_bucket=story.PARTNER_BUCKET)
    self.AddStory(ExamplePage(self))
