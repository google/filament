#!/usr/bin/env python
"""Parameters to control Mapreduce."""

__all__ = ["CONFIG_NAMESPACE",
           "config"]

import pickle


# To break circular dependency and more.
# pylint: disable=g-import-not-at-top


# For the mapreduce in python 25 runtime, this import will fail.
# TODO(user): Remove all pipeline import protections after 25 mr defunct.
try:
  from pipeline import util as pipeline_util
except ImportError:
  pipeline_util = None

from google.appengine.api import lib_config

CONFIG_NAMESPACE = "mapreduce"


# pylint: disable=protected-access
# pylint: disable=invalid-name


class _JobConfigMeta(type):
  """Metaclass that controls class creation."""

  _OPTIONS = "_options"
  _REQUIRED = "_required"

  def __new__(mcs, classname, bases, class_dict):
    """Creates a _Config class and modifies its class dict.

    Args:
      classname: name of the class.
      bases: a list of base classes.
      class_dict: original class dict.

    Returns:
      A new _Config class. The modified class will have two fields.
      _options field is a dict from option name to _Option objects.
      _required field is a set of required option names.
    """
    options = {}
    required = set()
    for name, option in class_dict.iteritems():
      if isinstance(option, _Option):
        options[name] = option
        if option.required:
          required.add(name)

    for name in options:
      class_dict.pop(name)
    class_dict[mcs._OPTIONS] = options
    class_dict[mcs._REQUIRED] = required
    cls = type.__new__(mcs, classname, bases, class_dict)

    # Handle inheritance of _Config.
    if object not in bases:
      parent_options = {}
      # Update options from the root down.
      for c in reversed(cls.__mro__):
        if mcs._OPTIONS in c.__dict__:
          # Children override parent.
          parent_options.update(c.__dict__[mcs._OPTIONS])
        if mcs._REQUIRED in c.__dict__:
          required.update(c.__dict__[mcs._REQUIRED])
      for k, v in parent_options.iteritems():
        if k not in options:
          options[k] = v
    return cls


class _Option(object):
  """An option for _Config."""

  def __init__(self, kind, required=False, default_factory=None,
               can_be_none=False):
    """Init.

    Args:
      kind: type of the option.
      required: whether user is required to supply a value.
      default_factory: a factory, when called, returns the default value.
      can_be_none: whether value can be None.

    Raises:
      ValueError: if arguments aren't compatible.
    """
    if required and default_factory is not None:
      raise ValueError("No default_factory value when option is required.")
    self.kind = kind
    self.required = required
    self.default_factory = default_factory
    self.can_be_none = can_be_none


class _Config(object):
  """Root class for all per job configuration."""

  __metaclass__ = _JobConfigMeta

  def __init__(self, _lenient=False, **kwds):
    """Init.

    Args:
      _lenient: When true, no option is required.
      **kwds: keyword arguments for options and their values.
    """
    self._verify_keys(kwds, _lenient)
    self._set_values(kwds, _lenient)

  def _verify_keys(self, kwds, _lenient):
    keys = set()
    for k in kwds:
      if k not in self._options:
        raise ValueError("Option %s is not supported." % (k))
      keys.add(k)
    if not _lenient:
      missing = self._required - keys
      if missing:
        raise ValueError("Options %s are required." % tuple(missing))

  def _set_values(self, kwds, _lenient):
    for k, option in self._options.iteritems():
      v = kwds.get(k)
      if v is None and option.default_factory:
        v = option.default_factory()
      setattr(self, k, v)
      if _lenient:
        continue
      if v is None and option.can_be_none:
        continue
      if isinstance(v, type) and not issubclass(v, option.kind):
        raise TypeError(
            "Expect subclass of %r for option %s. Got %r" % (
                option.kind, k, v))
      if not isinstance(v, type) and not isinstance(v, option.kind):
        raise TypeError("Expect type %r for option %s. Got %r" % (
            option.kind, k, v))

  def __eq__(self, other):
    if not isinstance(other, self.__class__):
      return False
    return other.__dict__ == self.__dict__

  def __repr__(self):
    return str(self.__dict__)

  def to_json(self):
    return {"config": pickle.dumps(self)}

  @classmethod
  def from_json(cls, json):
    return pickle.loads(json["config"])


# TODO(user): Make more of these private.
class _ConfigDefaults(object):
  """Default configs.

  Do not change parameters whose names begin with _.

  SHARD_MAX_ATTEMPTS: Max attempts to execute a shard before giving up.

  TASK_MAX_ATTEMPTS: Max attempts to execute a task before dropping it. Task
    is any taskqueue task created by MR framework. A task is dropped
    when its X-AppEngine-TaskExecutionCount is bigger than this number.
    Dropping a task will cause abort on the entire MR job.

  TASK_MAX_DATA_PROCESSING_ATTEMPTS:
    Max times to execute a task when previous task attempts failed during
    data processing stage. An MR work task has three major stages:
    initial setup, data processing, and final checkpoint.
    Setup stage should be allowed to be retried more times than data processing
    stage: setup failures are caused by unavailable GAE services while
    data processing failures are mostly due to user function error out on
    certain input data. Thus, set TASK_MAX_ATTEMPTS higher than this parameter.

  QUEUE_NAME: Default queue for MR.

  SHARD_COUNT: Default shard count.

  PROCESSING_RATE_PER_SEC: Default rate of processed entities per second.

  BASE_PATH : Base path of mapreduce and pipeline handlers.
  """

  SHARD_MAX_ATTEMPTS = 4

  # Arbitrary big number.
  TASK_MAX_ATTEMPTS = 31

  TASK_MAX_DATA_PROCESSING_ATTEMPTS = 11

  QUEUE_NAME = "default"

  SHARD_COUNT = 8

  # Maximum number of mapper calls per second.
  # This parameter is useful for testing to force short slices.
  # Maybe make this a private constant instead.
  # If people want to rate limit their jobs, they can reduce shard count.
  PROCESSING_RATE_PER_SEC = 1000000

  # This path will be changed by build process when this is a part of SDK.
  BASE_PATH = "/mapreduce"

  # TODO(user): find a proper value for this.
  # The amount of time to perform scanning in one slice. New slice will be
  # scheduled as soon as current one takes this long.
  _SLICE_DURATION_SEC = 15

  # Delay between consecutive controller callback invocations.
  _CONTROLLER_PERIOD_SEC = 2


# TODO(user): changes this name to app_config
config = lib_config.register(CONFIG_NAMESPACE, _ConfigDefaults.__dict__)


# The following are constants that depends on the value of _config.
# They are constants because _config is completely initialized on the first
# request of an instance and will never change until user deploy a new version.
_DEFAULT_PIPELINE_BASE_PATH = config.BASE_PATH + "/pipeline"
# See b/11341023 for context.
_GCS_URLFETCH_TIMEOUT_SEC = 30
# If a lock has been held longer than this value, mapreduce will start to use
# logs API to check if the request has ended.
_LEASE_DURATION_SEC = config._SLICE_DURATION_SEC * 1.1
# In rare occasions, Logs API misses log entries. Thus
# if a lock has been held longer than this timeout, mapreduce assumes the
# request holding the lock has died, regardless of Logs API.
# 10 mins is taskqueue task timeout on a frontend.
_MAX_LEASE_DURATION_SEC = max(10 * 60 + 30, config._SLICE_DURATION_SEC * 1.5)
