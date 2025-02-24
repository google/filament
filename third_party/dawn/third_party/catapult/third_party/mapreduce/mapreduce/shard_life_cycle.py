#!/usr/bin/env python
"""Shard life cycle abstract class."""

# pylint: disable=protected-access
# pylint: disable=invalid-name


class _ShardLifeCycle(object):
  """Abstract class for objects that live along shard's life cycle.

  Objects that need to plug in business logic into a shard's life cycle
  should implement this interface.

  The life cycle is:
  * begin_shard is called at the beginning of every shard attempt.
  * begin_slice is called at the beginning of every slice attempt.
  * end_slice is called at the end of a slice. Slice may still fail
    after the call.
  * end_shard is called at the end of a shard. Shard may still fail
    after the call.

  All these methods are invoked as part of shard execution. So be careful
  not to perform long standing IO operations that may kill this request.
  """

  def begin_shard(self, shard_ctx):
    """Called at the beginning of a shard.

    This method may be called more than once due to shard and slice retry.
    Make it idempotent.

    Args:
      shard_ctx: map_job_context.ShardContext object.
    """
    pass

  def end_shard(self, shard_ctx):
    """Called at the end of a shard.

    This method may be called more than once due to shard and slice retry.
    Make it idempotent.

    If shard execution error out before reaching the end, this method
    won't be called.

    Args:
      shard_ctx: map_job_context.ShardContext object.
    """
    pass

  def begin_slice(self, slice_ctx):
    """Called at the beginning of a slice.

    This method may be called more than once due to slice retry.
    Make it idempotent.

    Args:
      slice_ctx: map_job_context.SliceContext object.
    """
    pass

  def end_slice(self, slice_ctx):
    """Called at the end of a slice.

    This method may be called more than once due to slice retry.
    Make it idempotent.

    If slice execution error out before reaching the end, this method
    won't be called.

    Args:
      slice_ctx: map_job_context.SliceContext object.
    """
    pass
