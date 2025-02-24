# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import functools
import itertools
import logging

from dashboard.common import timing

from google.appengine.ext import ndb
from google.appengine.ext import db

__all__ = (
    'PopulateTaskGraph',
    'TaskGraph',
    'TaskVertex',
    'Dependency',
    'Evaluate',
    'ExtendTaskGraph',
    'UpdateTask',
    'AppendTasklog',
)

TaskVertex = collections.namedtuple('TaskVertex',
                                    ('id', 'vertex_type', 'payload'))
Dependency = collections.namedtuple('Dependency', ('from_', 'to'))
TaskGraph = collections.namedtuple('TaskGraph', ('vertices', 'edges'))

# These InMemoryTask instances are meant to isolate the Task model which is
# actually persisted in Datastore.
InMemoryTask = collections.namedtuple(
    'InMemoryTask', ('id', 'task_type', 'payload', 'status', 'dependencies'))

VALID_TRANSITIONS = {
    'pending': {'ongoing', 'completed', 'failed', 'cancelled'},
    'ongoing': {'completed', 'failed', 'cancelled'},
    'cancelled': {'pending'},
    'completed': {'pending'},
    'failed': {'pending'},
}

# Traversal states used in the graph traversal. We use these as marks for when
# vertices are traversed, as how we would implement graph colouring in a graph
# traversal (like Depth First Search).
NOT_EVALUATED, CHILDREN_PENDING, EVALUATION_DONE = (0, 1, 2)

ReconstitutedTaskGraph = collections.namedtuple('ReconstitutedTaskGraph',
                                                ('terminal_tasks', 'tasks'))


class Error(Exception):
  pass


class InvalidAmendment(Error):
  pass


class TaskNotFound(Error):
  pass


class InvalidTransition(Error):
  pass


# These are internal-only models, used as an implementation detail of the
# execution engine.
class Task(ndb.Model):
  """A Task associated with a Pinpoint Job.

  Task instances are always associated with a Job. Tasks represent units of work
  that are in well-defined states. Updates to Task instances are transactional
  and need to be.
  """
  task_type = ndb.StringProperty(required=True)
  status = ndb.StringProperty(
      required=True, choices=list(VALID_TRANSITIONS.keys()))
  payload = ndb.JsonProperty(compressed=True, indexed=False)
  dependencies = ndb.KeyProperty(repeated=True, kind='Task')
  created = ndb.DateTimeProperty(required=True, auto_now_add=True)
  updated = ndb.DateTimeProperty(required=True, auto_now_add=True)

  def ToInMemoryTask(self):
    # We isolate the ndb model `Task` from the evaluator, to avoid accidentially
    # modifying the state in datastore.
    return InMemoryTask(
        id=self.key.id(),
        task_type=self.task_type,
        payload=self.payload,
        status=self.status,
        dependencies=[dep.id() for dep in self.dependencies])


class TaskLog(ndb.Model):
  """Log entries associated with Task instances.

  TaskLog instances are always associated with a Task. These entries are
  immutable once created.
  """
  timestamp = ndb.DateTimeProperty(
      required=True, auto_now_add=True, indexed=False)
  message = ndb.TextProperty()
  payload = ndb.JsonProperty(compressed=True, indexed=False)


@ndb.transactional(propagation=ndb.TransactionOptions.INDEPENDENT, retries=0)
def PopulateTaskGraph(job, graph):
  """Populate the Datastore with Task instances associated with a Job.

  The `graph` argument must have two properties: a collection of `TaskVertex`
  instances named `vertices` and a collection of `Dependency` instances named
  `dependencies`.
  """
  if job is None:
    raise ValueError('job must not be None.')

  job_key = job.key
  tasks = {
      v.id: Task(
          key=ndb.Key(Task, v.id, parent=job_key),
          task_type=v.vertex_type,
          payload=v.payload,
          status='pending') for v in graph.vertices
  }
  dependencies = set()
  for dependency in graph.edges:
    dependency_key = ndb.Key(Task, dependency.to, parent=job_key)
    if dependency not in dependencies:
      tasks[dependency.from_].dependencies.append(dependency_key)
      dependencies.add(dependency)

  ndb.put_multi(list(tasks.values()), use_cache=True)


@ndb.transactional(propagation=ndb.TransactionOptions.INDEPENDENT, retries=0)
def ExtendTaskGraph(job, vertices, dependencies):
  """Add new vertices and dependency links to the graph.

  Args:
    job: a dashboard.pinpoint.model.job.Job instance.
    vertices: an iterable of TaskVertex instances.
    dependencies: an iterable of Dependency instances.
  """
  if job is None:
    raise ValueError('job must not be None.')
  if not vertices and not dependencies:
    return

  job_key = job.key
  amendment_task_graph = {
      v.id: Task(
          key=ndb.Key(Task, v.id, parent=job_key),
          task_type=v.vertex_type,
          status='pending',
          payload=v.payload) for v in vertices
  }

  # Ensure that the keys we're adding are not in the graph yet.
  current_tasks = Task.query(ancestor=job_key).fetch()
  current_task_keys = set(t.key for t in current_tasks)
  new_task_keys = set(t.key for t in amendment_task_graph.values())
  overlap = new_task_keys & current_task_keys
  if overlap:
    raise InvalidAmendment('vertices (%r) already in task graph.' % (overlap,))

  # Then we add the dependencies.
  current_task_graph = {t.key.id(): t for t in current_tasks}
  handled_dependencies = set()
  update_filter = set(amendment_task_graph)
  for dependency in dependencies:
    dependency_key = ndb.Key(Task, dependency.to, parent=job_key)
    if dependency not in handled_dependencies:
      current_task = current_task_graph.get(dependency.from_)
      amendment_task = amendment_task_graph.get(dependency.from_)
      if current_task is None and amendment_task is None:
        raise InvalidAmendment('dependency `from` (%s) not in amended graph.' %
                               (dependency.from_,))
      if current_task:
        current_task_graph[dependency.from_].dependencies.append(dependency_key)
      if amendment_task:
        amendment_task_graph[dependency.from_].dependencies.append(
            dependency_key)
      handled_dependencies.add(dependency)
      update_filter.add(dependency.from_)

  ndb.put_multi(
      itertools.chain(
          list(amendment_task_graph.values()),
          [t for id_, t in current_task_graph.items() if id_ in update_filter]),
      use_cache=True)


@ndb.transactional(propagation=ndb.TransactionOptions.INDEPENDENT, retries=0)
def UpdateTask(job, task_id, new_state=None, payload=None):
  """Update a task.

  This enforces that the status transitions are semantically correct, where only
  the transitions defined in the VALID_TRANSITIONS map are allowed.

  When either new_state or payload are not None, this function performs the
  update transactionally. At least one of `new_state` or `payload` must be
  provided in calls to this function.
  """
  if new_state is None and payload is None:
    raise ValueError('Set one of `new_state` or `payload`.')

  if new_state and new_state not in VALID_TRANSITIONS:
    raise InvalidTransition('Unknown state: %s' % (new_state,))

  task = Task.get_by_id(task_id, parent=job.key)
  if not task:
    raise TaskNotFound('Task with id "%s" not found for job "%s".' %
                       (task_id, job.job_id))

  if new_state:
    valid_transitions = VALID_TRANSITIONS.get(task.status)
    if new_state not in valid_transitions:
      raise InvalidTransition(
          'Attempting transition from "%s" to "%s" not in %s; task = %s' %
          (task.status, new_state, valid_transitions, task))
    task.status = new_state

  if payload:
    task.payload = payload

  task.put()


def LogStateTransitionFailures(wrapped_action):
  """Decorator to log state transition failures.

  This is a convenience decorator to handle state transition failures, and
  suppress further exception propagation of the transition failure.
  """

  @functools.wraps(wrapped_action)
  def ActionWrapper(*args, **kwargs):
    try:
      return wrapped_action(*args, **kwargs)
    except InvalidTransition as e:
      logging.error('State transition failed: %s', e)
      return None
    except db.TransactionFailedError as e:
      logging.error('Transaction failed: %s', e)
      return None

  return ActionWrapper


@ndb.transactional(propagation=ndb.TransactionOptions.INDEPENDENT, retries=0)
def AppendTasklog(job, task_id, message, payload):
  task_log = TaskLog(
      parent=ndb.Key(Task, task_id, parent=job.key),
      message=message,
      payload=payload)
  task_log.put()


@ndb.transactional(propagation=ndb.TransactionOptions.INDEPENDENT, retries=0)
def _LoadTaskGraph(job):
  with timing.WallTimeLogger('ExecutionEngine:_LoadTaskGraph'):
    tasks = Task.query(ancestor=job.key).fetch()
    # The way we get the terminal tasks is by looking at tasks where nothing
    # depends on them.
    has_dependents = set()
    for task in tasks:
      has_dependents |= set(task.dependencies)
    terminal_tasks = [t.key for t in tasks if t.key not in has_dependents]
    return ReconstitutedTaskGraph(
        terminal_tasks=terminal_tasks, tasks={task.key: task for task in tasks})


class NoopAction:

  @staticmethod
  def __str__():
    return 'NoopAction()'

  @staticmethod
  def __call__(_):
    pass


@ndb.non_transactional
@timing.TimeWall('ExecutionEngine:Evaluate')
def Evaluate(job, event, evaluator):
  """Applies an evaluator given a task in the task graph and an event as input.

  This function implements a depth-first search traversal of the task graph and
  applies the `evaluator` given a task and the event input in post-order
  traversal. We start the DFS from the terminal tasks (those that don't have
  dependencies) and call the `evaluator` function with a representation of the
  task in the graph, an `event` as input, and an accumulator argument.

  The `evaluator` must be a callable which accepts three arguments:

    - task: an InMemoryTask instance, representing a task in the graph.
    - event: an object whose shape/type is defined by the caller of the
      `Evaluate` function and that the evaluator can handle.
    - accumulator: a dictionary which is mutable which is valid in the scope of
      a traversal of the graph.

  The `evaluator` must return either None or an iterable of callables which take
  a single argument, which is the accumulator at the end of a traversal.

  Events are free-form but usually are dictionaries which constitute inputs that
  are external to the task graph evaluation. This could model events in an
  event-driven evaluation of tasks, or synthetic inputs to the system. It is
  more important that the `event` information is known to the evaluator
  implementation, and is provided as-is to the evaluator in this function.

  The Evaluate function will keep iterating while there are actions still being
  produced by the evaluator. When there are no more actions to run, the Evaluate
  function will return the most recent traversal's accumulator.
  """
  if job is None:
    raise ValueError('job must not be None.')

  accumulator = {}
  actions = [NoopAction()]
  while actions:
    for action in actions:
      logging.debug('Running action: %s', action)
      # Each action should be a callable which takes the accumulator as an
      # input. We want to run each action in their own transaction as well.
      # This must not be called in a transaction.
      with timing.WallTimeLogger('ExecutionEngine:ActionRunner<%s>' %
                                 (type(action).__name__,)):
        action(accumulator)

    # Clear the actions and accumulator for this traversal.
    del actions[:]
    accumulator.clear()

    # Load the graph transactionally.
    graph = _LoadTaskGraph(job)

    if not graph.tasks:
      logging.debug('Task graph empty for job %s', job.job_id)
      return None

    # First get all the "terminal" tasks, and traverse the dependencies in a
    # depth-first-search.
    task_stack = [graph.tasks[task] for task in graph.terminal_tasks]

    # If the stack is empty, we should start at an arbitrary point.
    if not task_stack:
      task_stack = [list(graph.tasks.values())[0]]
    vertex_states = {}
    while task_stack:
      task = task_stack[-1]
      state = vertex_states.get(task.key, NOT_EVALUATED)
      if state == CHILDREN_PENDING:
        in_memory_task = task.ToInMemoryTask()
        result_actions = evaluator(in_memory_task, event, accumulator)
        if result_actions:
          actions.extend(result_actions)
        vertex_states[task.key] = EVALUATION_DONE
      elif state == NOT_EVALUATED:
        # This vertex is coloured white, we should traverse the dependencies.
        vertex_states[task.key] = CHILDREN_PENDING
        for dependency in task.dependencies:
          if vertex_states.get(dependency, NOT_EVALUATED) == NOT_EVALUATED:
            task_stack.append(graph.tasks[dependency])
      else:
        assert state == EVALUATION_DONE
        task_stack.pop()

  return accumulator
