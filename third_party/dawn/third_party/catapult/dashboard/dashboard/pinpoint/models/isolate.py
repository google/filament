# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Model for storing information to look up isolates.

An isolate is a way to describe the dependencies of a specific build.

More about isolates:
https://github.com/luci/luci-py/blob/master/appengine/isolate/doc/client/Design.md
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime

from google.appengine.ext import deferred
from google.appengine.ext import ndb

# Isolates expire on RBE CAS server after 32 days. We expire
# our isolate lookups a little bit sooner, just to be safe.
ISOLATE_EXPIRY_DURATION = datetime.timedelta(days=30)


def Get(builder_name, change, target):
  """Retrieve an isolate hash from the Datastore.

  Args:
    builder_name: The name of the builder that produced the isolate.
    change: The Change the isolate was built at.
    target: The compile target the isolate is for.

  Returns:
    A tuple containing the isolate server and isolate hash as strings.
  """
  entity = ndb.Key(Isolate, _Key(builder_name, change, target)).get()
  if not entity:
    raise KeyError('No isolate with builder %s, change %s, and target %s.' %
                   (builder_name, change, target))

  if entity.created + ISOLATE_EXPIRY_DURATION < datetime.datetime.utcnow():
    raise KeyError('Isolate with builder %s, change %s, and target %s was '
                   'found, but is expired.' % (builder_name, change, target))

  return entity.isolate_server, entity.isolate_hash


def Put(isolate_infos):
  """Add isolate hashes to the Datastore.

  This function takes multiple entries to do a batched Datastore put.

  Args:
    isolate_infos: An iterable of tuples. Each tuple is of the form
        (builder_name, change, target, isolate_server, isolate_hash).
  """
  entities = []
  for isolate_info in isolate_infos:
    builder_name, change, target, isolate_server, isolate_hash = isolate_info
    entity = Isolate(
        isolate_server=isolate_server,
        isolate_hash=isolate_hash,
        id=_Key(builder_name, change, target))
    entities.append(entity)
  ndb.put_multi(entities)


# TODO(dberris): Turn this into a Dataflow job instead.
def DeleteExpiredIsolates(start_cursor=None):
  expire_time = datetime.datetime.utcnow() - ISOLATE_EXPIRY_DURATION
  q = Isolate.query()
  q = q.filter(Isolate.created < expire_time)

  keys, next_cursor, more = q.fetch_page(
      1000, start_cursor=start_cursor, keys_only=True)

  ndb.delete_multi(keys)

  if more and next_cursor:
    deferred.defer(DeleteExpiredIsolates, next_cursor)


class Isolate(ndb.Model):
  isolate_server = ndb.StringProperty(indexed=False, required=True)
  isolate_hash = ndb.StringProperty(indexed=False, required=True)
  created = ndb.DateTimeProperty(auto_now_add=True)

  # We can afford to look directly in Datastore here since we don't expect to
  # make multiple calls to this at a high rate to benefit from being in
  # memcache. This lets us clear out the cache in Datastore and not have to
  # clear out memcache as well.
  _use_memcache = False
  _use_datastore = True
  _use_cache = False


def _Key(builder_name, change, target):
  # The key must be stable across machines, platforms,
  # Python versions, and Python invocations.
  return '\n'.join((builder_name, change.id_string, target))
