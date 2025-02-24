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

"""Utility functions for use with the Google App Engine Pipeline API."""

__all__ = ["for_name",
           "JsonEncoder",
           "JsonDecoder"]

#pylint: disable=g-bad-name

import datetime
import inspect
import logging
import os

try:
  import json
except ImportError:
  import simplejson as json

# pylint: disable=protected-access


def _get_task_target():
  """Get the default target for a pipeline task.

  Current version id format is: user_defined_version.minor_version_number
  Current module id is just the module's name. It could be "default"

  Returns:
    A complete target name is of format version.module. If module is the
  default module, just version. None if target can not be determined.
  """
  # Break circular dependency.
  # pylint: disable=g-import-not-at-top
  import pipeline
  if pipeline._TEST_MODE:
    return None

  # Further protect against test cases that doesn't set env vars
  # propertly.
  if ("CURRENT_VERSION_ID" not in os.environ or
      "CURRENT_MODULE_ID" not in os.environ):
    logging.warning("Running Pipeline in non TEST_MODE but important "
                    "env vars are not set.")
    return None

  version = os.environ["CURRENT_VERSION_ID"].split(".")[0]
  module = os.environ["CURRENT_MODULE_ID"]
  if module == "default":
    return version
  return "%s.%s" % (version, module)


def for_name(fq_name, recursive=False):
  """Find class/function/method specified by its fully qualified name.

  Fully qualified can be specified as:
    * <module_name>.<class_name>
    * <module_name>.<function_name>
    * <module_name>.<class_name>.<method_name> (an unbound method will be
      returned in this case).

  for_name works by doing __import__ for <module_name>, and looks for
  <class_name>/<function_name> in module's __dict__/attrs. If fully qualified
  name doesn't contain '.', the current module will be used.

  Args:
    fq_name: fully qualified name of something to find

  Returns:
    class object.

  Raises:
    ImportError: when specified module could not be loaded or the class
    was not found in the module.
  """
  fq_name = str(fq_name)
  module_name = __name__
  short_name = fq_name

  if fq_name.rfind(".") >= 0:
    (module_name, short_name) = (fq_name[:fq_name.rfind(".")],
                                 fq_name[fq_name.rfind(".") + 1:])

  try:
    result = __import__(module_name, None, None, [short_name])
    return result.__dict__[short_name]
  except KeyError:
    # If we're recursively inside a for_name() chain, then we want to raise
    # this error as a key error so we can report the actual source of the
    # problem. If we're *not* recursively being called, that means the
    # module was found and the specific item could not be loaded, and thus
    # we want to raise an ImportError directly.
    if recursive:
      raise
    else:
      raise ImportError("Could not find '%s' on path '%s'" % (
                        short_name, module_name))
  except ImportError, e:
    # module_name is not actually a module. Try for_name for it to figure
    # out what's this.
    try:
      module = for_name(module_name, recursive=True)
      if hasattr(module, short_name):
        return getattr(module, short_name)
      else:
        # The module was found, but the function component is missing.
        raise KeyError()
    except KeyError:
      raise ImportError("Could not find '%s' on path '%s'" % (
                        short_name, module_name))
    except ImportError:
      # This means recursive import attempts failed, thus we will raise the
      # first ImportError we encountered, since it's likely the most accurate.
      pass
    # Raise the original import error that caused all of this, since it is
    # likely the real cause of the overall problem.
    raise


def is_generator_function(obj):
  """Return true if the object is a user-defined generator function.

  Generator function objects provides same attributes as functions.
  See isfunction.__doc__ for attributes listing.

  Adapted from Python 2.6.

  Args:
    obj: an object to test.

  Returns:
    true if the object is generator function.
  """
  CO_GENERATOR = 0x20
  return bool(((inspect.isfunction(obj) or inspect.ismethod(obj)) and
               obj.func_code.co_flags & CO_GENERATOR))


class JsonEncoder(json.JSONEncoder):
  """Pipeline customized json encoder."""

  TYPE_ID = "__pipeline_json_type"

  def default(self, o):
    """Inherit docs."""
    if type(o) in _TYPE_TO_ENCODER:
      encoder = _TYPE_TO_ENCODER[type(o)]
      json_struct = encoder(o)
      json_struct[self.TYPE_ID] = type(o).__name__
      return json_struct
    return super(JsonEncoder, self).default(o)


class JsonDecoder(json.JSONDecoder):
  """Pipeline customized json decoder."""

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
  """Extend what Pipeline can serialize.

  Args:
    object_type: type of the object.
    encoder: a function that takes in an object and returns
      a dict of json primitives.
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
