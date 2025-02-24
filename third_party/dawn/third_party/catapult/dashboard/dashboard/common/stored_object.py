# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""A module for storing and getting objects from datastore.

App Engine datastore limits entity size to less than 1 MB; this module
supports storing larger objects by splitting the data and using multiple
datastore entities.

Although this module contains ndb.Model classes, these are not intended
to be used directly by other modules.

Example:
  john = Account()
  john.username = 'John'
  john.userid = 123
  stored_object.Set(john.userid, john)
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import pickle

from google.appengine.ext import ndb

# Max bytes per entity.
_CHUNK_SIZE = 1000 * 1000


@ndb.synctasklet
@ndb.transactional(propagation=ndb.TransactionOptions.ALLOWED, xg=True)
def Get(key):
  """Gets the value.

  Args:
    key: String key value.

  Returns:
    A value for key.
  """
  result = yield GetAsync(key)
  raise ndb.Return(result)


@ndb.tasklet
@ndb.transactional(propagation=ndb.TransactionOptions.ALLOWED, xg=True)
def GetAsync(key):
  entity = yield ndb.Key(MultipartEntity, key).get_async()
  if not entity:
    raise ndb.Return(None)
  yield entity.GetPartsAsync()
  raise ndb.Return(entity.GetData())


@ndb.synctasklet
@ndb.transactional(propagation=ndb.TransactionOptions.ALLOWED, xg=True)
def Set(key, value):
  """Sets the value in datastore.

  Args:
    key: String key value.
    value: A pickleable value to be stored.
  """
  yield SetAsync(key, value)


@ndb.tasklet
@ndb.transactional(propagation=ndb.TransactionOptions.ALLOWED, xg=True)
def SetAsync(key, value):
  entity = yield ndb.Key(MultipartEntity, key).get_async()
  if not entity:
    entity = MultipartEntity(id=key)
  entity.SetData(value)
  yield entity.PutAsync()


@ndb.synctasklet
@ndb.transactional(propagation=ndb.TransactionOptions.ALLOWED, xg=True)
def Delete(key):
  """Deletes the value in datastore."""
  yield DeleteAsync(key)


@ndb.tasklet
@ndb.transactional(propagation=ndb.TransactionOptions.ALLOWED, xg=True)
def DeleteAsync(key):
  multipart_entity_key = ndb.Key(MultipartEntity, key)
  # Check if the entity exists before attempting to delete it in order to avoid
  # contention errors.
  # "Every attempt to create, update, or delete an entity takes place in the
  # context of a transaction,"
  # "There is a write throughput limit of about one transaction per second
  # within a single entity group"
  entity = yield multipart_entity_key.get_async()
  if not entity:
    return
  yield (multipart_entity_key.delete_async(),
         MultipartEntity.DeleteAsync(multipart_entity_key))


class MultipartEntity(ndb.Model):
  """Container for PartEntity."""

  _use_memcache = True

  # Number of entities use to store serialized.
  size = ndb.IntegerProperty(default=0, indexed=False)

  @ndb.tasklet
  @ndb.transactional(propagation=ndb.TransactionOptions.ALLOWED, xg=True)
  def GetPartsAsync(self):
    """Deserializes data from multiple PartEntity."""
    if not self.size:
      raise ndb.Return(None)

    string_id = self.key.string_id()
    part_keys = [
        ndb.Key(MultipartEntity, string_id, PartEntity, i + 1)
        for i in range(self.size)
    ]
    part_entities = yield ndb.get_multi_async(part_keys)
    serialized = b''.join(p.value for p in part_entities if p is not None)
    self.SetData(pickle.loads(serialized))

  @classmethod
  @ndb.tasklet
  @ndb.transactional(propagation=ndb.TransactionOptions.ALLOWED, xg=True)
  def DeleteAsync(cls, key):
    part_keys = yield PartEntity.query(ancestor=key).fetch_async(keys_only=True)
    yield ndb.delete_multi_async(part_keys)

  @ndb.tasklet
  @ndb.transactional(propagation=ndb.TransactionOptions.ALLOWED, xg=True)
  def PutAsync(self):
    """Stores serialized data over multiple PartEntity."""
    part_list = [
        PartEntity(id=i + 1, parent=self.key, value=part)
        for i, part in enumerate(_Serialize(self.GetData()))
    ]
    self.size = len(part_list)
    yield ndb.put_multi_async(part_list + [self])

  def GetData(self):
    return getattr(self, '_data', None)

  def SetData(self, data):
    setattr(self, '_data', data)


class PartEntity(ndb.Model):
  """Holds a part of serialized data for MultipartEntity.

  This entity key has the form:
    ndb.Key('MultipartEntity', multipart_entity_id, 'PartEntity', part_index)
  """
  value = ndb.BlobProperty()


def _Serialize(value):
  """Serializes value and returns a list of its parts.

  Args:
    value: A pickleable value.

  Returns:
    A list of string representation of the value that has been pickled and split
    into _CHUNK_SIZE.
  """
  serialized = pickle.dumps(value, 2)
  length = len(serialized)
  values = []
  for i in range(0, length, _CHUNK_SIZE):
    values.append(serialized[i:i + _CHUNK_SIZE])
  return values
