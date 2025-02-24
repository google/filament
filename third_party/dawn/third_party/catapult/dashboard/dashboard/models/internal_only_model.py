# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Base classes for Model classes that can be internal-only."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from google.appengine.ext import ndb

from dashboard.common import datastore_hooks


class InternalOnlyModel(ndb.Model):
  """A superclass for Models that have an internal_only property."""

  @classmethod
  def _post_get_hook(cls, key, future):  # pylint: disable=unused-argument
    """Throws an exception when external users try to get() internal data."""
    entity = future.get_result()
    if entity is None:
      return
    # Internal-only objects should never be accessed by non-internal accounts!
    if (getattr(entity, 'internal_only', False)
        and not datastore_hooks.IsUnalteredQueryPermitted()):
      # Keep info about the fact that we're doing an access check out of the
      # callstack in case app engine shows it to the user.
      assert False


class CreateHookInternalOnlyModel(InternalOnlyModel):
  """Base Model class which implements a create hook called CreateCallback."""

  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)
    # This attribute is used to keep track of whether its the first time the
    # entity has been created, so that the CreateCallback is only called once.
    self._is_saved = False

  def _post_put_hook(self, future):
    """Invokes a callback upon creating a new entity."""
    if not self._is_saved:
      try:
        future.check_success()
      except Exception:  # pylint: disable=broad-except
        # check_success throws an exception for failures, but the reference
        # isn't explicit about what type of exception gets thrown.
        pass
      if callable(getattr(self, 'CreateCallback', None)):
        self.CreateCallback()
    self._is_saved = True
