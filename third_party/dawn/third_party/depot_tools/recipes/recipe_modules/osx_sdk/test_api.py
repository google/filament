# Copyright 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re

from recipe_engine import recipe_test_api


class OSXSDKTestApi(recipe_test_api.RecipeTestApi):
  # In tests, this will be the version that we simulate macOS to be.
  DEFAULT_MACOS_VERSION = '14.4'

  def macos_version(
      self,
      major_minor: str = DEFAULT_MACOS_VERSION) -> recipe_test_api.TestData:
    """Mock the macOS Major.Minor[.Patch] version that osx_sdk will use to pick
    the Xcode SDK version from its internal table.

    This will only be used if the recipe does not explicitly select an SDK
    version via the osx_sdk properties (which can be mocked via the
    `pick_sdk_version` method below).

    Example use:

       yield api.test(
         'my-test-name',
         api.osx_sdk.macos_version('13.3'),
       )
    """
    if not re.match(r'^\d+(\.\d+){1,2}$', major_minor):
      raise ValueError(
          f'Expected Major.Minor[.Patch] (e.g. 14.4), got {major_minor=}')

    return self.step_data('find macOS version',
                          stdout=self.m.raw_io.output_text(major_minor))

  def pick_sdk_version(self,
                       sdk_version: str = 'deadbeef'
                       ) -> recipe_test_api.TestData:
    """This should be used to pick a precise SDK version.

    Recipes used on builders which configure the XCode version via properties
    should use this to more accurately reflect how these recipes will run in
    production. Specifically, when the XCode version is selected via properties,
    the osx_sdk module will not need or attempt to discover the current macOS
    version (which is mockable with the `macos_version` method above).

    Example use:

       yield api.test(
         'my-test-name',
         api.osx_sdk.pick_sdk_version('13c100'),
       )
    """
    return self.m.properties(
        **{"$depot_tools/osx_sdk": {
            "sdk_version": sdk_version,
        }})
