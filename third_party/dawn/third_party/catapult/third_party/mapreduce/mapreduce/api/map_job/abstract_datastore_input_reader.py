#!/usr/bin/env python
"""Abstract Datastore Input Reader implementation for the map_job API."""
import random

from google.appengine.api import datastore
from google.appengine.ext import key_range
from mapreduce import datastore_range_iterators as db_iters
from mapreduce import errors
from mapreduce import key_ranges
from mapreduce import model
from mapreduce import namespace_range
from mapreduce.api.map_job import input_reader

# pylint: disable=protected-access
# pylint: disable=invalid-name


class AbstractDatastoreInputReader(input_reader.InputReader):
  """Implementation of an abstract base class for a Datastore input reader."""

  # Number of entities to fetch at once while doing scanning.
  _BATCH_SIZE = 50

  # Maximum number of shards we'll create.
  _MAX_SHARD_COUNT = 256

  # The maximum number of namespaces that will be sharded by datastore key
  # before switching to a strategy where sharding is done lexographically by
  # namespace.
  MAX_NAMESPACES_FOR_KEY_SHARD = 10

  _APP_PARAM = "_app"

  # reader parameters.
  NAMESPACE_PARAM = "namespace"
  ENTITY_KIND_PARAM = "entity_kind"
  KEYS_ONLY_PARAM = "keys_only"
  BATCH_SIZE_PARAM = "batch_size"
  KEY_RANGE_PARAM = "key_range"
  FILTERS_PARAM = "filters"

  _KEY_RANGE_ITER_CLS = db_iters.AbstractKeyRangeIterator

  def __init__(self, iterator):
    """Create new AbstractDatastoreInputReader object.

    This is internal constructor. Use split_input to create readers instead.

    Args:
      iterator: an iterator that generates objects for this input reader.
    """
    self._iter = iterator

  def __iter__(self):
    """Yields whatever the internal iterator yields."""
    for o in self._iter:
      yield o

  def __str__(self):
    """Returns the string representation of this InputReader."""
    return repr(self._iter)

  def to_json(self):
    """Inherit doc."""
    return self._iter.to_json()

  @classmethod
  def from_json(cls, state):
    """Inherit doc."""
    return cls(db_iters.RangeIteratorFactory.from_json(state))

  @classmethod
  def _get_query_spec(cls, params):
    """Construct a model.QuerySpec from model.MapperSpec."""
    entity_kind = params[cls.ENTITY_KIND_PARAM]
    filters = params.get(cls.FILTERS_PARAM)
    app = params.get(cls._APP_PARAM)
    ns = params.get(cls.NAMESPACE_PARAM)

    return model.QuerySpec(
        entity_kind=cls._get_raw_entity_kind(entity_kind),
        keys_only=bool(params.get(cls.KEYS_ONLY_PARAM, False)),
        filters=filters,
        batch_size=int(params.get(cls.BATCH_SIZE_PARAM, cls._BATCH_SIZE)),
        model_class_path=entity_kind,
        app=app,
        ns=ns)

  @classmethod
  def split_input(cls, job_config):
    """Inherit doc."""
    shard_count = job_config.shard_count
    params = job_config.input_reader_params
    query_spec = cls._get_query_spec(params)

    namespaces = None
    if query_spec.ns is not None:
      k_ranges = cls._to_key_ranges_by_shard(
          query_spec.app, [query_spec.ns], shard_count, query_spec)
    else:
      ns_keys = namespace_range.get_namespace_keys(
          query_spec.app, cls.MAX_NAMESPACES_FOR_KEY_SHARD+1)
      # No namespace means the app may have some data but those data are not
      # visible yet. Just return.
      if not ns_keys:
        return
      # If the number of ns is small, we shard each ns by key and assign each
      # shard a piece of a ns.
      elif len(ns_keys) <= cls.MAX_NAMESPACES_FOR_KEY_SHARD:
        namespaces = [ns_key.name() or "" for ns_key in ns_keys]
        k_ranges = cls._to_key_ranges_by_shard(
            query_spec.app, namespaces, shard_count, query_spec)
      # When number of ns is large, we can only split lexicographically by ns.
      else:
        ns_ranges = namespace_range.NamespaceRange.split(n=shard_count,
                                                         contiguous=False,
                                                         can_query=lambda: True,
                                                         _app=query_spec.app)
        k_ranges = [key_ranges.KeyRangesFactory.create_from_ns_range(ns_range)
                    for ns_range in ns_ranges]

    iters = [db_iters.RangeIteratorFactory.create_key_ranges_iterator(
        r, query_spec, cls._KEY_RANGE_ITER_CLS) for r in k_ranges]

    return [cls(i) for i in iters]

  @classmethod
  def _to_key_ranges_by_shard(cls, app, namespaces, shard_count, query_spec):
    """Get a list of key_ranges.KeyRanges objects, one for each shard.

    This method uses scatter index to split each namespace into pieces
    and assign those pieces to shards.

    Args:
      app: app_id in str.
      namespaces: a list of namespaces in str.
      shard_count: number of shards to split.
      query_spec: model.QuerySpec.

    Returns:
      a list of key_ranges.KeyRanges objects.
    """
    key_ranges_by_ns = []
    # Split each ns into n splits. If a ns doesn't have enough scatter to
    # split into n, the last few splits are None.
    for namespace in namespaces:
      ranges = cls._split_ns_by_scatter(
          shard_count,
          namespace,
          query_spec.entity_kind,
          app)
      # The nth split of each ns will be assigned to the nth shard.
      # Shuffle so that None are not all by the end.
      random.shuffle(ranges)
      key_ranges_by_ns.append(ranges)

    # KeyRanges from different namespaces might be very different in size.
    # Use round robin to make sure each shard can have at most one split
    # or a None from a ns.
    ranges_by_shard = [[] for _ in range(shard_count)]
    for ranges in key_ranges_by_ns:
      for i, k_range in enumerate(ranges):
        if k_range:
          ranges_by_shard[i].append(k_range)

    key_ranges_by_shard = []
    for ranges in ranges_by_shard:
      if ranges:
        key_ranges_by_shard.append(key_ranges.KeyRangesFactory.create_from_list(
            ranges))
    return key_ranges_by_shard

  @classmethod
  def _split_ns_by_scatter(cls,
                           shard_count,
                           namespace,
                           raw_entity_kind,
                           app):
    """Split a namespace by scatter index into key_range.KeyRange.

    TODO(user): Power this with key_range.KeyRange.compute_split_points.

    Args:
      shard_count: number of shards.
      namespace: namespace name to split. str.
      raw_entity_kind: low level datastore API entity kind.
      app: app id in str.

    Returns:
      A list of key_range.KeyRange objects. If there are not enough entities to
    splits into requested shards, the returned list will contain KeyRanges
    ordered lexicographically with any Nones appearing at the end.
    """
    if shard_count == 1:
      # With one shard we don't need to calculate any split points at all.
      return [key_range.KeyRange(namespace=namespace, _app=app)]

    ds_query = datastore.Query(kind=raw_entity_kind,
                               namespace=namespace,
                               _app=app,
                               keys_only=True)
    ds_query.Order("__scatter__")
    oversampling_factor = 32
    random_keys = ds_query.Get(shard_count * oversampling_factor)

    if not random_keys:
      # There are no entities with scatter property. We have no idea
      # how to split.
      return ([key_range.KeyRange(namespace=namespace, _app=app)] +
              [None] * (shard_count - 1))

    random_keys.sort()

    if len(random_keys) >= shard_count:
      # We've got a lot of scatter values. Sample them down.
      random_keys = cls._choose_split_points(random_keys, shard_count)

    k_ranges = []

    k_ranges.append(key_range.KeyRange(
        key_start=None,
        key_end=random_keys[0],
        direction=key_range.KeyRange.ASC,
        include_start=False,
        include_end=False,
        namespace=namespace,
        _app=app))

    for i in range(0, len(random_keys) - 1):
      k_ranges.append(key_range.KeyRange(
          key_start=random_keys[i],
          key_end=random_keys[i+1],
          direction=key_range.KeyRange.ASC,
          include_start=True,
          include_end=False,
          namespace=namespace,
          _app=app))

    k_ranges.append(key_range.KeyRange(
        key_start=random_keys[-1],
        key_end=None,
        direction=key_range.KeyRange.ASC,
        include_start=True,
        include_end=False,
        namespace=namespace,
        _app=app))

    if len(k_ranges) < shard_count:
      # We need to have as many shards as it was requested. Add some Nones.
      k_ranges += [None] * (shard_count - len(k_ranges))
    return k_ranges

  @classmethod
  def _choose_split_points(cls, sorted_keys, shard_count):
    """Returns the best split points given a random set of datastore.Keys."""
    assert len(sorted_keys) >= shard_count
    index_stride = len(sorted_keys) / float(shard_count)
    return [sorted_keys[int(round(index_stride * i))]
            for i in range(1, shard_count)]

  @classmethod
  def validate(cls, job_config):
    """Inherit docs."""
    super(AbstractDatastoreInputReader, cls).validate(job_config)
    params = job_config.input_reader_params

    # Check for the required entity kind parameter.
    if cls.ENTITY_KIND_PARAM not in params:
      raise errors.BadReaderParamsError("Missing input reader parameter "
                                        "'entity_kind'")
    # Validate the batch size parameter.
    if cls.BATCH_SIZE_PARAM in params:
      try:
        batch_size = int(params[cls.BATCH_SIZE_PARAM])
        if batch_size < 1:
          raise errors.BadReaderParamsError("Bad batch size: %s" % batch_size)
      except ValueError, e:
        raise errors.BadReaderParamsError("Bad batch size: %s" % e)
    # Validate the keys only parameter.
    try:
      bool(params.get(cls.KEYS_ONLY_PARAM, False))
    except:
      raise errors.BadReaderParamsError("keys_only expects a boolean value but "
                                        "got %s",
                                        params[cls.KEYS_ONLY_PARAM])
    # Validate the namespace parameter.
    if cls.NAMESPACE_PARAM in params:
      if not isinstance(params[cls.NAMESPACE_PARAM],
                        (str, unicode, type(None))):
        raise errors.BadReaderParamsError("Expected a single namespace string")

    # Validate the filters parameter.
    if cls.FILTERS_PARAM in params:
      filters = params[cls.FILTERS_PARAM]
      if not isinstance(filters, list):
        raise errors.BadReaderParamsError("Expected list for filters parameter")
      for f in filters:
        if not isinstance(f, (tuple, list)):
          raise errors.BadReaderParamsError("Filter should be a tuple or list: "
                                            "%s", f)
        if len(f) != 3:
          raise errors.BadReaderParamsError("Filter should be a 3-tuple: %s", f)
        prop, op, _ = f
        if not isinstance(prop, basestring):
          raise errors.BadReaderParamsError("Property should be string: %s",
                                            prop)
        if not isinstance(op, basestring):
          raise errors.BadReaderParamsError("Operator should be string: %s", op)

  @classmethod
  def _get_raw_entity_kind(cls, entity_kind_or_model_classpath):
    """Returns the entity kind to use with low level datastore calls.

    Args:
      entity_kind_or_model_classpath: user specified entity kind or model
        classpath.

    Returns:
      the entity kind in str to use with low level datastore calls.
    """
    return entity_kind_or_model_classpath
