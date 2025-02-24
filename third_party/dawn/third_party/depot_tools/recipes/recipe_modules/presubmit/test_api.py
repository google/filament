# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import recipe_test_api

from PB.recipe_modules.depot_tools.presubmit import properties

class PresubmitTestApi(recipe_test_api.RecipeTestApi):

  def __call__(self, runhooks=False, timeout_s=480):
    return self.m.properties(
        **{
            '$depot_tools/presubmit': properties.InputProperties(
                runhooks=runhooks,
                timeout_s=timeout_s,
            ),
        }
    )
