#!/usr/bin/env python
"""Input Reader interface for map job."""

from mapreduce import errors
from mapreduce import json_util
from mapreduce import shard_life_cycle

# pylint: disable=protected-access
# pylint: disable=invalid-name


class InputReader(shard_life_cycle._ShardLifeCycle, json_util.JsonMixin):
  """Abstract base class for input readers.

  InputReader's lifecycle:
  1. validate() is called to validate JobConfig.
  2. split_input is called to split inputs based on map_job.JobConfig.
     The class method creates a set of InputReader instances.
  3. beging_shard/end_shard/begin_slice/end_slice are called at the time
     implied by the names.
  4. next() is called by each shard on each instance. The output of next()
     is fed into JobConfig.mapper instance.
  5. to_json()/from_json() are used to persist reader's state across multiple
     slices.
  """

  # Counters.
  # Bytes read.
  COUNTER_IO_READ_BYTE = "io-read-byte"
  # Milliseconds spent reading data.
  COUNTER_IO_READ_MSEC = "io-read-msec"

  def __init__(self):
    self._slice_ctx = None

  def __iter__(self):
    return self

  def next(self):
    """Returns the next input from this input reader.

    Returns:
      The next input read by this input reader. The return value is
      fed into mapper.

    Raises:
      StopIteration when no more item is left.
    """
    raise NotImplementedError("next() not implemented in %s" % self.__class__)

  @classmethod
  def from_json(cls, state):
    """Creates an instance of the InputReader for the given state.

    Args:
      state: The InputReader state as returned by to_json.

    Returns:
      An instance of the InputReader that can resume iteration.
    """
    raise NotImplementedError("from_json() not implemented in %s" % cls)

  def to_json(self):
    """Returns input reader state for the remaining inputs.

    Returns:
      A json-serializable state for the InputReader.
    """
    raise NotImplementedError("to_json() not implemented in %s" %
                              self.__class__)

  @classmethod
  def split_input(cls, job_config):
    """Returns an iterator of input readers.

    This method returns a container of input readers,
    one for each shard. The container must have __iter__ defined.
    http://docs.python.org/2/reference/datamodel.html#object.__iter__

    This method should try to split inputs among readers evenly.

    Args:
      job_config: an instance of map_job.JobConfig.

    Returns:
      An iterator of input readers.
    """
    raise NotImplementedError("split_input() not implemented in %s" % cls)

  @classmethod
  def validate(cls, job_config):
    """Validates relevant parameters.

    This method can validate fields which it deems relevant.

    Args:
      job_config: an instance of map_job.JobConfig.

    Raises:
      errors.BadReaderParamsError: required parameters are missing or invalid.
    """
    if job_config.input_reader_cls != cls:
      raise errors.BadReaderParamsError(
          "Expect input reader class %r, got %r." %
          (cls, job_config.input_reader_cls))

  def begin_slice(self, slice_ctx):
    """Keeps an internal reference to slice_ctx.

    Args:
      slice_ctx: SliceContext singleton instance for this slice.
    """
    self._slice_ctx = slice_ctx

  def end_slice(self, slice_ctx):
    """Drops the internal reference to slice_ctx.

    Args:
      slice_ctx: SliceContext singleton instance for this slice.
    """
    self._slice_ctx = None

  @classmethod
  def params_to_json(cls, params):
    """Translates JobConfig.input_reader_params to json serializable format.

    For most reader, this may be an identity transformation.

    Args:
      params: JobConfig.input_reader_params.

    Returns:
      The json serializable format of params.
    """
    return params

  @classmethod
  def params_from_json(cls, json_params):
    """Reverse function of params_to_json."""
    return json_params
