# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import recipe_test_api

class GitTestApi(recipe_test_api.RecipeTestApi):
  def count_objects_output(self, value):
    return (
        'count: %s\n'
        'size: %s\n'
        'in_pack: %s\n'
        'packs: %s\n'
        'size-pack: %s\n'
        'prune-packable: %s\n'
        'garbage: %s\n'
        'size-garbage: %s\n'
    ) % tuple([value] * 8)
