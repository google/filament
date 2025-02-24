# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import hashlib
import struct
import typing

from recipe_engine import recipe_test_api


class BotUpdateTestApi(recipe_test_api.RecipeTestApi):

  @recipe_test_api.mod_test_data
  @staticmethod
  def fail_checkout(val: bool):
    return val

  @recipe_test_api.mod_test_data
  @staticmethod
  def fail_patch(val: bool | typing.Literal['download']):
    return val

  @recipe_test_api.mod_test_data
  @staticmethod
  def commit_positions(val: bool):
    return val

  def output_json(self,
                  first_sln,
                  revision_mapping,
                  *,
                  patch_root=None,
                  fixed_revisions=None,
                  fail_checkout: bool = False,
                  fail_patch: bool | typing.Literal['download'] = False,
                  commit_positions: bool = True):
    """Deterministically synthesize json.output test data for gclient's
    --output-json option.
    """
    t = recipe_test_api.StepTestData()

    output = {
        'did_run': True,
    }

    if fail_checkout:
      t.retcode = 1
    else:
      revisions = {
          project_name: self.gen_revision(project_name)
          for project_name in set(revision_mapping.values())
      }
      if fixed_revisions:
        for project_name, revision in fixed_revisions.items():
          if revision == 'HEAD':
            revision = self.gen_revision(project_name)
          elif revision.startswith('refs/') or revision.startswith('origin/'):
            revision = self.gen_revision('{}@{}'.format(project_name, revision))
          revisions[project_name] = revision

      properties = {
          property_name: revisions[project_name]
          for property_name, project_name in revision_mapping.items()
      }
      properties.setdefault('got_revision', self.gen_revision(first_sln))
      if commit_positions:

        def get_ref(project_name):
          revision = fixed_revisions.get(project_name, 'HEAD')
          if revision.startswith('origin/'):
            return revision.replace('origin/', 'refs/heads/', 1)
          if revision.startswith('refs/'):
            return revision
          return 'refs/heads/main'

        properties.update({
            '%s_cp' % property_name:
            ('%s@{#%s}' %
             (get_ref(project_name), self.gen_commit_position(project_name)))
            for property_name, project_name in revision_mapping.items()
        })

      output.update({
          'patch_root': patch_root,
          'root': first_sln,
          'properties': properties,
          'step_text': 'Some step text'
      })

      output.update({
          'manifest': {
              project_name: {
                  'repository': 'https://fake.org/%s.git' % project_name,
                  'revision': revision,
              }
              for project_name, revision in revisions.items()
          }
      })

      output.update({
          'source_manifest': {
              'version': 0,
              'directories': {
                  project_name: {
                      'git_checkout': {
                          'repo_url': 'https://fake.org/%s.git' % project_name,
                          'revision': revision
                      }
                  }
                  for project_name, revision in revisions.items()
              }
          }
      })

      if fixed_revisions:
        output['fixed_revisions'] = fixed_revisions

      if patch_root and fail_patch:
        output['patch_failure'] = True
        output['failed_patch_body'] = '\n'.join([
            'Downloading patch...',
            'Applying the patch...',
            'Patch: foo/bar.py',
            'Index: foo/bar.py',
            'diff --git a/foo/bar.py b/foo/bar.py',
            'index HASH..HASH MODE',
            '--- a/foo/bar.py',
            '+++ b/foo/bar.py',
            'context',
            '+something',
            '-something',
            'more context',
        ])
        output['patch_apply_return_code'] = 1
        if fail_patch == 'download':
          output['patch_apply_return_code'] = 3
          t.retcode = 87
        else:
          t.retcode = 88

    return t + self.m.json.output(output)

  @staticmethod
  def gen_revision(project):
    """Hash project to bogus deterministic git hash values."""
    h = hashlib.sha1(project.encode('utf-8'))
    return h.hexdigest()

  @staticmethod
  def gen_commit_position(project):
    """Hash project to bogus deterministic Cr-Commit-Position values."""
    h = hashlib.sha1(project.encode('utf-8'))
    return struct.unpack('!I', h.digest()[:4])[0] % 300000
