#!/usr/bin/env python
"""User API for controlling Map job execution."""

from google.appengine.api import taskqueue
from google.appengine.datastore import datastore_rpc
from google.appengine.ext import db
from mapreduce import model
from mapreduce import util
from mapreduce.api.map_job import map_job_config

# pylint: disable=g-bad-name
# pylint: disable=protected-access


class Job(object):
  """The job submitter's view of the job.

  The class allows user to submit a job, control a submitted job,
  query its state and result.
  """

  RUNNING = "running"
  FAILED = model.MapreduceState.RESULT_FAILED
  ABORTED = model.MapreduceState.RESULT_ABORTED
  SUCCESS = model.MapreduceState.RESULT_SUCCESS

  STATUS_ENUM = [RUNNING, FAILED, ABORTED, SUCCESS]

  def __init__(self, state=None):
    """Init the job instance representing the job with id job_id.

    Do not directly call this method. Use class methods to construct
    new instances.

    Args:
      state: model.MapreduceState.
    """
    self._state = state

    self.job_config = map_job_config.JobConfig._to_map_job_config(
        state.mapreduce_spec,
        queue_name=state.mapreduce_spec.params.get("queue_name"))

  @classmethod
  def get_job_by_id(cls, job_id=None):
    """Gets the job instance representing the job with id job_id.

    Args:
      job_id: a job id, job_config.job_id, of a submitted job.

    Returns:
      A Job instance for job_id.
    """
    state = cls.__get_state_by_id(job_id)
    return cls(state)

  def get_status(self):
    """Get status enum.

    Returns:
      One of the status enum.
    """
    self.__update_state()
    if self._state.active:
      return self.RUNNING
    else:
      return self._state.result_status

  def abort(self):
    """Aborts the job."""
    model.MapreduceControl.abort(self.job_config.job_id)

  def get_counters(self):
    """Get counters from this job.

    When a job is running, counter values won't be very accurate.

    Returns:
      An iterator that returns (counter_name, value) pairs of type
      (basestring, int)
    """
    self.__update_state()
    return self._state.counters_map.counters.iteritems()

  def get_counter(self, counter_name, default=0):
    """Get the value of the named counter from this job.

    When a job is running, counter values won't be very accurate.

    Args:
      counter_name: name of the counter in string.
      default: default value if the counter doesn't exist.

    Returns:
      Value in int of the named counter.
    """
    self.__update_state()
    return self._state.counters_map.get(counter_name, default)

  def get_outputs(self):
    """Get outputs of this job.

    Should only call if status is SUCCESS.

    Yields:
      Iterators, one for each shard. Each iterator is
      from the argument of map_job.output_writer.commit_output.
    """
    assert self.SUCCESS == self.get_status()
    ss = model.ShardState.find_all_by_mapreduce_state(self._state)
    for s in ss:
      yield iter(s.writer_state.get("outs", []))

  @classmethod
  def submit(cls, job_config, in_xg_transaction=False):
    """Submit the job to run.

    Args:
      job_config: an instance of map_job.MapJobConfig.
      in_xg_transaction: controls what transaction scope to use to start this MR
        job. If True, there has to be an already opened cross-group transaction
        scope. MR will use one entity group from it.
        If False, MR will create an independent transaction to start the job
        regardless of any existing transaction scopes.

    Returns:
      a Job instance representing the submitted job.
    """
    cls.__validate_job_config(job_config)
    mapper_spec = job_config._get_mapper_spec()

    # Create mr spec.
    mapreduce_params = job_config._get_mr_params()
    mapreduce_spec = model.MapreduceSpec(
        job_config.job_name,
        job_config.job_id,
        mapper_spec.to_json(),
        mapreduce_params,
        util._obj_to_path(job_config._hooks_cls))

    # Save states and enqueue task.
    if in_xg_transaction:
      propagation = db.MANDATORY
    else:
      propagation = db.INDEPENDENT

    state = None
    @db.transactional(propagation=propagation)
    def _txn():
      state = cls.__create_and_save_state(job_config, mapreduce_spec)
      cls.__add_kickoff_task(job_config, mapreduce_spec)
      return state

    state = _txn()
    return cls(state)

  def __update_state(self):
    """Fetches most up to date state from db."""
    # Only if the job was not in a terminal state.
    if self._state.active:
      self._state = self.__get_state_by_id(self.job_config.job_id)

  @classmethod
  def __get_state_by_id(cls, job_id):
    """Get job state by id.

    Args:
      job_id: job id.

    Returns:
      model.MapreduceState for the job.

    Raises:
      ValueError: if the job state is missing.
    """
    state = model.MapreduceState.get_by_job_id(job_id)
    if state is None:
      raise ValueError("Job state for job %s is missing." % job_id)
    return state

  @classmethod
  def __validate_job_config(cls, job_config):
    # Validate input reader and output writer.
    job_config.input_reader_cls.validate(job_config)
    if job_config.output_writer_cls:
      job_config.output_writer_cls.validate(job_config._get_mapper_spec())

  @classmethod
  def __create_and_save_state(cls, job_config, mapreduce_spec):
    """Save map job state to datastore.

    Save state to datastore so that UI can see it immediately.

    Args:
      job_config: map_job.JobConfig.
      mapreduce_spec: model.MapreduceSpec.

    Returns:
      model.MapreduceState for this job.
    """
    state = model.MapreduceState.create_new(job_config.job_id)
    state.mapreduce_spec = mapreduce_spec
    state.active = True
    state.active_shards = 0
    state.app_id = job_config._app
    config = datastore_rpc.Configuration(force_writes=job_config._force_writes)
    state.put(config=config)
    return state

  @classmethod
  def __add_kickoff_task(cls, job_config, mapreduce_spec):
    """Add kickoff task to taskqueue.

    Args:
      job_config: map_job.JobConfig.
      mapreduce_spec: model.MapreduceSpec,
    """
    params = {"mapreduce_id": job_config.job_id}
    # Task is not named so that it can be added within a transaction.
    kickoff_task = taskqueue.Task(
        # TODO(user): Perhaps make this url a computed field of job_config.
        url=job_config._base_path + "/kickoffjob_callback/" + job_config.job_id,
        headers=util._get_task_headers(job_config.job_id),
        params=params)
    if job_config._hooks_cls:
      hooks = job_config._hooks_cls(mapreduce_spec)
      try:
        hooks.enqueue_kickoff_task(kickoff_task, job_config.queue_name)
        return
      except NotImplementedError:
        pass
    kickoff_task.add(job_config.queue_name, transactional=True)

