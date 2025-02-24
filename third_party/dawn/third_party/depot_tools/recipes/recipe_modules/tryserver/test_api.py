# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import recipe_test_api


class TryserverTestApi(recipe_test_api.RecipeTestApi):

  @recipe_test_api.mod_test_data
  @staticmethod
  def gerrit_change_target_ref(target_ref):
    assert target_ref.startswith('refs/')
    return target_ref

  def get_files_affected_by_patch(self,
                                  files,
                                  step_name='git diff to analyze patch'):
    """Override test data for the `get_files_affected_by_patch` method.

    Args:
      files: The files that git should report as changed as paths
        relative to the root of the repo.

    Example:
      yield api.test(
          'my_test',
          api.tryserver.get_files_affected_by_patch('foo.cc',
                                                    'bar/baz.cc'),
      )
    """
    output = self.m.raw_io.stream_output('\n'.join(files))
    return self.override_step_data(step_name, output)

  def get_footers(self, footers, step_name='parse description'):
    """Override test data for the `get_footers` method.

    Args:
      footers: A dict containing key/value pairs for footers to return. To be
          accurate to git_footers.py's actual behavior, values should be lists.

    Example:
      yield api.test(
          'my_test',
          api.tryserver.get_footers({'Footer-Key': ['footer_value']}))
    """
    output = self.m.json.output(footers)
    return self.override_step_data(step_name, output)
