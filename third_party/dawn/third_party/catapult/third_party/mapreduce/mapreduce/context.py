#!/usr/bin/env python
#
# Copyright 2010 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Mapreduce execution context.

Mapreduce context provides handler code with information about
current mapreduce execution and organizes utility data flow
from handlers such as counters, log messages, mutation pools.
"""



__all__ = ["get",
           "Pool",
           "Context",
           "COUNTER_MAPPER_CALLS",
           "COUNTER_MAPPER_WALLTIME_MS",
           "DATASTORE_DEADLINE",
           "MAX_ENTITY_COUNT",
          ]

import heapq
import logging
import threading

try:
  from google.appengine.ext import ndb
except ImportError:
  ndb = None
from google.appengine.api import datastore
from google.appengine.ext import db
from google.appengine.runtime import apiproxy_errors


# Maximum number of items. Pool will be flushed when reaches this amount.
# Datastore API is smart to group entities into as few RPCs as possible without
# exceeding RPC size. So in theory, MAX_ENTITY_COUNT can be as big as
# the instance's memory can hold.
# This number is just an estimate.
# TODO(user): Do batching by entity size if cheap. b/10427424
MAX_ENTITY_COUNT = 20

# Deadline in seconds for mutation pool datastore operations.
DATASTORE_DEADLINE = 15

# The name of the counter which counts all mapper calls.
COUNTER_MAPPER_CALLS = "mapper-calls"

# Total walltime in msec given to mapper process. This is not just mapper
# hundler function, but includes all i/o overhead.
COUNTER_MAPPER_WALLTIME_MS = "mapper-walltime-ms"


# pylint: disable=protected-access
# pylint: disable=g-bad-name


def _normalize_entity(value):
  """Return an entity from an entity or model instance."""
  if ndb is not None and isinstance(value, ndb.Model):
    return None
  if getattr(value, "_populate_internal_entity", None):
    return value._populate_internal_entity()
  return value


def _normalize_key(value):
  """Return a key from an entity, model instance, key, or key string."""
  if ndb is not None and isinstance(value, (ndb.Model, ndb.Key)):
    return None
  if getattr(value, "key", None):
    return value.key()
  elif isinstance(value, basestring):
    return datastore.Key(value)
  else:
    return value


class _ItemList(object):
  """A buffer that holds arbitrary items and auto flushes them when full.

  Callers of this class provides the logic on how to flush.
  This class takes care of the common logic of when to flush and when to retry.

  Properties:
    items: list of objects.
    length: length of item list.
    size: aggregate item size in bytes.
  """

  DEFAULT_RETRIES = 3
  _LARGEST_ITEMS_TO_LOG = 5

  def __init__(self,
               max_entity_count,
               flush_function,
               timeout_retries=DEFAULT_RETRIES,
               repr_function=None):
    """Constructor.

    Args:
      max_entity_count: maximum number of entities before flushing it to db.
      flush_function: a function that can flush the items. The function is
        called with a list of items as the first argument, a dict of options
        as second argument. Currently options can contain {"deadline": int}.
        see self.flush on how the function is called.
      timeout_retries: how many times to retry upon timeouts.
      repr_function: a function that turns an item into meaningful
        representation. For debugging large items.
    """
    self.items = []
    self.__max_entity_count = int(max_entity_count)
    self.__flush_function = flush_function
    self.__repr_function = repr_function
    self.__timeout_retries = int(timeout_retries)

  def __str__(self):
    return "ItemList of with %s items" % len(self.items)

  def append(self, item):
    """Add new item to the list.

    If needed, append will first flush existing items and clear existing items.

    Args:
      item: an item to add to the list.
    """
    if self.should_flush():
      self.flush()
    self.items.append(item)

  def flush(self):
    """Force a flush."""
    if not self.items:
      return

    retry = 0
    options = {"deadline": DATASTORE_DEADLINE}
    while retry <= self.__timeout_retries:
      try:
        self.__flush_function(self.items, options)
        self.clear()
        break
      except db.Timeout, e:
        logging.warning(e)
        logging.warning("Flushing '%s' timed out. Will retry for the %s time.",
                        self, retry)
        retry += 1
        options["deadline"] *= 2
      except apiproxy_errors.RequestTooLargeError:
        self._log_largest_items()
        raise
    else:
      raise

  def _log_largest_items(self):
    if not self.__repr_function:
      logging.error("Got RequestTooLargeError but can't interpret items in "
                    "_ItemList %s.", self)
      return

    sizes = [len(self.__repr_function(i)) for i in self.items]
    largest = heapq.nlargest(self._LARGEST_ITEMS_TO_LOG,
                             zip(sizes, self.items),
                             lambda t: t[0])
    # Set field for for test only.
    self._largest = [(s, self.__repr_function(i)) for s, i in largest]
    logging.error("Got RequestTooLargeError. Largest items: %r", self._largest)

  def clear(self):
    """Clear item list."""
    self.items = []

  def should_flush(self):
    """Whether to flush before append the next entity.

    Returns:
      True to flush. False other.
    """
    return len(self.items) >= self.__max_entity_count


class Pool(object):
  """Mutation pool accumulates changes to perform them in patch.

  Any Pool subclass should not be public. Instead, Pool should define an
  operation.Operation class and let user uses that. For example, in a map
  function, user can do:

  def map(foo):
    yield OperationOnMyPool(any_argument)

  Since Operation is a callable object, Mapreduce library will invoke
  any Operation object that is yielded with context.Context instance.
  The operation object can then access MyPool from Context.get_pool.
  """

  def flush(self):
    """Flush all changes."""
    raise NotImplementedError()


class _MutationPool(Pool):
  """Mutation pool accumulates datastore changes to perform them in batch.

  Properties:
    puts: _ItemList of entities to put to datastore.
    deletes: _ItemList of keys to delete from datastore.
    ndb_puts: _ItemList of ndb entities to put to datastore.
    ndb_deletes: _ItemList of ndb keys to delete from datastore.
  """

  def __init__(self,
               max_entity_count=MAX_ENTITY_COUNT,
               mapreduce_spec=None):
    """Constructor.

    Args:
      max_entity_count: maximum number of entities before flushing it to db.
      mapreduce_spec: An optional instance of MapperSpec.
    """
    self.max_entity_count = max_entity_count
    params = mapreduce_spec.params if mapreduce_spec is not None else {}
    self.force_writes = bool(params.get("force_ops_writes", False))
    self.puts = _ItemList(max_entity_count,
                          self._flush_puts,
                          repr_function=self._db_repr)
    self.deletes = _ItemList(max_entity_count,
                             self._flush_deletes)
    self.ndb_puts = _ItemList(max_entity_count,
                              self._flush_ndb_puts,
                              repr_function=self._ndb_repr)
    self.ndb_deletes = _ItemList(max_entity_count,
                                 self._flush_ndb_deletes)

  def put(self, entity):
    """Registers entity to put to datastore.

    Args:
      entity: an entity or model instance to put.
    """
    actual_entity = _normalize_entity(entity)
    if actual_entity is None:
      return self.ndb_put(entity)
    self.puts.append(actual_entity)

  def ndb_put(self, entity):
    """Like put(), but for NDB entities."""
    assert ndb is not None and isinstance(entity, ndb.Model)
    self.ndb_puts.append(entity)

  def delete(self, entity):
    """Registers entity to delete from datastore.

    Args:
      entity: an entity, model instance, or key to delete.
    """
    key = _normalize_key(entity)
    if key is None:
      return self.ndb_delete(entity)
    self.deletes.append(key)

  def ndb_delete(self, entity_or_key):
    """Like delete(), but for NDB entities/keys."""
    if ndb is not None and isinstance(entity_or_key, ndb.Model):
      key = entity_or_key.key
    else:
      key = entity_or_key
    self.ndb_deletes.append(key)

  def flush(self):
    """Flush(apply) all changed to datastore."""
    self.puts.flush()
    self.deletes.flush()
    self.ndb_puts.flush()
    self.ndb_deletes.flush()

  @classmethod
  def _db_repr(cls, entity):
    """Converts entity to a readable repr.

    Args:
      entity: datastore.Entity or datastore_types.Key.

    Returns:
      Proto in str.
    """
    return str(entity._ToPb())

  @classmethod
  def _ndb_repr(cls, entity):
    """Converts entity to a readable repr.

    Args:
      entity: ndb.Model

    Returns:
      Proto in str.
    """
    return str(entity._to_pb())

  def _flush_puts(self, items, options):
    """Flush all puts to datastore."""
    datastore.Put(items, config=self._create_config(options))

  def _flush_deletes(self, items, options):
    """Flush all deletes to datastore."""
    datastore.Delete(items, config=self._create_config(options))

  def _flush_ndb_puts(self, items, options):
    """Flush all NDB puts to datastore."""
    assert ndb is not None
    ndb.put_multi(items, config=self._create_config(options))

  def _flush_ndb_deletes(self, items, options):
    """Flush all deletes to datastore."""
    assert ndb is not None
    ndb.delete_multi(items, config=self._create_config(options))

  def _create_config(self, options):
    """Creates datastore Config.

    Returns:
      A datastore_rpc.Configuration instance.
    """
    return datastore.CreateConfig(deadline=options["deadline"],
                                  force_writes=self.force_writes)


class _Counters(Pool):
  """Regulates access to counters.

  Counters Pool is a str to int map. It is saved as part of ShardState so it
  is flushed when ShardState commits to datastore successfully.
  """

  def __init__(self, shard_state):
    """Constructor.

    Args:
      shard_state: current mapreduce shard state as model.ShardState.
    """
    self._shard_state = shard_state

  def increment(self, counter_name, delta=1):
    """Increment counter value.

    Args:
      counter_name: name of the counter as string.
      delta: increment delta as int.
    """
    self._shard_state.counters_map.increment(counter_name, delta)

  def flush(self):
    """Flush unsaved counter values."""
    pass


# TODO(user): Define what fields should be public.
class Context(object):
  """MapReduce execution context.

  The main purpose of Context is to facilitate IO. User code, input reader,
  and output writer code can plug in pools (see Pool class) to Context to
  batch operations.

  There is a single Context instance associated with each worker thread.
  It can be accessed via context.get(). handlers.MapperWorkerHandler creates
  this instance before any IO code (input_reader, output_writer, user functions)
  is called.

  Each Pool decides how to batch and when to flush.
  Context and all its pools are flushed by the end of a slice.
  Upon error in slice execution, what is flushed is undefined. (See _Counters
  for an exception).

  Properties:
    mapreduce_spec: current mapreduce specification as model.MapreduceSpec.
  """

  # Current context instance
  _local = threading.local()

  def __init__(self, mapreduce_spec, shard_state, task_retry_count=0):
    """Constructor.

    Args:
      mapreduce_spec: mapreduce specification as model.MapreduceSpec.
      shard_state: an instance of model.ShardState. This has to be the same
        instance as the one MapperWorkerHandler mutates. All mutations are
        flushed to datastore in the end of the slice.
      task_retry_count: how many times this task has been retried.
    """
    self._shard_state = shard_state
    self.mapreduce_spec = mapreduce_spec
    # TODO(user): Create a hierarchy of Context classes. Certain fields
    # like task_retry_count only makes sense in TaskAttemptContext.
    self.task_retry_count = task_retry_count

    if self.mapreduce_spec:
      self.mapreduce_id = self.mapreduce_spec.mapreduce_id
    else:
      # Only in tests
      self.mapreduce_id = None
    if shard_state:
      self.shard_id = shard_state.get_shard_id()
    else:
      # Only in tests
      self.shard_id = None

    # TODO(user): Allow user to specify max entity count for the pool
    # as they know how big their entities are.
    self._mutation_pool = _MutationPool(mapreduce_spec=mapreduce_spec)
    self._counters = _Counters(shard_state)
    # TODO(user): Remove this after fixing
    # keyhole/dataeng/imagery/feeds/client_lib.py in another CL.
    self.counters = self._counters

    self._pools = {}
    self.register_pool("mutation_pool", self._mutation_pool)
    self.register_pool("counters", self.counters)

  def flush(self):
    """Flush all information recorded in context."""
    for pool in self._pools.values():
      pool.flush()

  def register_pool(self, key, pool):
    """Register an arbitrary pool to be flushed together with this context.

    Args:
      key: pool key as string.
      pool: a pool instance.
    """
    self._pools[key] = pool

  def get_pool(self, key):
    """Obtains an instance of registered pool.

    Args:
      key: pool key as string.

    Returns:
      an instance of the pool registered earlier, or None.
    """
    return self._pools.get(key, None)

  @classmethod
  def _set(cls, context):
    """Set current context instance.

    Args:
      context: new context as Context or None.
    """
    cls._local._context_instance = context


# This method is intended for user code to access context instance.
# MR framework should still try to take context as an explicit argument
# whenever possible (dependency injection).
def get():
  """Get current context instance.

  Returns:
    current context as Context.
  """
  if not hasattr(Context._local, "_context_instance") :
    return None
  return Context._local._context_instance
