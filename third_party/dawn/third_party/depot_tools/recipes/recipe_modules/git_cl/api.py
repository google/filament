# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import recipe_api
from recipe_engine.config_types import Path

from typing import Optional


class GitClApi(recipe_api.RecipeApi):
  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)
    self._default_repo_location: Optional[Path] = None

  def __call__(self, subcmd, args, name=None, **kwargs):
    if not name:
      name = 'git_cl ' + subcmd

    if kwargs.get('suffix'):
      name = name + ' (%s)' % kwargs.pop('suffix')

    my_loc = self._default_repo_location
    cmd = ['vpython3', self.repo_resource('git_cl.py'), subcmd] + args
    with self.m.context(cwd=self.m.context.cwd or my_loc):
      return self.m.step(name, cmd, **kwargs)

  def set_default_repo_location(self, path: Optional[Path]):
    """Sets the working directory where `git cl` will run, unless `cwd` from the
    context module has been set.

    If you set `path` to None, this will remove the default.
    """
    self._default_repo_location = path

  def get_description(self, patch_url=None, **kwargs):
    """DEPRECATED. Consider using gerrit.get_change_description instead."""
    args = ['-d']
    if patch_url:
      args.append(patch_url)

    return self('description', args, stdout=self.m.raw_io.output(), **kwargs)

  def set_description(self, description, patch_url=None, **kwargs):
    args = ['-n', '-']
    if patch_url:
      args.append(patch_url)

    return self(
        'description', args, stdout=self.m.raw_io.output(),
        stdin=self.m.raw_io.input_text(description),
        name='git_cl set description', **kwargs)

  def upload(self, message, upload_args=None, **kwargs):
    upload_args = upload_args or []

    upload_args.extend(['--message-file', self.m.raw_io.input_text(message)])

    return self('upload', upload_args, **kwargs)

  def issue(self, **kwargs):
    return self('issue', [], stdout=self.m.raw_io.output(), **kwargs)
