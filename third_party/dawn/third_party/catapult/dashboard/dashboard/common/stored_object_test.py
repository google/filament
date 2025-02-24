# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from dashboard.common import stored_object
from dashboard.common import testing_common


class SampleSerializableClass:

  def __init__(self, data):
    self.data = data
    self.user_name = 'Chris'
    self.user_id = 1234
    self.family = {
        'nieces': 1,
        'nephews': 6,
    }

  def __eq__(self, other):
    return self.__dict__ == other.__dict__

  def __hash__(self):
    return hash(self.data, self.user_name, self.user_id, self.family)


class StoredObjectTest(testing_common.TestCase):

  def testSetAndGet(self):
    new_account = SampleSerializableClass('Some account data.')
    stored_object.Set('chris', new_account)
    chris_account = stored_object.Get('chris')
    self.assertEqual(new_account, chris_account)

  def testSetAndGet_LargeObject(self):
    a_large_string = '0' * 2097152
    new_account = SampleSerializableClass(a_large_string)
    stored_object.Set('chris', new_account)
    chris_account = stored_object.Get('chris')

    part_entities = stored_object.PartEntity.query().fetch()

    self.assertEqual(new_account, chris_account)

    # chris_account object should be stored over 3 PartEntity entities.
    self.assertEqual(3, len(part_entities))

  def testDelete_LargeObject_AllEntitiesDeleted(self):
    a_large_string = '0' * 2097152
    new_account = SampleSerializableClass(a_large_string)
    stored_object.Set('chris', new_account)

    stored_object.Delete('chris')

    multipart_entities = stored_object.MultipartEntity.query().fetch()
    self.assertEqual(0, len(multipart_entities))
    part_entities = stored_object.PartEntity.query().fetch()
    self.assertEqual(0, len(part_entities))


if __name__ == '__main__':
  unittest.main()
