#!/usr/bin/env python
# Copyright 2010 Google Inc. All Rights Reserved.
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
#

"""Defines executor tasks handlers for MapReduce implementation."""



# pylint: disable=protected-access
# pylint: disable=g-bad-name

import datetime
import logging
import math
import os
import random
import sys
import time
import traceback
import zlib

try:
  import json
except ImportError:
  import simplejson as json

from google.appengine.ext import ndb

from google.appengine import runtime
from google.appengine.api import datastore_errors
from google.appengine.api import logservice
from google.appengine.api import taskqueue
from google.appengine.ext import db
from mapreduce import base_handler
from mapreduce import context
from mapreduce import errors
from mapreduce import input_readers
from mapreduce import map_job_context
from mapreduce import model
from mapreduce import operation
from mapreduce import output_writers
from mapreduce import parameters
from mapreduce import shard_life_cycle
from mapreduce import util
from mapreduce.api import map_job
from google.appengine.runtime import apiproxy_errors

# pylint: disable=g-import-not-at-top
try:
  import cloudstorage
  # In 25 runtime, the above code will be scrubbed to import the stub version
  # of cloudstorage. All occurences of the following if condition in MR
  # codebase is to tell it apart.
  # TODO(user): Remove after 25 runtime MR is abondoned.
  if hasattr(cloudstorage, "_STUB"):
    cloudstorage = None
except ImportError:
  cloudstorage = None  # CloudStorage library not available


# A guide to logging.
# log.critical: messages user absolutely should see, e.g. failed job.
# log.error: exceptions during processing user data, or unexpected
# errors detected by mr framework.
# log.warning: errors mr framework knows how to handle.
# log.info: other expected events.


# Set of strings of various test-injected faults.
_TEST_INJECTED_FAULTS = set()


def _run_task_hook(hooks, method, task, queue_name):
  """Invokes hooks.method(task, queue_name).

  Args:
    hooks: A hooks.Hooks instance or None.
    method: The name of the method to invoke on the hooks class e.g.
        "enqueue_kickoff_task".
    task: The taskqueue.Task to pass to the hook method.
    queue_name: The name of the queue to pass to the hook method.

  Returns:
    True if the hooks.Hooks instance handled the method, False otherwise.
  """
  if hooks is not None:
    try:
      getattr(hooks, method)(task, queue_name)
    except NotImplementedError:
      # Use the default task addition implementation.
      return False

    return True
  return False


class MapperWorkerCallbackHandler(base_handler.HugeTaskHandler):
  """Callback handler for mapreduce worker task."""

  # These directives instruct self.__return() how to set state and enqueue task.
  _TASK_DIRECTIVE = util._enum(
      # Task is running as expected.
      PROCEED_TASK="proceed_task",
      # Need to retry task. Lock was NOT acquired when the error occur.
      # Don't change payload or datastore.
      RETRY_TASK="retry_task",
      # Need to retry task. Lock was acquired when the error occurr.
      # Don't change payload or datastore.
      RETRY_SLICE="retry_slice",
      # Drop the task (due to duplicated task). Must log permanent drop.
      DROP_TASK="drop_task",
      # See handlers.MapperWorkerCallbackHandler._attempt_slice_recovery.
      RECOVER_SLICE="recover_slice",
      # Need to retry the shard.
      RETRY_SHARD="retry_shard",
      # Need to drop task and fail the shard. Log permanent failure.
      FAIL_TASK="fail_task",
      # Need to abort the shard.
      ABORT_SHARD="abort_shard")

  def __init__(self, *args):
    """Constructor."""
    super(MapperWorkerCallbackHandler, self).__init__(*args)
    self._time = time.time
    self.slice_context = None
    self.shard_context = None

  def _drop_gracefully(self):
    """Drop worker task gracefully.

    Set current shard_state to failed. Controller logic will take care of
    other shards and the entire MR.
    """
    shard_id = self.request.headers[util._MR_SHARD_ID_TASK_HEADER]
    mr_id = self.request.headers[util._MR_ID_TASK_HEADER]
    shard_state, mr_state = db.get([
        model.ShardState.get_key_by_shard_id(shard_id),
        model.MapreduceState.get_key_by_job_id(mr_id)])

    if shard_state and shard_state.active:
      shard_state.set_for_failure()
      config = util.create_datastore_write_config(mr_state.mapreduce_spec)
      shard_state.put(config=config)

  def _try_acquire_lease(self, shard_state, tstate):
    """Validate datastore and the task payload are consistent.

    If so, attempt to get a lease on this slice's execution.
    See model.ShardState doc on slice_start_time.

    Args:
      shard_state: model.ShardState from datastore.
      tstate: model.TransientShardState from taskqueue paylod.

    Returns:
      A _TASK_DIRECTIVE enum. PROCEED_TASK if lock is acquired.
    RETRY_TASK if task should be retried, DROP_TASK if task should
    be dropped. Only old tasks (comparing to datastore state)
    will be dropped. Future tasks are retried until they naturally
    become old so that we don't ever stuck MR.
    """
    # Controller will tally shard_states and properly handle the situation.
    if not shard_state:
      logging.warning("State not found for shard %s; Possible spurious task "
                      "execution. Dropping this task.",
                      tstate.shard_id)
      return self._TASK_DIRECTIVE.DROP_TASK

    if not shard_state.active:
      logging.warning("Shard %s is not active. Possible spurious task "
                      "execution. Dropping this task.", tstate.shard_id)
      logging.warning(str(shard_state))
      return self._TASK_DIRECTIVE.DROP_TASK

    # Validate shard retry count.
    if shard_state.retries > tstate.retries:
      logging.warning(
          "Got shard %s from previous shard retry %s. Possible spurious "
          "task execution. Dropping this task.",
          tstate.shard_id,
          tstate.retries)
      logging.warning(str(shard_state))
      return self._TASK_DIRECTIVE.DROP_TASK
    elif shard_state.retries < tstate.retries:
      # By the end of last slice, task enqueue succeeded but datastore commit
      # failed. That transaction will be retried and adding the same task
      # will pass.
      logging.warning(
          "ShardState for %s is behind slice. Waiting for it to catch up",
          shard_state.shard_id)
      return self._TASK_DIRECTIVE.RETRY_TASK

    # Validate slice id.
    # Taskqueue executes old successful tasks.
    if shard_state.slice_id > tstate.slice_id:
      logging.warning(
          "Task %s-%s is behind ShardState %s. Dropping task.""",
          tstate.shard_id, tstate.slice_id, shard_state.slice_id)
      return self._TASK_DIRECTIVE.DROP_TASK
    # By the end of last slice, task enqueue succeeded but datastore commit
    # failed. That transaction will be retried and adding the same task
    # will pass. User data is duplicated in this case.
    elif shard_state.slice_id < tstate.slice_id:
      logging.warning(
          "Task %s-%s is ahead of ShardState %s. Waiting for it to catch up.",
          tstate.shard_id, tstate.slice_id, shard_state.slice_id)
      return self._TASK_DIRECTIVE.RETRY_TASK

    # Check potential duplicated tasks for the same slice.
    # See model.ShardState doc.
    if shard_state.slice_start_time:
      countdown = self._wait_time(shard_state,
                                  parameters._LEASE_DURATION_SEC)
      if countdown > 0:
        logging.warning(
            "Last retry of slice %s-%s may be still running."
            "Will try again in %s seconds", tstate.shard_id, tstate.slice_id,
            countdown)
        # TODO(user): There might be a better way. Taskqueue's countdown
        # only applies to add new tasks, not retry of tasks.
        # Reduce contention.
        time.sleep(countdown)
        return self._TASK_DIRECTIVE.RETRY_TASK
      # lease could have expired. Verify with logs API.
      else:
        if self._wait_time(shard_state,
                           parameters._MAX_LEASE_DURATION_SEC):
          if not self._has_old_request_ended(shard_state):
            logging.warning(
                "Last retry of slice %s-%s is still in flight with request_id "
                "%s. Will try again later.", tstate.shard_id, tstate.slice_id,
                shard_state.slice_request_id)
            return self._TASK_DIRECTIVE.RETRY_TASK
        else:
          logging.warning(
              "Last retry of slice %s-%s has no log entry and has"
              "timed out after %s seconds",
              tstate.shard_id, tstate.slice_id,
              parameters._MAX_LEASE_DURATION_SEC)

    # Lease expired or slice_start_time not set.
    config = util.create_datastore_write_config(tstate.mapreduce_spec)
    @db.transactional(retries=5)
    def _tx():
      """Use datastore to set slice_start_time to now.

      If failed for any reason, raise error to retry the task (hence all
      the previous validation code). The task would die naturally eventually.

      Raises:
        Rollback: If the shard state is missing.

      Returns:
        A _TASK_DIRECTIVE enum.
      """
      fresh_state = model.ShardState.get_by_shard_id(tstate.shard_id)
      if not fresh_state:
        logging.warning("ShardState missing.")
        raise db.Rollback()
      if (fresh_state.active and
          fresh_state.slice_id == shard_state.slice_id and
          fresh_state.slice_start_time == shard_state.slice_start_time):
        shard_state.slice_start_time = datetime.datetime.now()
        shard_state.slice_request_id = os.environ.get("REQUEST_LOG_ID")
        shard_state.acquired_once = True
        shard_state.put(config=config)
        return self._TASK_DIRECTIVE.PROCEED_TASK
      else:
        logging.warning(
            "Contention on slice %s-%s execution. Will retry again.",
            tstate.shard_id, tstate.slice_id)
        # One proposer should win. In case all lost, back off arbitrarily.
        time.sleep(random.randrange(1, 5))
        return self._TASK_DIRECTIVE.RETRY_TASK

    return _tx()

  def _has_old_request_ended(self, shard_state):
    """Whether previous slice retry has ended according to Logs API.

    Args:
      shard_state: shard state.

    Returns:
      True if the request of previous slice retry has ended. False if it has
    not or unknown.
    """
    assert shard_state.slice_start_time is not None
    assert shard_state.slice_request_id is not None
    request_ids = [shard_state.slice_request_id]
    logs = None
    try:
      logs = list(logservice.fetch(request_ids=request_ids))
    except (apiproxy_errors.FeatureNotEnabledError,
        apiproxy_errors.CapabilityDisabledError) as e:
      # Managed VMs do not have access to the logservice API
      # See https://groups.google.com/forum/#!topic/app-engine-managed-vms/r8i65uiFW0w
      logging.warning("Ignoring exception: %s", e)

    if not logs or not logs[0].finished:
      return False
    return True

  def _wait_time(self, shard_state, secs, now=datetime.datetime.now):
    """Time to wait until slice_start_time is secs ago from now.

    Args:
      shard_state: shard state.
      secs: duration in seconds.
      now: a func that gets now.

    Returns:
      0 if no wait. A positive int in seconds otherwise. Always around up.
    """
    assert shard_state.slice_start_time is not None
    delta = now() - shard_state.slice_start_time
    duration = datetime.timedelta(seconds=secs)
    if delta < duration:
      return util.total_seconds(duration - delta)
    else:
      return 0

  def _try_free_lease(self, shard_state, slice_retry=False):
    """Try to free lease.

    A lightweight transaction to update shard_state and unset
    slice_start_time to allow the next retry to happen without blocking.
    We don't care if this fails or not because the lease will expire
    anyway.

    Under normal execution, _save_state_and_schedule_next is the exit point.
    It updates/saves shard state and schedules the next slice or returns.
    Other exit points are:
    1. _are_states_consistent: at the beginning of handle, checks
      if datastore states and the task are in sync.
      If not, raise or return.
    2. _attempt_slice_retry: may raise exception to taskqueue.
    3. _save_state_and_schedule_next: may raise exception when taskqueue/db
       unreachable.

    This handler should try to free the lease on every exceptional exit point.

    Args:
      shard_state: model.ShardState.
      slice_retry: whether to count this as a failed slice execution.
    """
    @db.transactional
    def _tx():
      fresh_state = model.ShardState.get_by_shard_id(shard_state.shard_id)
      if fresh_state and fresh_state.active:
        # Free lease.
        fresh_state.slice_start_time = None
        fresh_state.slice_request_id = None
        if slice_retry:
          fresh_state.slice_retries += 1
        fresh_state.put()
    try:
      _tx()
    # pylint: disable=broad-except
    except Exception, e:
      logging.warning(e)
      logging.warning(
          "Release lock for shard %s failed. Wait for lease to expire.",
          shard_state.shard_id)

  def _maintain_LC(self, obj, slice_id, last_slice=False, begin_slice=True,
                   shard_ctx=None, slice_ctx=None):
    """Makes sure shard life cycle interface are respected.

    Args:
      obj: the obj that may have implemented _ShardLifeCycle.
      slice_id: current slice_id
      last_slice: whether this is the last slice.
      begin_slice: whether this is the beginning or the end of a slice.
      shard_ctx: shard ctx for dependency injection. If None, it will be read
        from self.
      slice_ctx: slice ctx for dependency injection. If None, it will be read
        from self.
    """
    if obj is None or not isinstance(obj, shard_life_cycle._ShardLifeCycle):
      return

    shard_context = shard_ctx or self.shard_context
    slice_context = slice_ctx or self.slice_context
    if begin_slice:
      if slice_id == 0:
        obj.begin_shard(shard_context)
      obj.begin_slice(slice_context)
    else:
      obj.end_slice(slice_context)
      if last_slice:
        obj.end_shard(shard_context)

  def _lc_start_slice(self, tstate, slice_id):
    self._maintain_LC(tstate.output_writer, slice_id)
    self._maintain_LC(tstate.input_reader, slice_id)
    self._maintain_LC(tstate.handler, slice_id)

  def _lc_end_slice(self, tstate, slice_id, last_slice=False):
    self._maintain_LC(tstate.handler, slice_id, last_slice=last_slice,
                      begin_slice=False)
    self._maintain_LC(tstate.input_reader, slice_id, last_slice=last_slice,
                      begin_slice=False)
    self._maintain_LC(tstate.output_writer, slice_id, last_slice=last_slice,
                      begin_slice=False)

  def handle(self):
    """Handle request.

    This method has to be careful to pass the same ShardState instance to
    its subroutines calls if the calls mutate or read from ShardState.
    Note especially that Context instance caches and updates the ShardState
    instance.

    Returns:
      Set HTTP status code and always returns None.
    """
    # Reconstruct basic states.
    self._start_time = self._time()
    shard_id = self.request.headers[util._MR_SHARD_ID_TASK_HEADER]
    mr_id = self.request.headers[util._MR_ID_TASK_HEADER]
    spec = model.MapreduceSpec._get_mapreduce_spec(mr_id)
    shard_state, control = db.get([
        model.ShardState.get_key_by_shard_id(shard_id),
        model.MapreduceControl.get_key_by_job_id(mr_id),
    ])

    # Set context before any IO code is called.
    ctx = context.Context(spec, shard_state,
                          task_retry_count=self.task_retry_count())
    context.Context._set(ctx)

    # Unmarshall input reader, output writer, and other transient states.
    tstate = model.TransientShardState.from_request(self.request)

    # Try acquire a lease on the shard.
    if shard_state:
      is_this_a_retry = shard_state.acquired_once
    task_directive = self._try_acquire_lease(shard_state, tstate)
    if task_directive in (self._TASK_DIRECTIVE.RETRY_TASK,
                          self._TASK_DIRECTIVE.DROP_TASK):
      return self.__return(shard_state, tstate, task_directive)
    assert task_directive == self._TASK_DIRECTIVE.PROCEED_TASK

    # Abort shard if received signal.
    if control and control.command == model.MapreduceControl.ABORT:
      task_directive = self._TASK_DIRECTIVE.ABORT_SHARD
      return self.__return(shard_state, tstate, task_directive)

    # Retry shard if user disabled slice retry.
    if (is_this_a_retry and
        parameters.config.TASK_MAX_DATA_PROCESSING_ATTEMPTS <= 1):
      task_directive = self._TASK_DIRECTIVE.RETRY_SHARD
      return self.__return(shard_state, tstate, task_directive)

    # TODO(user): Find a better way to set these per thread configs.
    # E.g. what if user change it?
    util._set_ndb_cache_policy()

    job_config = map_job.JobConfig._to_map_job_config(
        spec,
        os.environ.get("HTTP_X_APPENGINE_QUEUENAME"))
    job_context = map_job_context.JobContext(job_config)
    self.shard_context = map_job_context.ShardContext(job_context, shard_state)
    self.slice_context = map_job_context.SliceContext(self.shard_context,
                                                      shard_state,
                                                      tstate)
    try:
      slice_id = tstate.slice_id
      self._lc_start_slice(tstate, slice_id)

      if shard_state.is_input_finished():
        self._lc_end_slice(tstate, slice_id, last_slice=True)
        # Finalize the stream and set status if there's no more input.
        if (tstate.output_writer and
            isinstance(tstate.output_writer, output_writers.OutputWriter)):
          # It's possible that finalization is successful but
          # saving state failed. In this case this shard will retry upon
          # finalization error.
          # TODO(user): make finalize method idempotent!
          tstate.output_writer.finalize(ctx, shard_state)
        shard_state.set_for_success()
        return self.__return(shard_state, tstate, task_directive)

      if is_this_a_retry:
        task_directive = self._attempt_slice_recovery(shard_state, tstate)
        if task_directive != self._TASK_DIRECTIVE.PROCEED_TASK:
          return self.__return(shard_state, tstate, task_directive)

      last_slice = self._process_inputs(
          tstate.input_reader, shard_state, tstate, ctx)

      self._lc_end_slice(tstate, slice_id)

      ctx.flush()

      if last_slice:
        # We're done processing data but we still need to finalize the output
        # stream. We save this condition in datastore and force a new slice.
        # That way if finalize fails no input data will be retried.
        shard_state.set_input_finished()
    # pylint: disable=broad-except
    except Exception, e:
      logging.warning("Shard %s got error.", shard_state.shard_id)
      logging.error(traceback.format_exc())

      # Fail fast.
      if type(e) is errors.FailJobError:
        logging.error("Got FailJobError.")
        task_directive = self._TASK_DIRECTIVE.FAIL_TASK
      else:
        task_directive = self._TASK_DIRECTIVE.RETRY_SLICE

    self.__return(shard_state, tstate, task_directive)

  def __return(self, shard_state, tstate, task_directive):
    """Handler should always call this as the last statement."""
    task_directive = self._set_state(shard_state, tstate, task_directive)
    self._save_state_and_schedule_next(shard_state, tstate, task_directive)
    context.Context._set(None)

  def _process_inputs(self,
                      input_reader,
                      shard_state,
                      tstate,
                      ctx):
    """Read inputs, process them, and write out outputs.

    This is the core logic of MapReduce. It reads inputs from input reader,
    invokes user specified mapper function, and writes output with
    output writer. It also updates shard_state accordingly.
    e.g. if shard processing is done, set shard_state.active to False.

    If errors.FailJobError is caught, it will fail this MR job.
    All other exceptions will be logged and raised to taskqueue for retry
    until the number of retries exceeds a limit.

    Args:
      input_reader: input reader.
      shard_state: shard state.
      tstate: transient shard state.
      ctx: mapreduce context.

    Returns:
      Whether this shard has finished processing all its input split.
    """
    processing_limit = self._processing_limit(tstate.mapreduce_spec)
    if processing_limit == 0:
      return

    finished_shard = True
    # Input reader may not be an iterator. It is only a container.
    iterator = iter(input_reader)

    while True:
      try:
        entity = iterator.next()
      except StopIteration:
        break
      # Reading input got exception. If we assume
      # 1. The input reader have done enough retries.
      # 2. The input reader can still serialize correctly after this exception.
      # 3. The input reader, upon resume, will try to re-read this failed
      #    record.
      # 4. This exception doesn't imply the input reader is permanently stuck.
      # we can serialize current slice immediately to avoid duplicated
      # outputs.
      # TODO(user): Validate these assumptions on all readers. MR should
      # also have a way to detect fake forward progress.

      if isinstance(entity, db.Model):
        shard_state.last_work_item = repr(entity.key())
      elif isinstance(entity, ndb.Model):
        shard_state.last_work_item = repr(entity.key)
      else:
        shard_state.last_work_item = repr(entity)[:100]

      processing_limit -= 1

      if not self._process_datum(
          entity, input_reader, ctx, tstate):
        finished_shard = False
        break
      elif processing_limit == 0:
        finished_shard = False
        break

    # Flush context and its pools.
    self.slice_context.incr(
        context.COUNTER_MAPPER_WALLTIME_MS,
        int((self._time() - self._start_time)*1000))

    return finished_shard

  def _process_datum(self, data, input_reader, ctx, transient_shard_state):
    """Process a single data piece.

    Call mapper handler on the data.

    Args:
      data: a datum to process.
      input_reader: input reader.
      ctx: mapreduce context
      transient_shard_state: transient shard state.

    Returns:
      True if scan should be continued, False if scan should be stopped.
    """
    if data is not input_readers.ALLOW_CHECKPOINT:
      self.slice_context.incr(context.COUNTER_MAPPER_CALLS)

      handler = transient_shard_state.handler

      if isinstance(handler, map_job.Mapper):
        handler(self.slice_context, data)
      else:
        if input_reader.expand_parameters:
          result = handler(*data)
        else:
          result = handler(data)

        if util.is_generator(result):
          for output in result:
            if isinstance(output, operation.Operation):
              output(ctx)
            else:
              output_writer = transient_shard_state.output_writer
              if not output_writer:
                logging.warning(
                    "Handler yielded %s, but no output writer is set.", output)
              else:
                output_writer.write(output)

    if self._time() - self._start_time >= parameters.config._SLICE_DURATION_SEC:
      return False
    return True

  def _set_state(self, shard_state, tstate, task_directive):
    """Set shard_state and tstate based on task_directive.

    Args:
      shard_state: model.ShardState for current shard.
      tstate: model.TransientShardState for current shard.
      task_directive: self._TASK_DIRECTIVE for current shard.

    Returns:
      A _TASK_DIRECTIVE enum.
      PROCEED_TASK if task should proceed normally.
      RETRY_SHARD if shard should be retried.
      RETRY_SLICE if slice should be retried.
      FAIL_TASK if sahrd should fail.
      RECOVER_SLICE if slice should be recovered.
      ABORT_SHARD if shard should be aborted.
      RETRY_TASK if task should be retried.
      DROP_TASK if task should be dropped.
    """
    if task_directive in (self._TASK_DIRECTIVE.RETRY_TASK,
                          self._TASK_DIRECTIVE.DROP_TASK):
      return task_directive

    if task_directive == self._TASK_DIRECTIVE.ABORT_SHARD:
      shard_state.set_for_abort()
      return task_directive

    if task_directive == self._TASK_DIRECTIVE.PROCEED_TASK:
      shard_state.advance_for_next_slice()
      tstate.advance_for_next_slice()
      return task_directive

    if task_directive == self._TASK_DIRECTIVE.RECOVER_SLICE:
      tstate.advance_for_next_slice(recovery_slice=True)
      shard_state.advance_for_next_slice(recovery_slice=True)
      return task_directive

    if task_directive == self._TASK_DIRECTIVE.RETRY_SLICE:
      task_directive = self._attempt_slice_retry(shard_state, tstate)
    if task_directive == self._TASK_DIRECTIVE.RETRY_SHARD:
      task_directive = self._attempt_shard_retry(shard_state, tstate)
    if task_directive == self._TASK_DIRECTIVE.FAIL_TASK:
      shard_state.set_for_failure()

    return task_directive

  def _save_state_and_schedule_next(self, shard_state, tstate, task_directive):
    """Save state and schedule task.

    Save shard state to datastore.
    Schedule next slice if needed.
    Set HTTP response code.
    No modification to any shard_state or tstate.

    Args:
      shard_state: model.ShardState for current shard.
      tstate: model.TransientShardState for current shard.
      task_directive: enum _TASK_DIRECTIVE.

    Returns:
      The task to retry if applicable.
    """
    spec = tstate.mapreduce_spec

    if task_directive == self._TASK_DIRECTIVE.DROP_TASK:
      return
    if task_directive in (self._TASK_DIRECTIVE.RETRY_SLICE,
                          self._TASK_DIRECTIVE.RETRY_TASK):
      # Set HTTP code to 500.
      return self.retry_task()
    elif task_directive == self._TASK_DIRECTIVE.ABORT_SHARD:
      logging.info("Aborting shard %d of job '%s'",
                   shard_state.shard_number, shard_state.mapreduce_id)
      task = None
    elif task_directive == self._TASK_DIRECTIVE.FAIL_TASK:
      logging.critical("Shard %s failed permanently.", shard_state.shard_id)
      task = None
    elif task_directive == self._TASK_DIRECTIVE.RETRY_SHARD:
      logging.warning("Shard %s is going to be attempted for the %s time.",
                      shard_state.shard_id,
                      shard_state.retries + 1)
      task = self._state_to_task(tstate, shard_state)
    elif task_directive == self._TASK_DIRECTIVE.RECOVER_SLICE:
      logging.warning("Shard %s slice %s is being recovered.",
                      shard_state.shard_id,
                      shard_state.slice_id)
      task = self._state_to_task(tstate, shard_state)
    else:
      assert task_directive == self._TASK_DIRECTIVE.PROCEED_TASK
      countdown = self._get_countdown_for_next_slice(spec)
      task = self._state_to_task(tstate, shard_state, countdown=countdown)

    # Prepare parameters for db transaction and taskqueue.
    queue_name = os.environ.get("HTTP_X_APPENGINE_QUEUENAME",
                                # For test only.
                                # TODO(user): Remove this.
                                "default")
    config = util.create_datastore_write_config(spec)

    @db.transactional(retries=5)
    def _tx():
      """The Transaction helper."""
      fresh_shard_state = model.ShardState.get_by_shard_id(tstate.shard_id)
      if not fresh_shard_state:
        raise db.Rollback()
      if (not fresh_shard_state.active or
          "worker_active_state_collision" in _TEST_INJECTED_FAULTS):
        logging.warning("Shard %s is not active. Possible spurious task "
                        "execution. Dropping this task.", tstate.shard_id)
        logging.warning("Datastore's %s", str(fresh_shard_state))
        logging.warning("Slice's %s", str(shard_state))
        return
      fresh_shard_state.copy_from(shard_state)
      fresh_shard_state.put(config=config)
      # Add task in the same datastore transaction.
      # This way we guarantee taskqueue is never behind datastore states.
      # Old tasks will be dropped.
      # Future task won't run until datastore states catches up.
      if fresh_shard_state.active:
        # Not adding task transactionally.
        # transactional enqueue requires tasks with no name.
        self._add_task(task, spec, queue_name)

    try:
      _tx()
    except (datastore_errors.Error,
            taskqueue.Error,
            runtime.DeadlineExceededError,
            apiproxy_errors.Error), e:
      logging.warning(
          "Can't transactionally continue shard. "
          "Will retry slice %s %s for the %s time.",
          tstate.shard_id,
          tstate.slice_id,
          self.task_retry_count() + 1)
      self._try_free_lease(shard_state)
      raise e

  def _attempt_slice_recovery(self, shard_state, tstate):
    """Recover a slice.

    This is run when a slice had been previously attempted and output
    may have been written. If an output writer requires slice recovery,
    we run those logic to remove output duplicates. Otherwise we just retry
    the slice.

    If recovery is needed, then the entire slice will be dedicated
    to recovery logic. No data processing will take place. Thus we call
    the slice "recovery slice". This is needed for correctness:
    An output writer instance can be out of sync from its physical
    medium only when the slice dies after acquring the shard lock but before
    committing shard state to db. The worst failure case is when
    shard state failed to commit after the NAMED task for the next slice was
    added. Thus, recovery slice has a special logic to increment current
    slice_id n to n+2. If the task for n+1 had been added, it will be dropped
    because it is behind shard state.

    Args:
      shard_state: an instance of Model.ShardState.
      tstate: an instance of Model.TransientShardState.

    Returns:
      _TASK_DIRECTIVE.PROCEED_TASK to continue with this retry.
      _TASK_DIRECTIVE.RECOVER_SLICE to recover this slice.
      The next slice will start at the same input as
      this slice but output to a new instance of output writer.
      Combining outputs from all writer instances is up to implementation.
    """
    mapper_spec = tstate.mapreduce_spec.mapper
    if not (tstate.output_writer and
            tstate.output_writer._supports_slice_recovery(mapper_spec)):
      return self._TASK_DIRECTIVE.PROCEED_TASK

    tstate.output_writer = tstate.output_writer._recover(
        tstate.mapreduce_spec, shard_state.shard_number,
        shard_state.retries + 1)
    return self._TASK_DIRECTIVE.RECOVER_SLICE

  def _attempt_shard_retry(self, shard_state, tstate):
    """Whether to retry shard.

    This method may modify shard_state and tstate to prepare for retry or fail.

    Args:
      shard_state: model.ShardState for current shard.
      tstate: model.TransientShardState for current shard.

    Returns:
      A _TASK_DIRECTIVE enum. RETRY_SHARD if shard should be retried.
    FAIL_TASK otherwise.
    """
    shard_attempts = shard_state.retries + 1

    if shard_attempts >= parameters.config.SHARD_MAX_ATTEMPTS:
      logging.warning(
          "Shard attempt %s exceeded %s max attempts.",
          shard_attempts, parameters.config.SHARD_MAX_ATTEMPTS)
      return self._TASK_DIRECTIVE.FAIL_TASK
    if tstate.output_writer and (
        not tstate.output_writer._supports_shard_retry(tstate)):
      logging.warning("Output writer %s does not support shard retry.",
                      tstate.output_writer.__class__.__name__)
      return self._TASK_DIRECTIVE.FAIL_TASK

    shard_state.reset_for_retry()
    logging.warning("Shard %s attempt %s failed with up to %s attempts.",
                    shard_state.shard_id,
                    shard_state.retries,
                    parameters.config.SHARD_MAX_ATTEMPTS)
    output_writer = None
    if tstate.output_writer:
      output_writer = tstate.output_writer.create(
          tstate.mapreduce_spec, shard_state.shard_number, shard_attempts + 1)
    tstate.reset_for_retry(output_writer)
    return self._TASK_DIRECTIVE.RETRY_SHARD

  def _attempt_slice_retry(self, shard_state, tstate):
    """Attempt to retry this slice.

    This method may modify shard_state and tstate to prepare for retry or fail.

    Args:
      shard_state: model.ShardState for current shard.
      tstate: model.TransientShardState for current shard.

    Returns:
      A _TASK_DIRECTIVE enum. RETRY_SLICE if slice should be retried.
    RETRY_SHARD if shard retry should be attempted.
    """
    if (shard_state.slice_retries + 1 <
        parameters.config.TASK_MAX_DATA_PROCESSING_ATTEMPTS):
      logging.warning(
          "Slice %s %s failed for the %s of up to %s attempts "
          "(%s of %s taskqueue execution attempts). "
          "Will retry now.",
          tstate.shard_id,
          tstate.slice_id,
          shard_state.slice_retries + 1,
          parameters.config.TASK_MAX_DATA_PROCESSING_ATTEMPTS,
          self.task_retry_count() + 1,
          parameters.config.TASK_MAX_ATTEMPTS)
      # Clear info related to current exception. Otherwise, the real
      # callstack that includes a frame for this method will show up
      # in log.
      sys.exc_clear()
      self._try_free_lease(shard_state, slice_retry=True)
      return self._TASK_DIRECTIVE.RETRY_SLICE

    if parameters.config.TASK_MAX_DATA_PROCESSING_ATTEMPTS > 0:
      logging.warning("Slice attempt %s exceeded %s max attempts.",
                      self.task_retry_count() + 1,
                      parameters.config.TASK_MAX_DATA_PROCESSING_ATTEMPTS)
    return self._TASK_DIRECTIVE.RETRY_SHARD

  @staticmethod
  def get_task_name(shard_id, slice_id, retry=0):
    """Compute single worker task name.

    Args:
      shard_id: shard id.
      slice_id: slice id.
      retry: current shard retry count.

    Returns:
      task name which should be used to process specified shard/slice.
    """
    # Prefix the task name with something unique to this framework's
    # namespace so we don't conflict with user tasks on the queue.
    return "appengine-mrshard-%s-%s-retry-%s" % (
        shard_id, slice_id, retry)

  def _get_countdown_for_next_slice(self, spec):
    """Get countdown for next slice's task.

    When user sets processing rate, we set countdown to delay task execution.

    Args:
      spec: model.MapreduceSpec

    Returns:
      countdown in int.
    """
    countdown = 0
    if self._processing_limit(spec) != -1:
      countdown = max(
          int(parameters.config._SLICE_DURATION_SEC -
              (self._time() - self._start_time)), 0)
    return countdown

  @classmethod
  def _state_to_task(cls,
                     tstate,
                     shard_state,
                     eta=None,
                     countdown=None):
    """Generate task for slice according to current states.

    Args:
      tstate: An instance of TransientShardState.
      shard_state: An instance of ShardState.
      eta: Absolute time when the MR should execute. May not be specified
        if 'countdown' is also supplied. This may be timezone-aware or
        timezone-naive.
      countdown: Time in seconds into the future that this MR should execute.
        Defaults to zero.

    Returns:
      A model.HugeTask instance for the slice specified by current states.
    """
    base_path = tstate.base_path

    task_name = MapperWorkerCallbackHandler.get_task_name(
        tstate.shard_id,
        tstate.slice_id,
        tstate.retries)

    headers = util._get_task_headers(tstate.mapreduce_spec.mapreduce_id)
    headers[util._MR_SHARD_ID_TASK_HEADER] = tstate.shard_id

    worker_task = model.HugeTask(
        url=base_path + "/worker_callback/" + tstate.shard_id,
        params=tstate.to_dict(),
        name=task_name,
        eta=eta,
        countdown=countdown,
        parent=shard_state,
        headers=headers)
    return worker_task

  @classmethod
  def _add_task(cls,
                worker_task,
                mapreduce_spec,
                queue_name):
    """Schedule slice scanning by adding it to the task queue.

    Args:
      worker_task: a model.HugeTask task for slice. This is NOT a taskqueue
        task.
      mapreduce_spec: an instance of model.MapreduceSpec.
      queue_name: Optional queue to run on; uses the current queue of
        execution or the default queue if unspecified.
    """
    if not _run_task_hook(mapreduce_spec.get_hooks(),
                          "enqueue_worker_task",
                          worker_task,
                          queue_name):
      try:
        # Not adding transactionally because worker_task has name.
        # Named task is not allowed for transactional add.
        worker_task.add(queue_name)
      except (taskqueue.TombstonedTaskError,
              taskqueue.TaskAlreadyExistsError), e:
        logging.warning("Task %r already exists. %s: %s",
                        worker_task.name,
                        e.__class__,
                        e)

  def _processing_limit(self, spec):
    """Get the limit on the number of map calls allowed by this slice.

    Args:
      spec: a Mapreduce spec.

    Returns:
      The limit as a positive int if specified by user. -1 otherwise.
    """
    processing_rate = float(spec.mapper.params.get("processing_rate", 0))
    slice_processing_limit = -1
    if processing_rate > 0:
      slice_processing_limit = int(math.ceil(
          parameters.config._SLICE_DURATION_SEC*processing_rate/
          int(spec.mapper.shard_count)))
    return slice_processing_limit

  # Deprecated. Only used by old test cases.
  # TODO(user): clean up tests.
  @classmethod
  def _schedule_slice(cls,
                      shard_state,
                      tstate,
                      queue_name=None,
                      eta=None,
                      countdown=None):
    """Schedule slice scanning by adding it to the task queue.

    Args:
      shard_state: An instance of ShardState.
      tstate: An instance of TransientShardState.
      queue_name: Optional queue to run on; uses the current queue of
        execution or the default queue if unspecified.
      eta: Absolute time when the MR should execute. May not be specified
        if 'countdown' is also supplied. This may be timezone-aware or
        timezone-naive.
      countdown: Time in seconds into the future that this MR should execute.
        Defaults to zero.
    """
    queue_name = queue_name or os.environ.get("HTTP_X_APPENGINE_QUEUENAME",
                                              "default")
    task = cls._state_to_task(tstate, shard_state, eta, countdown)
    cls._add_task(task, tstate.mapreduce_spec, queue_name)


class ControllerCallbackHandler(base_handler.HugeTaskHandler):
  """Supervises mapreduce execution.

  Is also responsible for gathering execution status from shards together.

  This task is "continuously" running by adding itself again to taskqueue if
  and only if mapreduce is still active. A mapreduce is active if it has
  actively running shards.
  """

  def __init__(self, *args):
    """Constructor."""
    super(ControllerCallbackHandler, self).__init__(*args)
    self._time = time.time

  def _drop_gracefully(self):
    """Gracefully drop controller task.

    This method is called when decoding controller task payload failed.
    Upon this we mark ShardState and MapreduceState as failed so all
    tasks can stop.

    Writing to datastore is forced (ignore read-only mode) because we
    want the tasks to stop badly, and if force_writes was False,
    the job would have never been started.
    """
    mr_id = self.request.headers[util._MR_ID_TASK_HEADER]
    state = model.MapreduceState.get_by_job_id(mr_id)
    if not state or not state.active:
      return

    state.active = False
    state.result_status = model.MapreduceState.RESULT_FAILED
    config = util.create_datastore_write_config(state.mapreduce_spec)
    puts = []
    for ss in model.ShardState.find_all_by_mapreduce_state(state):
      if ss.active:
        ss.set_for_failure()
        puts.append(ss)
        # Avoid having too many shard states in memory.
        if len(puts) > model.ShardState._MAX_STATES_IN_MEMORY:
          db.put(puts, config=config)
          puts = []
    db.put(puts, config=config)
    # Put mr_state only after all shard_states are put.
    db.put(state, config=config)

  def handle(self):
    """Handle request."""
    spec = model.MapreduceSpec.from_json_str(
        self.request.get("mapreduce_spec"))
    state, control = db.get([
        model.MapreduceState.get_key_by_job_id(spec.mapreduce_id),
        model.MapreduceControl.get_key_by_job_id(spec.mapreduce_id),
    ])

    if not state:
      logging.warning("State not found for MR '%s'; dropping controller task.",
                      spec.mapreduce_id)
      return
    if not state.active:
      logging.warning(
          "MR %r is not active. Looks like spurious controller task execution.",
          spec.mapreduce_id)
      self._clean_up_mr(spec)
      return

    shard_states = model.ShardState.find_all_by_mapreduce_state(state)
    self._update_state_from_shard_states(state, shard_states, control)

    if state.active:
      ControllerCallbackHandler.reschedule(
          state, spec, self.serial_id() + 1)

  def _update_state_from_shard_states(self, state, shard_states, control):
    """Update mr state by examing shard states.

    Args:
      state: current mapreduce state as MapreduceState.
      shard_states: an iterator over shard states.
      control: model.MapreduceControl entity.
    """
    # Initialize vars.
    state.active_shards, state.aborted_shards, state.failed_shards = 0, 0, 0
    total_shards = 0
    processed_counts = []
    processed_status = []
    state.counters_map.clear()

    # Tally across shard states once.
    for s in shard_states:
      total_shards += 1
      status = 'unknown'
      if s.active:
        state.active_shards += 1
        status = 'running'
      if s.result_status == model.ShardState.RESULT_SUCCESS:
        status = 'success'
      elif s.result_status == model.ShardState.RESULT_ABORTED:
        state.aborted_shards += 1
        status = 'aborted'
      elif s.result_status == model.ShardState.RESULT_FAILED:
        state.failed_shards += 1
        status = 'failed'

      # Update stats in mapreduce state by aggregating stats from shard states.
      state.counters_map.add_map(s.counters_map)
      processed_counts.append(s.counters_map.get(context.COUNTER_MAPPER_CALLS))
      processed_status.append(status)

    state.set_processed_counts(processed_counts, processed_status)
    state.last_poll_time = datetime.datetime.utcfromtimestamp(self._time())

    spec = state.mapreduce_spec

    if total_shards != spec.mapper.shard_count:
      logging.error("Found %d shard states. Expect %d. "
                    "Issuing abort command to job '%s'",
                    total_shards, spec.mapper.shard_count,
                    spec.mapreduce_id)
      # We issue abort command to allow shards to stop themselves.
      model.MapreduceControl.abort(spec.mapreduce_id)

    # If any shard is active then the mr is active.
    # This way, controller won't prematurely stop before all the shards have.
    state.active = bool(state.active_shards)
    if not control and (state.failed_shards or state.aborted_shards):
      # Issue abort command if there are failed shards.
      model.MapreduceControl.abort(spec.mapreduce_id)

    if not state.active:
      # Set final result status derived from shard states.
      if state.failed_shards or not total_shards:
        state.result_status = model.MapreduceState.RESULT_FAILED
      # It's important failed shards is checked before aborted shards
      # because failed shards will trigger other shards to abort.
      elif state.aborted_shards:
        state.result_status = model.MapreduceState.RESULT_ABORTED
      else:
        state.result_status = model.MapreduceState.RESULT_SUCCESS
      self._finalize_outputs(spec, state)
      self._finalize_job(spec, state)
    else:
      @db.transactional(retries=5)
      def _put_state():
        """The helper for storing the state."""
        fresh_state = model.MapreduceState.get_by_job_id(spec.mapreduce_id)
        # We don't check anything other than active because we are only
        # updating stats. It's OK if they are briefly inconsistent.
        if not fresh_state.active:
          logging.warning(
              "Job %s is not active. Looks like spurious task execution. "
              "Dropping controller task.", spec.mapreduce_id)
          return
        config = util.create_datastore_write_config(spec)
        state.put(config=config)

      _put_state()

  def serial_id(self):
    """Get serial unique identifier of this task from request.

    Returns:
      serial identifier as int.
    """
    return int(self.request.get("serial_id"))

  @classmethod
  def _finalize_outputs(cls, mapreduce_spec, mapreduce_state):
    """Finalize outputs.

    Args:
      mapreduce_spec: an instance of MapreduceSpec.
      mapreduce_state: an instance of MapreduceState.
    """
    # Only finalize the output writers if the job is successful.
    if (mapreduce_spec.mapper.output_writer_class() and
        mapreduce_state.result_status == model.MapreduceState.RESULT_SUCCESS):
      mapreduce_spec.mapper.output_writer_class().finalize_job(mapreduce_state)

  @classmethod
  def _finalize_job(cls, mapreduce_spec, mapreduce_state):
    """Finalize job execution.

    Invokes done callback and save mapreduce state in a transaction,
    and schedule necessary clean ups. This method is idempotent.

    Args:
      mapreduce_spec: an instance of MapreduceSpec
      mapreduce_state: an instance of MapreduceState
    """
    config = util.create_datastore_write_config(mapreduce_spec)
    queue_name = util.get_queue_name(mapreduce_spec.params.get(
        model.MapreduceSpec.PARAM_DONE_CALLBACK_QUEUE))
    done_callback = mapreduce_spec.params.get(
        model.MapreduceSpec.PARAM_DONE_CALLBACK)
    done_task = None
    if done_callback:
      done_task = taskqueue.Task(
          url=done_callback,
          headers=util._get_task_headers(mapreduce_spec.mapreduce_id,
                                         util.CALLBACK_MR_ID_TASK_HEADER),
          method=mapreduce_spec.params.get("done_callback_method", "POST"))

    @db.transactional(retries=5)
    def _put_state():
      """Helper to store state."""
      fresh_state = model.MapreduceState.get_by_job_id(
          mapreduce_spec.mapreduce_id)
      if not fresh_state.active:
        logging.warning(
            "Job %s is not active. Looks like spurious task execution. "
            "Dropping task.", mapreduce_spec.mapreduce_id)
        return
      mapreduce_state.put(config=config)
      # Enqueue done_callback if needed.
      if done_task and not _run_task_hook(
          mapreduce_spec.get_hooks(),
          "enqueue_done_task",
          done_task,
          queue_name):
        done_task.add(queue_name, transactional=True)

    _put_state()
    logging.info("Final result for job '%s' is '%s'",
                 mapreduce_spec.mapreduce_id, mapreduce_state.result_status)
    cls._clean_up_mr(mapreduce_spec)

  @classmethod
  def _clean_up_mr(cls, mapreduce_spec):
    FinalizeJobHandler.schedule(mapreduce_spec)

  @staticmethod
  def get_task_name(mapreduce_spec, serial_id):
    """Compute single controller task name.

    Args:
      mapreduce_spec: specification of the mapreduce.
      serial_id: id of the invocation as int.

    Returns:
      task name which should be used to process specified shard/slice.
    """
    # Prefix the task name with something unique to this framework's
    # namespace so we don't conflict with user tasks on the queue.
    return "appengine-mrcontrol-%s-%s" % (
        mapreduce_spec.mapreduce_id, serial_id)

  @staticmethod
  def controller_parameters(mapreduce_spec, serial_id):
    """Fill in  controller task parameters.

    Returned parameters map is to be used as task payload, and it contains
    all the data, required by controller to perform its function.

    Args:
      mapreduce_spec: specification of the mapreduce.
      serial_id: id of the invocation as int.

    Returns:
      string->string map of parameters to be used as task payload.
    """
    return {"mapreduce_spec": mapreduce_spec.to_json_str(),
            "serial_id": str(serial_id)}

  @classmethod
  def reschedule(cls,
                 mapreduce_state,
                 mapreduce_spec,
                 serial_id,
                 queue_name=None):
    """Schedule new update status callback task.

    Args:
      mapreduce_state: mapreduce state as model.MapreduceState
      mapreduce_spec: mapreduce specification as MapreduceSpec.
      serial_id: id of the invocation as int.
      queue_name: The queue to schedule this task on. Will use the current
        queue of execution if not supplied.
    """
    task_name = ControllerCallbackHandler.get_task_name(
        mapreduce_spec, serial_id)
    task_params = ControllerCallbackHandler.controller_parameters(
        mapreduce_spec, serial_id)
    if not queue_name:
      queue_name = os.environ.get("HTTP_X_APPENGINE_QUEUENAME", "default")

    controller_callback_task = model.HugeTask(
        url=(mapreduce_spec.params["base_path"] + "/controller_callback/" +
             mapreduce_spec.mapreduce_id),
        name=task_name, params=task_params,
        countdown=parameters.config._CONTROLLER_PERIOD_SEC,
        parent=mapreduce_state,
        headers=util._get_task_headers(mapreduce_spec.mapreduce_id))

    if not _run_task_hook(mapreduce_spec.get_hooks(),
                          "enqueue_controller_task",
                          controller_callback_task,
                          queue_name):
      try:
        controller_callback_task.add(queue_name)
      except (taskqueue.TombstonedTaskError,
              taskqueue.TaskAlreadyExistsError), e:
        logging.warning("Task %r with params %r already exists. %s: %s",
                        task_name, task_params, e.__class__, e)


class KickOffJobHandler(base_handler.TaskQueueHandler):
  """Taskqueue handler which kicks off a mapreduce processing.

  This handler is idempotent.

  Precondition:
    The Model.MapreduceState entity for this mr is already created and
    saved to datastore by StartJobHandler._start_map.

  Request Parameters:
    mapreduce_id: in string.
  """

  # Datastore key used to save json serialized input readers.
  _SERIALIZED_INPUT_READERS_KEY = "input_readers_for_mr_%s"

  def handle(self):
    """Handles kick off request."""
    # Get and verify mr state.
    mr_id = self.request.get("mapreduce_id")
    # Log the mr_id since this is started in an unnamed task
    logging.info("Processing kickoff for job %s", mr_id)
    state = model.MapreduceState.get_by_job_id(mr_id)
    if not self._check_mr_state(state, mr_id):
      return

    # Create input readers.
    readers, serialized_readers_entity = self._get_input_readers(state)
    if readers is None:
      # We don't have any data. Finish map.
      logging.warning("Found no mapper input data to process.")
      state.active = False
      state.result_status = model.MapreduceState.RESULT_SUCCESS
      ControllerCallbackHandler._finalize_job(
          state.mapreduce_spec, state)
      return False

    # Create output writers.
    self._setup_output_writer(state)

    # Save states and make sure we use the saved input readers for
    # subsequent operations.
    result = self._save_states(state, serialized_readers_entity)
    if result is None:
      readers, _ = self._get_input_readers(state)
    elif not result:
      return

    queue_name = self.request.headers.get("X-AppEngine-QueueName")
    KickOffJobHandler._schedule_shards(state.mapreduce_spec, readers,
                                       queue_name,
                                       state.mapreduce_spec.params["base_path"],
                                       state)

    ControllerCallbackHandler.reschedule(
        state, state.mapreduce_spec, serial_id=0, queue_name=queue_name)

  def _drop_gracefully(self):
    """See parent."""
    mr_id = self.request.get("mapreduce_id")
    logging.error("Failed to kick off job %s", mr_id)

    state = model.MapreduceState.get_by_job_id(mr_id)
    if not self._check_mr_state(state, mr_id):
      return

    # Issue abort command just in case there are running tasks.
    config = util.create_datastore_write_config(state.mapreduce_spec)
    model.MapreduceControl.abort(mr_id, config=config)

    # Finalize job and invoke callback.
    state.active = False
    state.result_status = model.MapreduceState.RESULT_FAILED
    ControllerCallbackHandler._finalize_job(state.mapreduce_spec, state)

  def _get_input_readers(self, state):
    """Get input readers.

    Args:
      state: a MapreduceState model.

    Returns:
      A tuple: (a list of input readers, a model._HugeTaskPayload entity).
    The payload entity contains the json serialized input readers.
    (None, None) when input reader inplitting returned no data to process.
    """
    serialized_input_readers_key = (self._SERIALIZED_INPUT_READERS_KEY %
                                    state.key().id_or_name())
    serialized_input_readers = model._HugeTaskPayload.get_by_key_name(
        serialized_input_readers_key, parent=state)

    # Initialize input readers.
    input_reader_class = state.mapreduce_spec.mapper.input_reader_class()
    split_param = state.mapreduce_spec.mapper
    if issubclass(input_reader_class, map_job.InputReader):
      split_param = map_job.JobConfig._to_map_job_config(
          state.mapreduce_spec,
          os.environ.get("HTTP_X_APPENGINE_QUEUENAME"))
    if serialized_input_readers is None:
      readers = input_reader_class.split_input(split_param)
    else:
      readers = [input_reader_class.from_json_str(_json) for _json in
                 json.loads(zlib.decompress(
                 serialized_input_readers.payload))]

    if not readers:
      return None, None

    # Update state and spec with actual shard count.
    state.mapreduce_spec.mapper.shard_count = len(readers)
    state.active_shards = len(readers)

    # Prepare to save serialized input readers.
    if serialized_input_readers is None:
      # Use mr_state as parent so it can be easily cleaned up later.
      serialized_input_readers = model._HugeTaskPayload(
          key_name=serialized_input_readers_key, parent=state)
      readers_json_str = [i.to_json_str() for i in readers]
      serialized_input_readers.payload = zlib.compress(json.dumps(
                                                       readers_json_str))
    return readers, serialized_input_readers

  def _setup_output_writer(self, state):
    if not state.writer_state:
      output_writer_class = state.mapreduce_spec.mapper.output_writer_class()
      if output_writer_class:
        output_writer_class.init_job(state)

  @db.transactional
  def _save_states(self, state, serialized_readers_entity):
    """Run transaction to save state.

    Args:
      state: a model.MapreduceState entity.
      serialized_readers_entity: a model._HugeTaskPayload entity containing
        json serialized input readers.

    Returns:
      False if a fatal error is encountered and this task should be dropped
    immediately. True if transaction is successful. None if a previous
    attempt of this same transaction has already succeeded.
    """
    mr_id = state.key().id_or_name()
    fresh_state = model.MapreduceState.get_by_job_id(mr_id)
    if not self._check_mr_state(fresh_state, mr_id):
      return False
    if fresh_state.active_shards != 0:
      logging.warning(
          "Mapreduce %s already has active shards. Looks like spurious task "
          "execution.", mr_id)
      return None
    config = util.create_datastore_write_config(state.mapreduce_spec)
    db.put([state, serialized_readers_entity], config=config)
    return True

  @classmethod
  def _schedule_shards(cls,
                       spec,
                       readers,
                       queue_name,
                       base_path,
                       mr_state):
    """Prepares shard states and schedules their execution.

    Even though this method does not schedule shard task and save shard state
    transactionally, it's safe for taskqueue to retry this logic because
    the initial shard_state for each shard is the same from any retry.
    This is an important yet reasonable assumption on model.ShardState.

    Args:
      spec: mapreduce specification as MapreduceSpec.
      readers: list of InputReaders describing shard splits.
      queue_name: The queue to run this job on.
      base_path: The base url path of mapreduce callbacks.
      mr_state: The MapReduceState of current job.
    """
    # Create shard states.
    shard_states = []
    for shard_number, input_reader in enumerate(readers):
      shard_state = model.ShardState.create_new(spec.mapreduce_id, shard_number)
      shard_state.shard_description = str(input_reader)
      shard_states.append(shard_state)

    # Retrieves already existing shard states.
    existing_shard_states = db.get(shard.key() for shard in shard_states)
    existing_shard_keys = set(shard.key() for shard in existing_shard_states
                              if shard is not None)

    # Save non existent shard states.
    # Note: we could do this transactionally if necessary.
    db.put((shard for shard in shard_states
            if shard.key() not in existing_shard_keys),
           config=util.create_datastore_write_config(spec))

    # Create output writers.
    writer_class = spec.mapper.output_writer_class()
    writers = [None] * len(readers)
    if writer_class:
      for shard_number, shard_state in enumerate(shard_states):
        writers[shard_number] = writer_class.create(
            mr_state.mapreduce_spec,
            shard_state.shard_number, shard_state.retries + 1,
            mr_state.writer_state)

    # Schedule ALL shard tasks.
    # Since each task is named, _add_task will fall back gracefully if a
    # task already exists.
    for shard_number, (input_reader, output_writer) in enumerate(
        zip(readers, writers)):
      shard_id = model.ShardState.shard_id_from_number(
          spec.mapreduce_id, shard_number)
      task = MapperWorkerCallbackHandler._state_to_task(
          model.TransientShardState(
              base_path, spec, shard_id, 0, input_reader, input_reader,
              output_writer=output_writer,
              handler=spec.mapper.handler),
          shard_states[shard_number])
      MapperWorkerCallbackHandler._add_task(task,
                                            spec,
                                            queue_name)

  @classmethod
  def _check_mr_state(cls, state, mr_id):
    """Check MapreduceState.

    Args:
      state: an MapreduceState instance.
      mr_id: mapreduce id.

    Returns:
      True if state is valid. False if not and this task should be dropped.
    """
    if state is None:
      logging.warning(
          "Mapreduce State for job %s is missing. Dropping Task.",
          mr_id)
      return False
    if not state.active:
      logging.warning(
          "Mapreduce %s is not active. Looks like spurious task "
          "execution. Dropping Task.", mr_id)
      return False
    return True


class StartJobHandler(base_handler.PostJsonHandler):
  """Command handler starts a mapreduce job.

  This handler allows user to start a mr via a web form. It's _start_map
  method can also be used independently to start a mapreduce.
  """

  def handle(self):
    """Handles start request."""
    # Mapper spec as form arguments.
    mapreduce_name = self._get_required_param("name")
    mapper_input_reader_spec = self._get_required_param("mapper_input_reader")
    mapper_handler_spec = self._get_required_param("mapper_handler")
    mapper_output_writer_spec = self.request.get("mapper_output_writer")
    mapper_params = self._get_params(
        "mapper_params_validator", "mapper_params.")
    params = self._get_params(
        "params_validator", "params.")

    # Default values.
    mr_params = map_job.JobConfig._get_default_mr_params()
    mr_params.update(params)
    if "queue_name" in mapper_params:
      mr_params["queue_name"] = mapper_params["queue_name"]

    # Set some mapper param defaults if not present.
    mapper_params["processing_rate"] = int(mapper_params.get(
        "processing_rate") or parameters.config.PROCESSING_RATE_PER_SEC)

    # Validate the Mapper spec, handler, and input reader.
    mapper_spec = model.MapperSpec(
        mapper_handler_spec,
        mapper_input_reader_spec,
        mapper_params,
        int(mapper_params.get("shard_count", parameters.config.SHARD_COUNT)),
        output_writer_spec=mapper_output_writer_spec)

    mapreduce_id = self._start_map(
        mapreduce_name,
        mapper_spec,
        mr_params,
        queue_name=mr_params["queue_name"],
        _app=mapper_params.get("_app"))
    self.json_response["mapreduce_id"] = mapreduce_id

  def _get_params(self, validator_parameter, name_prefix):
    """Retrieves additional user-supplied params for the job and validates them.

    Args:
      validator_parameter: name of the request parameter which supplies
        validator for this parameter set.
      name_prefix: common prefix for all parameter names in the request.

    Raises:
      Any exception raised by the 'params_validator' request parameter if
      the params fail to validate.

    Returns:
      The user parameters.
    """
    params_validator = self.request.get(validator_parameter)

    user_params = {}
    for key in self.request.arguments():
      if key.startswith(name_prefix):
        values = self.request.get_all(key)
        adjusted_key = key[len(name_prefix):]
        if len(values) == 1:
          user_params[adjusted_key] = values[0]
        else:
          user_params[adjusted_key] = values

    if params_validator:
      resolved_validator = util.for_name(params_validator)
      resolved_validator(user_params)

    return user_params

  def _get_required_param(self, param_name):
    """Get a required request parameter.

    Args:
      param_name: name of request parameter to fetch.

    Returns:
      parameter value

    Raises:
      errors.NotEnoughArgumentsError: if parameter is not specified.
    """
    value = self.request.get(param_name)
    if not value:
      raise errors.NotEnoughArgumentsError(param_name + " not specified")
    return value

  @classmethod
  def _start_map(cls,
                 name,
                 mapper_spec,
                 mapreduce_params,
                 queue_name,
                 eta=None,
                 countdown=None,
                 hooks_class_name=None,
                 _app=None,
                 in_xg_transaction=False):
    # pylint: disable=g-doc-args
    # pylint: disable=g-doc-return-or-yield
    """See control.start_map.

    Requirements for this method:
    1. The request that invokes this method can either be regular or
       from taskqueue. So taskqueue specific headers can not be used.
    2. Each invocation transactionally starts an isolated mapreduce job with
       a unique id. MapreduceState should be immediately available after
       returning. See control.start_map's doc on transactional.
    3. Method should be lightweight.
    """
    # Validate input reader.
    mapper_input_reader_class = mapper_spec.input_reader_class()
    mapper_input_reader_class.validate(mapper_spec)

    # Validate output writer.
    mapper_output_writer_class = mapper_spec.output_writer_class()
    if mapper_output_writer_class:
      mapper_output_writer_class.validate(mapper_spec)

    # Create a new id and mr spec.
    mapreduce_id = model.MapreduceState.new_mapreduce_id()
    mapreduce_spec = model.MapreduceSpec(
        name,
        mapreduce_id,
        mapper_spec.to_json(),
        mapreduce_params,
        hooks_class_name)

    # Validate mapper handler.
    ctx = context.Context(mapreduce_spec, None)
    context.Context._set(ctx)
    try:
      # pylint: disable=pointless-statement
      mapper_spec.handler
    finally:
      context.Context._set(None)

    # Save states and enqueue task.
    if in_xg_transaction:
      propagation = db.MANDATORY
    else:
      propagation = db.INDEPENDENT

    @db.transactional(propagation=propagation)
    def _txn():
      cls._create_and_save_state(mapreduce_spec, _app)
      cls._add_kickoff_task(mapreduce_params["base_path"], mapreduce_spec, eta,
                            countdown, queue_name)
    _txn()

    return mapreduce_id

  @classmethod
  def _create_and_save_state(cls, mapreduce_spec, _app):
    """Save mapreduce state to datastore.

    Save state to datastore so that UI can see it immediately.

    Args:
      mapreduce_spec: model.MapreduceSpec,
      _app: app id if specified. None otherwise.

    Returns:
      The saved Mapreduce state.
    """
    state = model.MapreduceState.create_new(mapreduce_spec.mapreduce_id)
    state.mapreduce_spec = mapreduce_spec
    state.active = True
    state.active_shards = 0
    if _app:
      state.app_id = _app
    config = util.create_datastore_write_config(mapreduce_spec)
    state.put(config=config)
    return state

  @classmethod
  def _add_kickoff_task(cls,
                        base_path,
                        mapreduce_spec,
                        eta,
                        countdown,
                        queue_name):
    """Enqueues a new kickoff task."""
    params = {"mapreduce_id": mapreduce_spec.mapreduce_id}
    # Task is not named so that it can be added within a transaction.
    kickoff_task = taskqueue.Task(
        url=base_path + "/kickoffjob_callback/" + mapreduce_spec.mapreduce_id,
        headers=util._get_task_headers(mapreduce_spec.mapreduce_id),
        params=params,
        eta=eta,
        countdown=countdown)
    hooks = mapreduce_spec.get_hooks()
    if hooks is not None:
      try:
        hooks.enqueue_kickoff_task(kickoff_task, queue_name)
        return
      except NotImplementedError:
        pass
    kickoff_task.add(queue_name, transactional=True)


class FinalizeJobHandler(base_handler.TaskQueueHandler):
  """Finalize map job by deleting all temporary entities."""

  def handle(self):
    mapreduce_id = self.request.get("mapreduce_id")
    mapreduce_state = model.MapreduceState.get_by_job_id(mapreduce_id)
    if mapreduce_state:
      config = (
          util.create_datastore_write_config(mapreduce_state.mapreduce_spec))
      keys = [model.MapreduceControl.get_key_by_job_id(mapreduce_id)]
      for ss in model.ShardState.find_all_by_mapreduce_state(mapreduce_state):
        keys.extend(list(
            model._HugeTaskPayload.all().ancestor(ss).run(keys_only=True)))
      keys.extend(list(model._HugeTaskPayload.all().ancestor(
          mapreduce_state).run(keys_only=True)))
      db.delete(keys, config=config)

  @classmethod
  def schedule(cls, mapreduce_spec):
    """Schedule finalize task.

    Args:
      mapreduce_spec: mapreduce specification as MapreduceSpec.
    """
    task_name = mapreduce_spec.mapreduce_id + "-finalize"
    finalize_task = taskqueue.Task(
        name=task_name,
        url=(mapreduce_spec.params["base_path"] + "/finalizejob_callback/" +
             mapreduce_spec.mapreduce_id),
        params={"mapreduce_id": mapreduce_spec.mapreduce_id},
        headers=util._get_task_headers(mapreduce_spec.mapreduce_id))
    queue_name = util.get_queue_name(None)
    if not _run_task_hook(mapreduce_spec.get_hooks(),
                          "enqueue_controller_task",
                          finalize_task,
                          queue_name):
      try:
        finalize_task.add(queue_name)
      except (taskqueue.TombstonedTaskError,
              taskqueue.TaskAlreadyExistsError), e:
        logging.warning("Task %r already exists. %s: %s",
                        task_name, e.__class__, e)


class CleanUpJobHandler(base_handler.PostJsonHandler):
  """Command to kick off tasks to clean up a job's data."""

  def handle(self):
    mapreduce_id = self.request.get("mapreduce_id")

    mapreduce_state = model.MapreduceState.get_by_job_id(mapreduce_id)
    if mapreduce_state:
      shard_keys = model.ShardState.calculate_keys_by_mapreduce_state(
          mapreduce_state)
      db.delete(shard_keys)
      db.delete(mapreduce_state)
    self.json_response["status"] = ("Job %s successfully cleaned up." %
                                    mapreduce_id)


class AbortJobHandler(base_handler.PostJsonHandler):
  """Command to abort a running job."""

  def handle(self):
    model.MapreduceControl.abort(self.request.get("mapreduce_id"))
    self.json_response["status"] = "Abort signal sent."
