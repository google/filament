#!/usr/bin/env python
"""Json related utilities."""
import copy
import datetime
import logging

try:
  import json
except ImportError:
  import simplejson as json

from google.appengine.api import datastore_errors
from google.appengine.api import datastore_types
from google.appengine.ext import db
from google.appengine.ext import ndb

# pylint: disable=invalid-name


class JsonEncoder(json.JSONEncoder):
  """MR customized json encoder."""

  TYPE_ID = "__mr_json_type"

  def default(self, o):
    """Inherit docs."""
    if type(o) in _TYPE_TO_ENCODER:
      encoder = _TYPE_TO_ENCODER[type(o)]
      json_struct = encoder(o)
      json_struct[self.TYPE_ID] = type(o).__name__
      return json_struct
    return super(JsonEncoder, self).default(o)


class JsonDecoder(json.JSONDecoder):
  """MR customized json decoder."""

  def __init__(self, **kwargs):
    if "object_hook" not in kwargs:
      kwargs["object_hook"] = self._dict_to_obj
    super(JsonDecoder, self).__init__(**kwargs)

  def _dict_to_obj(self, d):
    """Converts a dictionary of json object to a Python object."""
    if JsonEncoder.TYPE_ID not in d:
      return d

    type_name = d.pop(JsonEncoder.TYPE_ID)
    if type_name in _TYPE_NAME_TO_DECODER:
      decoder = _TYPE_NAME_TO_DECODER[type_name]
      return decoder(d)
    else:
      raise TypeError("Invalid type %s.", type_name)


_DATETIME_FORMAT = "%Y-%m-%d %H:%M:%S.%f"


def _json_encode_datetime(o):
  """Json encode a datetime object.

  Args:
    o: a datetime object.

  Returns:
    A dict of json primitives.
  """
  return {"isostr": o.strftime(_DATETIME_FORMAT)}


def _json_decode_datetime(d):
  """Converts a dict of json primitives to a datetime object."""
  return datetime.datetime.strptime(d["isostr"], _DATETIME_FORMAT)


def _register_json_primitive(object_type, encoder, decoder):
  """Extend what MR can json serialize.

  Args:
    object_type: type of the object.
    encoder: a function that takes in an object and returns a dict of
       json primitives.
    decoder: inverse function of encoder.
  """
  global _TYPE_TO_ENCODER
  global _TYPE_NAME_TO_DECODER
  if object_type not in _TYPE_TO_ENCODER:
    _TYPE_TO_ENCODER[object_type] = encoder
    _TYPE_NAME_TO_DECODER[object_type.__name__] = decoder


_TYPE_TO_ENCODER = {}
_TYPE_NAME_TO_DECODER = {}
_register_json_primitive(datetime.datetime,
                         _json_encode_datetime,
                         _json_decode_datetime)

# ndb.Key
def _JsonEncodeKey(o):
    """Json encode an ndb.Key object."""
    return {'key_string': o.urlsafe()}

def _JsonDecodeKey(d):
    """Json decode a ndb.Key object."""
    k_c = d['key_string']
    if isinstance(k_c, (list, tuple)):
        return ndb.Key(flat=k_c)
    return ndb.Key(urlsafe=d['key_string'])

_register_json_primitive(ndb.Key, _JsonEncodeKey, _JsonDecodeKey)


class JsonMixin(object):
  """Simple, stateless json utilities mixin.

  Requires class to implement two methods:
    to_json(self): convert data to json-compatible datastructure (dict,
      list, strings, numbers)
    @classmethod from_json(cls, json): load data from json-compatible structure.
  """

  def to_json_str(self):
    """Convert data to json string representation.

    Returns:
      json representation as string.
    """
    _json = self.to_json()
    try:
      return json.dumps(_json, sort_keys=True, cls=JsonEncoder)
    except:
      logging.exception("Could not serialize JSON: %r", _json)
      raise

  @classmethod
  def from_json_str(cls, json_str):
    """Convert json string representation into class instance.

    Args:
      json_str: json representation as string.

    Returns:
      New instance of the class with data loaded from json string.
    """
    return cls.from_json(json.loads(json_str, cls=JsonDecoder))


class JsonProperty(db.UnindexedProperty):
  """Property type for storing json representation of data.

  Requires data types to implement two methods:
    to_json(self): convert data to json-compatible datastructure (dict,
      list, strings, numbers)
    @classmethod from_json(cls, json): load data from json-compatible structure.
  """

  def __init__(self, data_type, default=None, **kwargs):
    """Constructor.

    Args:
      data_type: underlying data type as class.
      default: default value for the property. The value is deep copied
        fore each model instance.
      **kwargs: remaining arguments.
    """
    kwargs["default"] = default
    super(JsonProperty, self).__init__(**kwargs)
    self.data_type = data_type

  def get_value_for_datastore(self, model_instance):
    """Gets value for datastore.

    Args:
      model_instance: instance of the model class.

    Returns:
      datastore-compatible value.
    """
    value = super(JsonProperty, self).get_value_for_datastore(model_instance)
    if not value:
      return None
    json_value = value
    if not isinstance(value, dict):
      json_value = value.to_json()
    if not json_value:
      return None
    return datastore_types.Text(json.dumps(
        json_value, sort_keys=True, cls=JsonEncoder))

  def make_value_from_datastore(self, value):
    """Convert value from datastore representation.

    Args:
      value: datastore value.

    Returns:
      value to store in the model.
    """

    if value is None:
      return None
    _json = json.loads(value, cls=JsonDecoder)
    if self.data_type == dict:
      return _json
    return self.data_type.from_json(_json)

  def validate(self, value):
    """Validate value.

    Args:
      value: model value.

    Returns:
      Whether the specified value is valid data type value.

    Raises:
      BadValueError: when value is not of self.data_type type.
    """
    if value is not None and not isinstance(value, self.data_type):
      raise datastore_errors.BadValueError(
          "Property %s must be convertible to a %s instance (%s)" %
          (self.name, self.data_type, value))
    return super(JsonProperty, self).validate(value)

  def empty(self, value):
    """Checks if value is empty.

    Args:
      value: model value.

    Returns:
      True passed value is empty.
    """
    return not value

  def default_value(self):
    """Create default model value.

    If default option was specified, then it will be deeply copied.
    None otherwise.

    Returns:
      default model value.
    """
    if self.default:
      return copy.deepcopy(self.default)
    else:
      return None
