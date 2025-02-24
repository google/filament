# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from unittest import mock

from google.appengine.ext import ndb

from dashboard.common import testing_common
from dashboard.models import internal_only_model


class InternalOnlyModelExample(internal_only_model.InternalOnlyModel):
  """Example Model which has an internal_only property."""

  internal_only = ndb.BooleanProperty(default=False)


class CreateHookModelExample(internal_only_model.CreateHookInternalOnlyModel):
  """Example Model which has a CreateHook method."""

  internal_only = ndb.BooleanProperty(default=False)

  def CreateCallback(self):
    pass


class InternalOnlyModelTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    testing_common.SetIsInternalUser('x@google.com', True)
    testing_common.SetIsInternalUser('x@foo.com', False)

  def testInternalOnlyModel_InternalUser_EntityFetched(self):
    key = InternalOnlyModelExample(internal_only=True).put()
    self.SetCurrentUser('x@google.com')
    self.assertEqual(key, key.get().key)

  def testInternalOnlyModel_ExternalUserInternalEntity_AssertionRaised(self):
    key = InternalOnlyModelExample(internal_only=True).put()
    self.SetCurrentUser('x@foo.com')
    with self.assertRaises(AssertionError):
      key.get()

  def testInternalOnlyModel_ExternalUserExternalEntity_EntityFetched(self):
    key = InternalOnlyModelExample(internal_only=False).put()
    self.SetCurrentUser('x@foo.com')
    self.assertEqual(key, key.get().key)

  @mock.patch.object(CreateHookModelExample, 'CreateCallback')
  def testCreateCallBack_CalledOncePerEntity(self, mock_create_callback):
    one = CreateHookModelExample()
    two = CreateHookModelExample()
    self.assertEqual(0, mock_create_callback.call_count)
    one_key = one.put()
    one.put()
    self.assertEqual(1, mock_create_callback.call_count)
    two_key = two.put()
    self.assertEqual(2, mock_create_callback.call_count)
    one_key.get().put()
    two_key.get().put()
    self.assertEqual(2, mock_create_callback.call_count)

  def testCreateHookEntity_InternalUserInternalEntity_EntityFetched(self):
    key = CreateHookModelExample(internal_only=True).put()
    self.SetCurrentUser('x@google.com')
    self.assertEqual(key, key.get().key)

  def testCreateHookEntity_ExternalUserInternalEntity_AssertionRaised(self):
    key = CreateHookModelExample(internal_only=True).put()
    self.SetCurrentUser('x@foo.com')
    with self.assertRaises(AssertionError):
      key.get()

  def testCreateHookEntity_ExternalUserExternalEntity_EntityFetched(self):
    key = CreateHookModelExample(internal_only=False).put()
    self.SetCurrentUser('x@foo.com')
    self.assertEqual(key, key.get().key)


if __name__ == '__main__':
  unittest.main()
