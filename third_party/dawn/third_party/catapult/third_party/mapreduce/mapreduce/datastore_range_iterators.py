#!/usr/bin/env python
"""Helpers iterators for input_readers.DatastoreInputReader."""



# pylint: disable=g-bad-name
import itertools
from google.appengine.datastore import datastore_query
from google.appengine.datastore import datastore_rpc
from google.appengine.ext import db
from google.appengine.ext import key_range
from mapreduce import json_util
from mapreduce import key_ranges
from mapreduce import model
from mapreduce import namespace_range
from mapreduce import property_range
from mapreduce import util

__all__ = [
    "RangeIteratorFactory",
    "RangeIterator",
    "AbstractKeyRangeIterator",
    "KeyRangeModelIterator",
    "KeyRangeEntityIterator",
    "KeyRangeKeyIterator",
    "KeyRangeEntityProtoIterator"]


class RangeIteratorFactory(object):
  """Factory to create RangeIterator."""

  @classmethod
  def create_property_range_iterator(cls,
                                     p_range,
                                     ns_range,
                                     query_spec):
    """Create a _PropertyRangeModelIterator.

    Args:
      p_range: a property_range.PropertyRange object that defines the
        conditions entities should safisfy.
      ns_range: a namesrange.NamespaceRange object that defines the namespaces
        to examine.
      query_spec: a model.QuerySpec object that defines how to retrieve
        entities from datastore.

    Returns:
      a RangeIterator.
    """
    return _PropertyRangeModelIterator(p_range,
                                       ns_range,
                                       query_spec)

  @classmethod
  def create_multi_property_range_iterator(cls,
                                           p_range_iters):
    """Create a RangeIterator.

    Args:
      p_range_iters: a list of RangeIterator objects to chain together.

    Returns:
      a RangeIterator.
    """
    return _MultiPropertyRangeModelIterator(p_range_iters)

  @classmethod
  def create_key_ranges_iterator(cls,
                                 k_ranges,
                                 query_spec,
                                 key_range_iter_cls):
    """Create a _KeyRangesIterator.

    Args:
      k_ranges: a key_ranges._KeyRanges object.
      query_spec: a model.query_spec object that defines how to retrieve
        entities from datastore.
      key_range_iter_cls: the class that iterates over a single key range.
        The value yielded by this class is yielded.

    Returns:
      a RangeIterator.
    """
    return _KeyRangesIterator(k_ranges, query_spec, key_range_iter_cls)

  @classmethod
  def from_json(cls, json):
    return _RANGE_ITERATORS[json["name"]].from_json(json)


class RangeIterator(json_util.JsonMixin):
  """Interface for DatastoreInputReader helpers.

  Technically, RangeIterator is a container. It contains all datastore
  entities that fall under a certain range (key range or proprety range).
  It implements __iter__, which returns a generator that can iterate
  through entities. It also implements marshalling logics. Marshalling
  saves the state of the container so that any new generator created
  can resume where the old generator left off.

  Caveats:
    1. Calling next() on the generators may also modify the container.
    2. Marshlling after StopIteration is raised has undefined behavior.
  """

  def __iter__(self):
    """Iter.

    Yields:
      Iterates over datastore entities and yields some kind of value
        for each entity.
    """
    raise NotImplementedError()

  def __repr__(self):
    raise NotImplementedError()

  def to_json(self):
    """Serializes all states into json form.

    Returns:
      all states in json-compatible map.
    """
    raise NotImplementedError()

  @classmethod
  def from_json(cls, json):
    """Reverse of to_json."""
    raise NotImplementedError()


class _PropertyRangeModelIterator(RangeIterator):
  """Yields db/ndb model entities within a property range."""

  def __init__(self, p_range, ns_range, query_spec):
    """Init.

    Args:
      p_range: a property_range.PropertyRange object that defines the
        conditions entities should safisfy.
      ns_range: a namesrange.NamespaceRange object that defines the namespaces
        to examine.
      query_spec: a model.QuerySpec object that defines how to retrieve
        entities from datastore.
    """
    self._property_range = p_range
    self._ns_range = ns_range
    self._query_spec = query_spec
    self._cursor = None
    self._query = None

  def __repr__(self):
    return "PropertyRangeIterator for %s" % str(self._property_range)

  def __iter__(self):
    """Iterate over entities.

    Yields:
      db model entities or ndb model entities if the model is defined with ndb.
    """
    for ns in self._ns_range:
      self._query = self._property_range.make_query(ns)
      if isinstance(self._query, db.Query):
        if self._cursor:
          self._query.with_cursor(self._cursor)
        for model_instance in self._query.run(
            batch_size=self._query_spec.batch_size,
            keys_only=self._query_spec.keys_only):
          yield model_instance
      else:
        self._query = self._query.iter(batch_size=self._query_spec.batch_size,
                                       keys_only=self._query_spec.keys_only,
                                       start_cursor=self._cursor,
                                       produce_cursors=True)
        for model_instance in self._query:
          yield model_instance
      self._query = None
      self._cursor = None
      if ns != self._ns_range.namespace_end:
        self._ns_range = self._ns_range.with_start_after(ns)

  def to_json(self):
    """Inherit doc."""
    cursor = self._cursor
    if self._query is not None:
      if isinstance(self._query, db.Query):
        cursor = self._query.cursor()
      else:
        cursor = self._query.cursor_after()

    if cursor is None or isinstance(cursor, basestring):
      cursor_object = False
    else:
      cursor_object = True
      cursor = cursor.to_websafe_string()

    return {"property_range": self._property_range.to_json(),
            "query_spec": self._query_spec.to_json(),
            "cursor": cursor,
            "ns_range": self._ns_range.to_json_object(),
            "name": self.__class__.__name__,
            "cursor_object": cursor_object}

  # TODO(user): it sucks we need to handle cursor_to_str in many places.
  # In the long run, datastore adaptor refactor will take care of this as
  # we will only need to deal with low level datastore API after that.
  # Thus we will not add Cursor as a json primitive MR should understand.
  @classmethod
  def from_json(cls, json):
    """Inherit doc."""
    obj = cls(property_range.PropertyRange.from_json(json["property_range"]),
              namespace_range.NamespaceRange.from_json_object(json["ns_range"]),
              model.QuerySpec.from_json(json["query_spec"]))
    cursor = json["cursor"]
    # lint bug. Class method can access protected fields.
    # pylint: disable=protected-access
    if cursor and json["cursor_object"]:
      obj._cursor = datastore_query.Cursor.from_websafe_string(cursor)
    else:
      obj._cursor = cursor
    return obj


class _MultiPropertyRangeModelIterator(RangeIterator):
  """Yields db/ndb model entities within a list of disjoint property ranges."""

  def __init__(self, p_range_iters):
    """Init.

    Args:
      p_range_iters: a list of _PropertyRangeModelIterator objects to chain
      together.
    """
    self._iters = p_range_iters

  def __repr__(self):
    return "MultiPropertyRangeIterator combining %s" % str(
      [str(it) for it in self._iters])

  def __iter__(self):
    """Iterate over entities.

    Yields:
      db model entities or ndb model entities if the model is defined with ndb.
    """
    for model_instance in itertools.chain.from_iterable(self._iters):
      yield model_instance

  def to_json(self):
    """Inherit doc."""
    json = {"name": self.__class__.__name__,
            "num_ranges": len(self._iters)}

    for i in xrange(len(self._iters)):
      json_item = self._iters[i].to_json()
      query_spec = json_item["query_spec"]
      item_name = json_item["name"]
      # Delete and move one level up
      del json_item["query_spec"]
      del json_item["name"]
      json[str(i)] = json_item
    # Store once to save space
    json["query_spec"] = query_spec
    json["item_name"] = item_name

    return json

  @classmethod
  def from_json(cls, json):
    """Inherit doc."""
    num_ranges = int(json["num_ranges"])
    query_spec = json["query_spec"]
    item_name = json["item_name"]

    p_range_iters = []
    for i in xrange(num_ranges):
      json_item = json[str(i)]
      # Place query_spec, name back into each iterator
      json_item["query_spec"] = query_spec
      json_item["name"] = item_name
      p_range_iters.append(_PropertyRangeModelIterator.from_json(json_item))

    obj = cls(p_range_iters)
    return obj


class _KeyRangesIterator(RangeIterator):
  """Create an iterator over a key_ranges.KeyRanges object."""

  def __init__(self,
               k_ranges,
               query_spec,
               key_range_iter_cls):
    """Init.

    Args:
      k_ranges: a key_ranges._KeyRanges object.
      query_spec: a model.query_spec object that defines how to retrieve
        entities from datastore.
      key_range_iter_cls: the class that iterates over a single key range.
        The value yielded by this class is yielded.
    """
    self._key_ranges = k_ranges
    self._query_spec = query_spec
    self._key_range_iter_cls = key_range_iter_cls
    self._current_iter = None
    self._current_key_range = None

  def __repr__(self):
    return "KeyRangesIterator for %s" % str(self._key_ranges)

  def __iter__(self):
    while True:
      if self._current_iter:
        for o in self._current_iter:
          yield o

      try:
        k_range = self._key_ranges.next()
        self._current_iter = self._key_range_iter_cls(k_range,
                                                      self._query_spec)
      except StopIteration:
        self._current_iter = None
        break

  def to_json(self):
    """Inherit doc."""
    current_iter = None
    if self._current_iter:
      current_iter = self._current_iter.to_json()

    return {"key_ranges": self._key_ranges.to_json(),
            "query_spec": self._query_spec.to_json(),
            "current_iter": current_iter,
            "key_range_iter_cls": self._key_range_iter_cls.__name__,
            "name": self.__class__.__name__}

  @classmethod
  def from_json(cls, json):
    """Inherit doc."""
    key_range_iter_cls = _KEY_RANGE_ITERATORS[json["key_range_iter_cls"]]
    obj = cls(key_ranges.KeyRangesFactory.from_json(json["key_ranges"]),
              model.QuerySpec.from_json(json["query_spec"]),
              key_range_iter_cls)

    current_iter = None
    if json["current_iter"]:
      current_iter = key_range_iter_cls.from_json(json["current_iter"])
    # pylint: disable=protected-access
    obj._current_iter = current_iter
    return obj


# A map from class name to class of all RangeIterators.
_RANGE_ITERATORS = {
    _PropertyRangeModelIterator.__name__: _PropertyRangeModelIterator,
    _MultiPropertyRangeModelIterator.__name__: _MultiPropertyRangeModelIterator,
    _KeyRangesIterator.__name__: _KeyRangesIterator
    }


class AbstractKeyRangeIterator(json_util.JsonMixin):
  """Iterates over a single key_range.KeyRange and yields value for each key.

  All subclasses do the same thing: iterate over a single KeyRange.
  They do so using different APIs (db, ndb, datastore) to return entities
  of different types (db model, ndb model, datastore entity, raw proto).
  """

  def __init__(self, k_range, query_spec):
    """Init.

    Args:
      k_range: a key_range.KeyRange object that defines the entity keys to
        operate on. KeyRange object already contains a namespace.
      query_spec: a model.query_spec object that defines how to retrieve
        entities from datastore.
    """
    self._key_range = k_range
    self._query_spec = query_spec
    self._cursor = None
    self._query = None

  def __iter__(self):
    """Iter."""
    raise NotImplementedError()

  def _get_cursor(self):
    """Get cursor on current query iterator for serialization."""
    raise NotImplementedError()

  def to_json(self):
    """Serializes all states into json form.

    Returns:
      all states in json-compatible map.
    """
    cursor = self._get_cursor()
    cursor_object = False
    if cursor and isinstance(cursor, datastore_query.Cursor):
      cursor = cursor.to_websafe_string()
      cursor_object = True
    return {"key_range": self._key_range.to_json(),
            "query_spec": self._query_spec.to_json(),
            "cursor": cursor,
            "cursor_object": cursor_object}

  @classmethod
  def from_json(cls, json):
    """Reverse of to_json."""
    obj = cls(key_range.KeyRange.from_json(json["key_range"]),
              model.QuerySpec.from_json(json["query_spec"]))
    cursor = json["cursor"]
    # lint bug. Class method can access protected fields.
    # pylint: disable=protected-access
    if cursor and json["cursor_object"]:
      obj._cursor = datastore_query.Cursor.from_websafe_string(cursor)
    else:
      obj._cursor = cursor
    return obj


class KeyRangeModelIterator(AbstractKeyRangeIterator):
  """Yields db/ndb model entities with a key range."""

  def __iter__(self):
    self._query = self._key_range.make_ascending_query(
        util.for_name(self._query_spec.model_class_path),
        filters=self._query_spec.filters)

    if isinstance(self._query, db.Query):
      if self._cursor:
        self._query.with_cursor(self._cursor)
      for model_instance in self._query.run(
          batch_size=self._query_spec.batch_size,
          keys_only=self._query_spec.keys_only):
        yield model_instance
    else:
      self._query = self._query.iter(batch_size=self._query_spec.batch_size,
                                     keys_only=self._query_spec.keys_only,
                                     start_cursor=self._cursor,
                                     produce_cursors=True)
      for model_instance in self._query:
        yield model_instance

  def _get_cursor(self):
    if self._query is None:
      return self._cursor

    if isinstance(self._query, db.Query):
      return self._query.cursor()
    else:
      return self._query.cursor_after()


class KeyRangeEntityIterator(AbstractKeyRangeIterator):
  """Yields datastore.Entity type within a key range."""

  _KEYS_ONLY = False

  def __iter__(self):
    self._query = self._key_range.make_ascending_datastore_query(
        self._query_spec.entity_kind, filters=self._query_spec.filters)
    for entity in self._query.Run(config=datastore_query.QueryOptions(
        batch_size=self._query_spec.batch_size,
        keys_only=self._query_spec.keys_only or self._KEYS_ONLY,
        start_cursor=self._cursor)):
      yield entity

  def _get_cursor(self):
    if self._query is None:
      return self._cursor
    return self._query.GetCursor()


class KeyRangeKeyIterator(KeyRangeEntityIterator):
  """Yields datastore.Key type within a key range."""

  _KEYS_ONLY = True


class KeyRangeEntityProtoIterator(AbstractKeyRangeIterator):
  """Yields datastore.Entity's raw proto within a key range."""

  def __iter__(self):
    query = self._key_range.make_ascending_datastore_query(
        self._query_spec.entity_kind, filters=self._query_spec.filters)
    # get a connection without adapter.
    connection = datastore_rpc.Connection()
    query_options = datastore_query.QueryOptions(
        batch_size=self._query_spec.batch_size,
        start_cursor=self._cursor,
        produce_cursors=True)

    # Transform datastore.Query:
    # datastore.Query -> datastore_query.Query -> datastore_query.Batcher ->
    # datastore_query.ResultsIterator
    self._query = datastore_query.ResultsIterator(
        query.GetQuery().run(connection, query_options))
    for entity_proto in self._query:
      yield entity_proto

  def _get_cursor(self):
    if self._query is None:
      return self._cursor
    return self._query.cursor()


# TODO(user): update this map automatically using metaclass if needed.
# Ideally, we want a parameter in datastore input reader to control
# the return type.
_KEY_RANGE_ITERATORS = {
    KeyRangeModelIterator.__name__: KeyRangeModelIterator,
    KeyRangeEntityIterator.__name__: KeyRangeEntityIterator,
    KeyRangeKeyIterator.__name__: KeyRangeKeyIterator,
    KeyRangeEntityProtoIterator.__name__: KeyRangeEntityProtoIterator
}
