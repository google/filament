# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry import story
from telemetry.core import platform
from telemetry.page import page
from telemetry.internal.util import binary_manager


HTTP_EXAMPLE = 'http://www.example.com'
HTTPS_EXAMPLE = 'https://www.example.com'


def FetchExampleDomainArchive():
  ''' Return the path to wpr go archive of example.com page.

  This may involve fetching the archives from cloud storage if it doesn't
  exist on local file system.
  '''
  p = platform.GetHostPlatform()
  return binary_manager.FetchPath(
      'example_domain_wpr_go_archive', p.GetOSName(), p.GetArchName())


class ExampleDomainPageSet(story.StorySet):
  def __init__(self):
    super().__init__(
        archive_data_file='data/example_domain.json',
        cloud_storage_bucket=story.PUBLIC_BUCKET)

    self.AddStory(page.Page(HTTP_EXAMPLE, self, name=HTTP_EXAMPLE))
    self.AddStory(page.Page(HTTPS_EXAMPLE, self, name=HTTPS_EXAMPLE))
