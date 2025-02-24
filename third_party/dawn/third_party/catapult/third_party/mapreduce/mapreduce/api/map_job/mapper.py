#!/usr/bin/env python
"""Interface for user defined mapper."""

from mapreduce import shard_life_cycle

# pylint: disable=protected-access
# pylint: disable=invalid-name


# TODO(user): Move common APIs to parent class.
class Mapper(shard_life_cycle._ShardLifeCycle):
  """Interface user's mapper should implement.

  Each shard initiates one instance. The instance is pickled
  and unpickled if a shard can't finish within the boundary of a single
  task (a.k.a a slice of the shard).

  Upon shard retry, a new instance will be used.

  Upon slice retry, the instance is unpickled from its state
  at the end of last slice.

  Be wary of the size of your mapper instances. They have to be persisted
  across slices.
  """

  def __init__(self):
    """Init.

    Init must not take additional arguments.
    """
    pass

  def __call__(self, slice_ctx, val):
    """Called for every value yielded by input reader.

    Normal case:
    This method is invoked exactly once on each input value. If an
    output writer is provided, this method can repeated call ctx.emit to
    write to output writer.

    On retries:
    Upon slice retry, some input value may have been processed by
    the previous attempt. This should not be a problem if your logic
    is idempotent (e.g write to datastore by key) but could be a
    problem otherwise (e.g write to cloud storage) and may result
    in duplicates.

    Advanced usage:
    Implementation can mimic combiner by tallying in-memory and
    and emit when memory is filling up or when end_slice() is called.
    CAUTION! Carefully tune to not to exceed memory limit or request deadline.

    Args:
      slice_ctx: map_job_context.SliceContext object.
      val: a single value yielded by your input reader. The type
        depends on the input reader. For example, some may yield a single
        datastore entity, others may yield a (int, str) tuple.
    """
    pass
