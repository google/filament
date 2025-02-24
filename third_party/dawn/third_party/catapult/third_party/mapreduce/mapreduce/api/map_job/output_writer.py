#!/usr/bin/env python
"""Output writer interface for map job."""

from mapreduce import errors
from mapreduce import json_util
from mapreduce import shard_life_cycle

# pylint: disable=protected-access
# pylint: disable=invalid-name


# Counter name for number of bytes written.
COUNTER_IO_WRITE_BYTES = "io-write-bytes"

# Counter name for time spent writing data in msec
COUNTER_IO_WRITE_MSEC = "io-write-msec"


class OutputWriter(shard_life_cycle._ShardLifeCycle, json_util.JsonMixin):
  """Abstract base class for output writers.

  OutputWriter's lifecycle:
    0) validate() is called to validate JobConfig.
    1) create() is called, which should create a new instance of output
       writer for the given shard
    2) beging_shard/end_shard/begin_slice/end_slice are called at the time
       implied by the names.
    3) from_json()/to_json() are used to persist writer's state across
       multiple slices.
    4) write() method is called with data yielded by JobConfig.mapper.
  """

  def __init__(self):
    self._slice_ctx = None

  @classmethod
  def validate(cls, job_config):
    """Validates relevant parameters.

    This method can validate fields which it deems relevant.

    Args:
      job_config: an instance of map_job.JobConfig.

    Raises:
      errors.BadWriterParamsError: required parameters are missing or invalid.
    """
    if job_config.output_writer_cls != cls:
      raise errors.BadWriterParamsError(
          "Expect output writer class %r, got %r." %
          (cls, job_config.output_writer_cls))

  @classmethod
  def from_json(cls, state):
    """Creates an instance of the OutputWriter for the given json state.

    No RPC should take place in this method. Use start_slice/end_slice instead.

    Args:
      state: The output writer state as returned by to_json.

    Returns:
      An instance of the OutputWriter that can resume writing.
    """
    raise NotImplementedError("from_json() not implemented in %s" % cls)

  def to_json(self):
    """Returns writer state.

    No RPC should take place in this method. Use start_slice/end_slice instead.

    Returns:
      A json-serializable state for the OutputWriter instance.
    """
    raise NotImplementedError("to_json() not implemented in %s" %
                              type(self))

  @classmethod
  def create(cls, shard_ctx):
    """Create new writer for a shard.

    Args:
      shard_ctx: map_job_context.ShardContext for this shard.
    """
    raise NotImplementedError("create() not implemented in %s" % cls)

  def write(self, data):
    """Write data.

    Args:
      data: actual data yielded from handler. User is responsible to match the
        type expected by this writer to the type yielded by mapper.
    """
    raise NotImplementedError("write() not implemented in %s" %
                              self.__class__)

  @classmethod
  def commit_output(cls, shard_ctx, iterator):
    """Saves output references when a shard finishes.

    Inside end_shard(), an output writer can optionally use this method
    to persist some references to the outputs from this shard
    (e.g a list of filenames)

    Args:
      shard_ctx: map_job_context.ShardContext for this shard.
      iterator: an iterator that yields json serializable
        references to the outputs from this shard.
        Contents from the iterator can be accessible later via
        map_job.Job.get_outputs.
    """
    # We accept an iterator just in case output references get too big.
    outs = tuple(iterator)
    shard_ctx._state.writer_state["outs"] = outs

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

  # TODO(user): Update recovery related method to not use *_spec.
  def _supports_slice_recovery(self, mapper_spec):
    """Whether this output writer supports slice recovery.

    Args:
      mapper_spec: instance of model.MapperSpec.

    Returns:
      True if it does. False otherwise.
    """
    return False

  # pylint: disable=unused-argument
  def _recover(self, mr_spec, shard_number, shard_attempt):
    """Create a new output writer instance from the old one.

    This method is called when _supports_slice_recovery returns True,
    and when there is a chance the old output writer instance is out of sync
    with its storage medium due to a retry of a slice. _recover should
    create a new instance based on the old one. When end_shard is called
    on the new instance, it could combine valid outputs from all instances
    to generate the final output. How the new instance maintains references
    to previous outputs is up to implementation.

    Any exception during recovery is subject to normal slice/shard retry.
    So recovery logic must be idempotent.

    Args:
      mr_spec: an instance of model.MapreduceSpec describing current job.
      shard_number: int shard number.
      shard_attempt: int shard attempt.

    Returns:
      a new instance of output writer.
    """
    raise NotImplementedError()
