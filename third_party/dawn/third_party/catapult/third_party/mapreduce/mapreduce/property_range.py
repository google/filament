#!/usr/bin/env python
"""A class representing entity property range."""



# pylint: disable=g-bad-name
# pylint: disable=g-import-not-at-top

import datetime

from google.appengine.ext import ndb

from google.appengine.ext import db
from mapreduce import errors
from mapreduce import util

__all__ = [
    "should_shard_by_property_range",
    "PropertyRange"]


def should_shard_by_property_range(filters):
  """Returns whether these filters suggests sharding by property range.

  Args:
    filters: user supplied filters. Each filter should be a list or tuple of
      format (<property_name_as_str>, <query_operator_as_str>,
      <value_of_certain_type>). Value type is up to the property's type.

  Returns:
    True if these filters suggests sharding by property range. False
  Otherwise.
  """
  if not filters:
    return False

  for f in filters:
    if f[1] != "=":
      return True
  return False


class PropertyRange(object):
  """A class that represents a range on a db.Model's property.

  It supports splitting the range into n shards and generating a query that
  returns entities within that range.
  """

  def __init__(self,
               filters,
               model_class_path):
    """Init.

    Args:
      filters: user supplied filters. Each filter should be a list or tuple of
        format (<property_name_as_str>, <query_operator_as_str>,
        <value_of_certain_type>). Value type should satisfy the property's type.
      model_class_path: full path to the model class in str.
    """
    self.filters = filters
    self.model_class_path = model_class_path
    self.model_class = util.for_name(self.model_class_path)
    self.prop, self.start, self.end = self._get_range_from_filters(
        self.filters, self.model_class)

  @classmethod
  def _get_range_from_filters(cls, filters, model_class):
    """Get property range from filters user provided.

    This method also validates there is one and only one closed range on a
    single property.

    Args:
      filters: user supplied filters. Each filter should be a list or tuple of
        format (<property_name_as_str>, <query_operator_as_str>,
        <value_of_certain_type>). Value type should satisfy the property's type.
      model_class: the model class for the entity type to apply filters on.

    Returns:
      a tuple of (property, start_filter, end_filter). property is the model's
    field that the range is about. start_filter and end_filter define the
    start and the end of the range. (None, None, None) if no range is found.

    Raises:
      BadReaderParamsError: if any filter is invalid in any way.
    """
    if not filters:
      return None, None, None

    range_property = None
    start_val = None
    end_val = None
    start_filter = None
    end_filter = None
    for f in filters:
      prop, op, val = f

      if op in [">", ">=", "<", "<="]:
        if range_property and range_property != prop:
          raise errors.BadReaderParamsError(
              "Range on only one property is supported.")
        range_property = prop

        if val is None:
          raise errors.BadReaderParamsError(
              "Range can't be None in filter %s", f)

        if op in [">", ">="]:
          if start_val is not None:
            raise errors.BadReaderParamsError(
                "Operation %s is specified more than once.", op)
          start_val = val
          start_filter = f
        else:
          if end_val is not None:
            raise errors.BadReaderParamsError(
                "Operation %s is specified more than once.", op)
          end_val = val
          end_filter = f
      elif op != "=":
        raise errors.BadReaderParamsError(
            "Only < <= > >= = are supported as operation. Got %s", op)

    if not range_property:
      return None, None, None

    if start_val is None or end_val is None:
      raise errors.BadReaderParamsError(
          "Filter should contains a complete range on property %s",
          range_property)
    if issubclass(model_class, db.Model):
      property_obj = model_class.properties()[range_property]
    else:
      property_obj = (
          model_class._properties[  # pylint: disable=protected-access
              range_property])
    supported_properties = (
        _DISCRETE_PROPERTY_SPLIT_FUNCTIONS.keys() +
        _CONTINUOUS_PROPERTY_SPLIT_FUNCTIONS.keys())
    if not isinstance(property_obj, tuple(supported_properties)):
      raise errors.BadReaderParamsError(
          "Filtered property %s is not supported by sharding.", range_property)
    if not start_val < end_val:
      raise errors.BadReaderParamsError(
          "Start value %s should be smaller than end value %s",
          start_val, end_val)

    return property_obj, start_filter, end_filter

  def split(self, n):
    """Evenly split this range into contiguous, non overlapping subranges.

    Args:
      n: number of splits.

    Returns:
      a list of contiguous, non overlapping sub PropertyRanges. Maybe less than
    n when not enough subranges.
    """
    new_range_filters = []
    name = self.start[0]
    prop_cls = self.prop.__class__
    if prop_cls in _DISCRETE_PROPERTY_SPLIT_FUNCTIONS:
      splitpoints = _DISCRETE_PROPERTY_SPLIT_FUNCTIONS[prop_cls](
          self.start[2], self.end[2], n,
          self.start[1] == ">=", self.end[1] == "<=")
      start_filter = (name, ">=", splitpoints[0])
      for p in splitpoints[1:]:
        end_filter = (name, "<", p)
        new_range_filters.append([start_filter, end_filter])
        start_filter = (name, ">=", p)
    else:
      splitpoints = _CONTINUOUS_PROPERTY_SPLIT_FUNCTIONS[prop_cls](
          self.start[2], self.end[2], n)
      start_filter = self.start
      for p in splitpoints:
        end_filter = (name, "<", p)
        new_range_filters.append([start_filter, end_filter])
        start_filter = (name, ">=", p)
      new_range_filters.append([start_filter, self.end])

    for f in new_range_filters:
      f.extend(self._equality_filters)

    return [self.__class__(f, self.model_class_path) for f in new_range_filters]

  def make_query(self, ns):
    """Make a query of entities within this range.

    Query options are not supported. They should be specified when the query
    is run.

    Args:
      ns: namespace of this query.

    Returns:
      a db.Query or ndb.Query, depends on the model class's type.
    """
    if issubclass(self.model_class, db.Model):
      query = db.Query(self.model_class, namespace=ns)
      for f in self.filters:
        query.filter("%s %s" % (f[0], f[1]), f[2])
    else:
      query = self.model_class.query(namespace=ns)
      for f in self.filters:
        query = query.filter(ndb.FilterNode(*f))
    return query

  @property
  def _equality_filters(self):
    return [f for f in self.filters if f[1] == "="]

  def to_json(self):
    return {"filters": self.filters,
            "model_class_path": self.model_class_path}

  @classmethod
  def from_json(cls, json):
    return cls(json["filters"], json["model_class_path"])


def _split_datetime_property(start, end, n, include_start, include_end):
  # datastore stored datetime precision is microsecond.
  if not include_start:
    start += datetime.timedelta(microseconds=1)
  if include_end:
    end += datetime.timedelta(microseconds=1)
  delta = end - start
  stride = delta // n
  if stride <= datetime.timedelta():
    raise ValueError("Range too small to split: start %r end %r", start, end)
  splitpoints = [start]
  previous = start
  for _ in range(n-1):
    point = previous + stride
    if point == previous or point > end:
      continue
    previous = point
    splitpoints.append(point)
  if end not in splitpoints:
    splitpoints.append(end)
  return splitpoints


def _split_float_property(start, end, n):
  delta = float(end - start)
  stride = delta / n
  if stride <= 0:
    raise ValueError("Range too small to split: start %r end %r", start, end)
  splitpoints = []
  for i in range(1, n):
    splitpoints.append(start + i * stride)
  return splitpoints


def _split_integer_property(start, end, n, include_start, include_end):
  if not include_start:
    start += 1
  if include_end:
    end += 1
  delta = float(end - start)
  stride = delta / n
  if stride <= 0:
    raise ValueError("Range too small to split: start %r end %r", start, end)
  splitpoints = [start]
  previous = start
  for i in range(1, n):
    point = start + int(round(i * stride))
    if point == previous or point > end:
      continue
    previous = point
    splitpoints.append(point)
  if end not in splitpoints:
    splitpoints.append(end)
  return splitpoints


def _split_string_property(start, end, n, include_start, include_end):
  try:
    start = start.encode("ascii")
    end = end.encode("ascii")
  except UnicodeEncodeError, e:
    raise ValueError("Only ascii str is supported.", e)

  return _split_byte_string_property(start, end, n, include_start, include_end)


# The alphabet splitting supports.
_ALPHABET = "".join(chr(i) for i in range(128))
# String length determines how many unique strings we can choose from.
# We can't split into more shards than this: len(_ALPHABET)^_STRING_LENGTH
_STRING_LENGTH = 4


def _split_byte_string_property(start, end, n, include_start, include_end):
  # Get prefix, suffix, and the real start/end to split on.
  i = 0
  for i, (s, e) in enumerate(zip(start, end)):
    if s != e:
      break
  common_prefix = start[:i]
  start_suffix = start[i+_STRING_LENGTH:]
  end_suffix = end[i+_STRING_LENGTH:]
  start = start[i:i+_STRING_LENGTH]
  end = end[i:i+_STRING_LENGTH]

  # Convert str to ord.
  weights = _get_weights(_STRING_LENGTH)
  start_ord = _str_to_ord(start, weights)
  if not include_start:
    start_ord += 1
  end_ord = _str_to_ord(end, weights)
  if include_end:
    end_ord += 1

  # Do split.
  stride = (end_ord - start_ord) / float(n)
  if stride <= 0:
    raise ValueError("Range too small to split: start %s end %s", start, end)
  splitpoints = [_ord_to_str(start_ord, weights)]
  previous = start_ord
  for i in range(1, n):
    point = start_ord + int(round(stride * i))
    if point == previous or point > end_ord:
      continue
    previous = point
    splitpoints.append(_ord_to_str(point, weights))
  end_str = _ord_to_str(end_ord, weights)
  if end_str not in splitpoints:
    splitpoints.append(end_str)

  # Append suffix.
  splitpoints[0] += start_suffix
  splitpoints[-1] += end_suffix

  return [common_prefix + point for point in splitpoints]


def _get_weights(max_length):
  """Get weights for each offset in str of certain max length.

  Args:
    max_length: max length of the strings.

  Returns:
    A list of ints as weights.

  Example:
    If max_length is 2 and alphabet is "ab", then we have order "", "a", "aa",
  "ab", "b", "ba", "bb". So the weight for the first char is 3.
  """
  weights = [1]
  for i in range(1, max_length):
    weights.append(weights[i-1] * len(_ALPHABET) + 1)
  weights.reverse()
  return weights


def _str_to_ord(content, weights):
  """Converts a string to its lexicographical order.

  Args:
    content: the string to convert. Of type str.
    weights: weights from _get_weights.

  Returns:
    an int or long that represents the order of this string. "" has order 0.
  """
  ordinal = 0
  for i, c in enumerate(content):
    ordinal += weights[i] * _ALPHABET.index(c) + 1
  return ordinal


def _ord_to_str(ordinal, weights):
  """Reverse function of _str_to_ord."""
  chars = []
  for weight in weights:
    if ordinal == 0:
      return "".join(chars)
    ordinal -= 1
    index, ordinal = divmod(ordinal, weight)
    chars.append(_ALPHABET[index])
  return "".join(chars)


# discrete property split functions all have the same interface.
# They take start, end, shard_number n, include_start, include_end.
# They return at most n+1 points, forming n ranges.
# Each range should be include_start, exclude_end.
_DISCRETE_PROPERTY_SPLIT_FUNCTIONS = {
    db.DateTimeProperty: _split_datetime_property,
    db.IntegerProperty: _split_integer_property,
    db.StringProperty: _split_string_property,
    db.ByteStringProperty: _split_byte_string_property,
    # ndb.
    ndb.DateTimeProperty: _split_datetime_property,
    ndb.IntegerProperty: _split_integer_property,
    ndb.StringProperty: _split_string_property,
    ndb.BlobProperty: _split_byte_string_property
}

_CONTINUOUS_PROPERTY_SPLIT_FUNCTIONS = {
    db.FloatProperty: _split_float_property,
    # ndb.
    ndb.FloatProperty: _split_float_property,
}
