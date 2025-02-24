#!/usr/bin/env python
"""An abstract for a collection of key_range.KeyRange objects."""



from google.appengine.ext import key_range
from mapreduce import namespace_range


__all__ = [
    "KeyRangesFactory",
    "KeyRanges"]

# pylint: disable=g-bad-name


class KeyRangesFactory(object):
  """Factory for KeyRanges."""

  @classmethod
  def create_from_list(cls, list_of_key_ranges):
    """Create a KeyRanges object.

    Args:
      list_of_key_ranges: a list of key_range.KeyRange object.

    Returns:
      A _KeyRanges object.
    """
    return _KeyRangesFromList(list_of_key_ranges)

  @classmethod
  def create_from_ns_range(cls, ns_range):
    """Create a KeyRanges object.

    Args:
      ns_range: a namespace_range.NameSpace Range object.

    Returns:
      A _KeyRanges object.
    """
    return _KeyRangesFromNSRange(ns_range)

  @classmethod
  def from_json(cls, json):
    """Deserialize from json.

    Args:
      json: a dict of json compatible fields.

    Returns:
      a KeyRanges object.

    Raises:
      ValueError: if the json is invalid.
    """
    if json["name"] in _KEYRANGES_CLASSES:
      return _KEYRANGES_CLASSES[json["name"]].from_json(json)
    raise ValueError("Invalid json %s", json)


class KeyRanges(object):
  """An abstraction for a collection of key_range.KeyRange objects."""

  def __iter__(self):
    return self

  def next(self):
    """Iterator iteraface."""
    raise NotImplementedError()

  def to_json(self):
    return {"name": self.__class__.__name__}

  @classmethod
  def from_json(cls):
    raise NotImplementedError()

  def __eq__(self):
    raise NotImplementedError()

  def __str__(self):
    raise NotImplementedError()


class _KeyRangesFromList(KeyRanges):
  """Create KeyRanges from a list."""

  def __init__(self, list_of_key_ranges):
    self._key_ranges = list_of_key_ranges

  def __eq__(self, other):
    if not isinstance(other, self.__class__):
      return False
    return self._key_ranges == other._key_ranges

  def next(self):
    if self._key_ranges:
      return self._key_ranges.pop()
    raise StopIteration()

  def __str__(self):
    if len(self._key_ranges) == 1:
      return "Single KeyRange %s" % (self._key_ranges[0])
    if self._key_ranges:
      return "From %s to %s" % (self._key_ranges[0], self._key_ranges[-1])
    return "Empty KeyRange."

  def to_json(self):
    json = super(_KeyRangesFromList, self).to_json()
    json.update(
        {"list_of_key_ranges": [kr.to_json() for kr in self._key_ranges]})
    return json

  @classmethod
  def from_json(cls, json):
    return cls(
        [key_range.KeyRange.from_json(kr) for kr in json["list_of_key_ranges"]])


class _KeyRangesFromNSRange(KeyRanges):
  """Create KeyRanges from a namespace range."""

  def __init__(self, ns_range):
    """Init."""
    self._ns_range = ns_range
    if self._ns_range is not None:
      self._iter = iter(self._ns_range)
      self._last_ns = None

  def __eq__(self, other):
    if not isinstance(other, self.__class__):
      return False
    return self._ns_range == other._ns_range

  def __str__(self):
    return str(self._ns_range)

  def next(self):
    if self._ns_range is None:
      raise StopIteration()

    self._last_ns = self._iter.next()
    current_ns_range = self._ns_range
    if self._last_ns == self._ns_range.namespace_end:
      self._ns_range = None
    return key_range.KeyRange(namespace=self._last_ns,
                              _app=current_ns_range.app)

  def to_json(self):
    json = super(_KeyRangesFromNSRange, self).to_json()
    ns_range = self._ns_range
    if self._ns_range is not None and self._last_ns is not None:
      ns_range = ns_range.with_start_after(self._last_ns)
    if ns_range is not None:
      json.update({"ns_range": ns_range.to_json_object()})
    return json

  @classmethod
  def from_json(cls, json):
    if "ns_range" in json:
      return cls(
          namespace_range.NamespaceRange.from_json_object(json["ns_range"]))
    else:
      return cls(None)


_KEYRANGES_CLASSES = {
    _KeyRangesFromList.__name__: _KeyRangesFromList,
    _KeyRangesFromNSRange.__name__: _KeyRangesFromNSRange
}
