# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""The `depot_tools` module provides safe functions to access paths within
the depot_tools repo."""

import contextlib

from recipe_engine import recipe_api

class DepotToolsApi(recipe_api.RecipeApi):
  def __init__(self, **kwargs):
    super(DepotToolsApi, self).__init__(**kwargs);
    self._cipd_bin_setup_called = False

  @property
  def download_from_google_storage_path(self):
    return self.repo_resource('download_from_google_storage.py')

  @property
  def upload_to_google_storage_path(self):
    return self.repo_resource('upload_to_google_storage.py')

  @property
  def roll_downstream_gcs_deps_path(self):
    return self.repo_resource('roll_downstream_gcs_deps.py')

  @property
  def root(self):
    """Returns (Path): The "depot_tools" root directory."""
    return self.repo_resource()

  @property
  def cros_path(self):
    return self.repo_resource('cros')

  @property
  def gn_py_path(self):
    return self.repo_resource('gn.py')

  # TODO(dnj): Remove this once everything uses the "gsutil" recipe module
  # version.
  @property
  def gsutil_py_path(self):
    return self.repo_resource('gsutil.py')

  @property
  def presubmit_support_py_path(self):
    return self.repo_resource('presubmit_support.py')

  @contextlib.contextmanager
  def on_path(self):
    """Use this context manager to put depot_tools on $PATH.

    Example:

    ```python
    with api.depot_tools.on_path():
      # run some steps
    ```
    """
    # By default Depot Tools do not auto update on the bots.
    # (crbug/1090603)
    with self.m.context(
        **{'env_suffixes': {
            'PATH': [self.root],
            'DEPOT_TOOLS_UPDATE': '0'
        }}):
      yield
