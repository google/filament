# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import hashlib

from recipe_engine import recipe_test_api

class GclientTestApi(recipe_test_api.RecipeTestApi):
  def diff_deps_test_data(self, files):
    return self.m.raw_io.stream_output_text(
        '\n'.join(['10>%s' % fname for fname in files]))

  def output_json(self, projects):
    """Deterministically synthesize json.output test data for gclient's
    --output-json option.

    Args:
      projects - a list of project paths (e.g. ['src', 'src/dependency'])
    """
    # TODO(iannucci): Account for parent_got_revision_mapping. Right now the
    # synthesized json output from this method will always use
    # gen_revision(project), but if parent_got_revision and its ilk are
    # specified, we should use those values instead.
    return self.m.json.output({
      'solutions': dict(
        (p+'/', {'revision': self.gen_revision(p)})
        for p in projects
      )
    })

  @staticmethod
  def gen_revision(project):
    """Hash project to bogus deterministic revision values."""
    h = hashlib.sha1(project.encode('utf-8'))
    return h.hexdigest()
