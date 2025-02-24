# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import functools
import logging

from google.appengine.api import urlfetch_errors

from six.moves import http_client
from six.moves import map  # pylint: disable=redefined-builtin

from dashboard.common import math_utils
from dashboard.models import anomaly
from dashboard.pinpoint.models import attempt as attempt_module
from dashboard.pinpoint.models import change as change_module
from dashboard.pinpoint.models import compare
from dashboard.pinpoint.models import errors
from dashboard.pinpoint.models import exploration

# We start with 10 attempts at a given change and double until we reach 160
# attempts max (that's 4 iterations).
MIN_ATTEMPTS = 10
MAX_ATTEMPTS = 160
MAX_BUILDS = 60
_DEFAULT_SPECULATION_LEVELS = 1

FUNCTIONAL = 'functional'
PERFORMANCE = 'performance'
TRY = 'try'
COMPARISON_MODES = (FUNCTIONAL, PERFORMANCE, TRY)


class JobState:
  """The internal state of a Job.

  Wrapping the entire internal state of a Job in a PickleProperty allows us to
  use regular Python objects, with constructors, dicts, and object references.

  We lose the ability to index and query the fields, but it's all internal
  anyway. Everything queryable should be on the Job object."""

  def __init__(self,
               quests,
               comparison_mode=None,
               comparison_magnitude=None,
               pin=None,
               initial_attempt_count=None):
    """Create a JobState.

    Args:
      comparison_mode: Either 'functional' or 'performance', which the Job uses
          to figure out whether to perform a functional or performance bisect.
          If None, the Job will not automatically add any Attempts or Changes.
      comparison_magnitude: The estimated size of the regression or improvement
          to look for. Smaller magnitudes require more repeats.
      quests: A sequence of quests to run on each Change.
      pin: A Change (Commits + Patch) to apply to every Change in this Job.
      initial_attempt_count: The number of attempts (iterations) to try first.
    """
    # _quests is mutable. Any modification should mutate the existing list
    # in-place rather than assign a new list, because every Attempt references
    # this object and will be updated automatically if it's mutated.
    self._quests = list(quests)

    self._comparison_mode = comparison_mode
    self._comparison_magnitude = comparison_magnitude
    # only bisections and verification jobs care about the improvement direction
    # assume unknown until specified
    self._improvement_direction = anomaly.UNKNOWN

    self._pin = pin

    # _changes can be in arbitrary order. Client should not assume that the
    # list of Changes is sorted in any particular order.
    self._changes = []

    # A mapping from a Change to a list of Attempts on that Change.
    self._attempts = {}
    self._initial_attempt_count = initial_attempt_count if initial_attempt_count else MIN_ATTEMPTS

  def PropagateJob(self, job):
    """Propagate a Job to every Quest.

    Args:
      job: A fully formed dashboard.pinpoint.models.job.Job instance.
    """
    for quest in self._quests:
      quest.PropagateJob(job)

  @property
  def metric(self):
    if self._comparison_mode == 'functional':
      return 'Failure rate'
    return self._quests[-1].metric if self._quests else ''

  @property
  def attempt_count(self):
    return self._initial_attempt_count

  def AddAttempts(self, change):
    if not hasattr(self, '_pin'):
      # TODO: Remove after data migration.
      self._pin = None

    if self._pin:
      change_with_pin = change.Update(self._pin)
    else:
      change_with_pin = change

    # This algorithm will double the number of attempts, to allow us to get
    # more attempts sooner and getting to better statistical decisions with
    # less iterations.
    initial_attempt_count = self._initial_attempt_count if hasattr(
        self, '_initial_attempt_count') else MIN_ATTEMPTS
    current_attempt_count = max(
        len(self._attempts[change]), initial_attempt_count)
    it = 0
    while it != current_attempt_count:
      attempt = attempt_module.Attempt(self._quests, change_with_pin)
      self._attempts[change].append(attempt)
      it += 1

  def AddChange(self, change, index=None):
    if index:
      self._changes.insert(index, change)
    else:
      self._changes.append(change)

    self._attempts[change] = []
    self.AddAttempts(change)

    if len(self._changes) > MAX_BUILDS:
      raise errors.BuildNumberExceeded(MAX_BUILDS)

  def SetImprovementDirection(self, improvement_direction):
    self._improvement_direction = improvement_direction

  def Explore(self, benchmark_arguments=None, job_id=None):
    """Compare Changes and bisect by adding additional Changes as needed.

    For every pair of adjacent Changes, compare their results as probability
    distributions. If the results are different, find surrounding Changes and
    add it to the Job. If the results are the same, do nothing.  If the results
    are inconclusive, add more Attempts to the Change with fewer Attempts until
    we decide they are the same or different.

    Intermediate points can only be added if the end Change represents a
    commit that comes after the start Change. Otherwise, this method won't
    explore further. For example, if Change A is repo@abc, and Change B is
    repo@abc + patch, there's no way to pick additional Changes to try.
    """
    if not self._changes:
      return

    def DetectChange(change_a, change_b, benchmark_arguments=None, job_id=None):
      comparison = self._Compare(change_a, change_b, benchmark_arguments,
                                 job_id)
      # We return None if the comparison determines that the result is
      # inconclusive.
      if comparison == compare.UNKNOWN:
        return None
      return comparison == compare.DIFFERENT

    changes_to_refine = set()

    def CollectChangesToRefine(change_a, change_b):
      # We will return the changes which has less than or equal number of
      # attempts but also doesn't have the maximum number of attempts.
      change_a_attempts = len(self._attempts[change_a])
      change_b_attempts = len(self._attempts[change_b])

      if (change_a_attempts <= change_b_attempts
          and change_a_attempts < MAX_ATTEMPTS):
        changes_to_refine.add(change_a)
      if (change_b_attempts <= change_a_attempts
          and change_b_attempts < MAX_ATTEMPTS):
        changes_to_refine.add(change_b)

    def FindMidpoint(change_a, change_b):
      try:
        return change_module.Change.Midpoint(change_a, change_b)
      except change_module.NonLinearError:
        return None
      except change_module.DepsParsingError as e:
        raise errors.RecoverableError(e)

    try:
      additional_changes = exploration.Speculate(
          self._changes,
          change_detected=DetectChange,
          on_unknown=CollectChangesToRefine,
          midpoint=FindMidpoint,
          levels=_DEFAULT_SPECULATION_LEVELS,
          benchmark_arguments=benchmark_arguments,
          job_id=job_id)
      logging.debug('Refinement list: %s', changes_to_refine)
      for change in changes_to_refine:
        self.AddAttempts(change)
      logging.debug('Edit list: %s', additional_changes)
      for index, change in additional_changes:
        self.AddChange(change, index)
    except (http_client.HTTPException,
            urlfetch_errors.DeadlineExceededError) as e:
      logging.debug('Encountered error: %s', e)
      raise errors.RecoverableError(e)

  def ScheduleWork(self):
    work_left = False
    for attempts in self._attempts.values():
      for attempt in attempts:
        if attempt.completed:
          logging.debug('JobQueueDebug: attempt completed. %s', attempt)
          continue

        logging.debug('JobQueueDebug: attempt scheduling new work. %s', attempt)
        attempt.ScheduleWork()
        work_left = True

    if not work_left:
      self._RaiseErrorIfAllAttemptsFailed()

    return work_left

  def _RaiseErrorIfAllAttemptsFailed(self):
    counter = collections.Counter()
    for attempts in self._attempts.values():
      for attempt in attempts:
        if not attempt.exception:
          return
        counter[attempt.exception['traceback'].splitlines()[-1]] += 1

    most_common_exceptions = counter.most_common(1)
    if not most_common_exceptions:
      return

    exception, exception_count = most_common_exceptions[0]
    attempt_count = sum(counter.values())
    raise errors.AllRunsFailed(exception_count, attempt_count, exception)

  def Differences(self):
    """Compares every pair of Changes and yields ones with different results.

    This method loops through every pair of adjacent Changes. If they have
    statistically different results, this method yields that pair.  (The second
    element of each returned pair is assumed to have caused the difference).

    Returns:
      A list of tuples: [(Change_before, Change_after), ...]
    """
    differences = []

    def Comparison(a, b):
      if not a:
        return b
      if self._Compare(a, b) == compare.DIFFERENT:
        differences.append((a, b))
      return b

    functools.reduce(Comparison, self._changes, None)
    return differences

  def AsDict(self, options=None):

    def Transform(change):
      result = {
          'change': change.AsDict(),
      }
      if 'INPUTS' not in options:
        result.update({
            'attempts': [
                attempt.AsDict() for attempt in self._attempts[change]
            ],
            'comparisons': {},
            'result_values': self.ResultValues(change),
        })
      return result, change

    def CollectStates(states, change_b):
      if len(states) == 0:
        states.append(Transform(change_b))
        return states

      transformed_a, change_a = states.pop()
      transformed_b, change_b = Transform(change_b)
      if 'INPUTS' not in options:
        comparison = self._Compare(change_a, change_b)
        transformed_a['comparisons']['next'] = comparison
        transformed_b['comparisons']['prev'] = comparison

      states.extend([(transformed_a, change_a), (transformed_b, change_b)])
      return states

    return {
        'comparison_mode':
            self._comparison_mode,
        'metric':
            self.metric,
        'quests':
            list(map(str, self._quests)),
        'state': [
            transformed for transformed, _ in functools.reduce(
                CollectStates, self._changes, [])
        ]
    }

  def _Compare(self, change_a, change_b, benchmark_arguments=None, job_id=None):
    """Compare the results of two Changes in this Job.

    Aggregate the exceptions and result_values across every Quest for both
    Changes. Then, compare all the results for each Quest. If any of them are
    different, return DIFFERENT. Otherwise, if any of them are inconclusive,
    return UNKNOWN.  Otherwise, they are the SAME.

    Arguments:
      change_a: The first Change whose results to compare.
      change_b: The second Change whose results to compare.

    Returns:
      PENDING: If either Change has an incomplete Attempt.
      DIFFERENT: If the two Changes (very likely) have different results.
      SAME: If the two Changes (probably) have the same result.
      UNKNOWN: If we'd like more data to make a decision.
    """
    attempts_a = self._attempts[change_a]
    attempts_b = self._attempts[change_b]

    if any(not attempt.completed for attempt in attempts_a + attempts_b):
      return compare.PENDING

    attempt_count = (len(attempts_a) + len(attempts_b)) // 2

    executions_by_quest_a = _ExecutionsPerQuest(attempts_a)
    executions_by_quest_b = _ExecutionsPerQuest(attempts_b)

    any_unknowns = False
    for quest in self._quests:
      executions_a = executions_by_quest_a[str(quest)]
      executions_b = executions_by_quest_b[str(quest)]

      # Compare exceptions.
      exceptions_a = tuple(
          bool(execution.exception) for execution in executions_a)
      exceptions_b = tuple(
          bool(execution.exception) for execution in executions_b)
      if exceptions_a and exceptions_b:
        if self._comparison_mode == FUNCTIONAL:
          if getattr(self, '_comparison_magnitude', None):
            comparison_magnitude = self._comparison_magnitude
          else:
            comparison_magnitude = 0.5
        else:
          comparison_magnitude = 1.0

        logging.debug('BisectDebug: Functional Comparing exceptions: %s, %s',
            exceptions_a, exceptions_b)
        comparison, _, _, _ = compare.Compare(exceptions_a, exceptions_b,
                                              attempt_count, FUNCTIONAL,
                                              comparison_magnitude,
                                              benchmark_arguments, job_id)
        logging.debug('BisectDebug: Functional Compare result: %s', comparison)
        if comparison == compare.DIFFERENT:
          return compare.DIFFERENT
        if comparison == compare.UNKNOWN:
          any_unknowns = True

      # Compare result values by consolidating all measurments by change, and
      # treating those as a single sample set for comparison.
      def AllValues(execution):
        for e in execution:
          if not e.result_values:
            continue
          for v in e.result_values:
            yield v

      all_a_values = tuple(AllValues(executions_a))
      all_b_values = tuple(AllValues(executions_b))
      if all_a_values and all_b_values:
        if getattr(self, '_comparison_magnitude', None):
          max_iqr = max(
              max(math_utils.Iqr(all_a_values), math_utils.Iqr(all_b_values)),
              0.001)
          comparison_magnitude = abs(self._comparison_magnitude / max_iqr)
        else:
          comparison_magnitude = 1.0

        sample_count = (len(all_a_values) + len(all_b_values)) // 2
        logging.debug('BisectDebug: Comparing values: %s, %s',
            all_a_values, all_b_values)
        mean_diff = Mean(all_b_values) - Mean(all_a_values)
        # Pinpoint jobs that exist prior to this change will not
        # have an improvement direction. See crbug/1351167#c4
        if not getattr(self, '_improvement_direction', None):
          self._improvement_direction = anomaly.UNKNOWN
        # if improvement, return same
        if ((mean_diff > 0 and self._improvement_direction == anomaly.UP)
            or mean_diff < 0 and self._improvement_direction == anomaly.DOWN):
          logging.debug(
              'BisectDebug: Improvement found. Improvement direction: %s, mean diff: %s',
              self._improvement_direction, mean_diff)
          return compare.SAME
        comparison, _, _, _ = compare.Compare(all_a_values, all_b_values,
                                              sample_count, PERFORMANCE,
                                              comparison_magnitude,
                                              benchmark_arguments, job_id)
        logging.debug('BisectDebug: Compare result: %s', comparison)
        if comparison == compare.DIFFERENT:
          return compare.DIFFERENT
        if comparison == compare.UNKNOWN:
          any_unknowns = True

    if any_unknowns:
      return compare.UNKNOWN

    return compare.SAME

  def ResultValues(self, change):
    quest_index = len(self._quests) - 1
    result_values = []

    if self._comparison_mode == 'functional':
      pass_fails = []
      for attempt in self._attempts[change]:
        if attempt.completed:
          pass_fails.append(int(attempt.failed))
      if pass_fails:
        result_values.append(Mean(pass_fails))

    elif self._comparison_mode == 'performance':
      for attempt in self._attempts[change]:
        if quest_index < len(attempt.executions):
          result_values += attempt.executions[quest_index].result_values

    return result_values

  def ChangesExamined(self):
    return len(self._changes)

  def TotalAttemptsExecuted(self):
    total_attempts = 0
    for attempts in self._attempts.values():
      total_attempts += len(attempts)
    return total_attempts

  def FirstOrLastChangeFailed(self):
    """Did all attempts complete and fail for the first or last change?"""
    if not self._changes:
      # No changes, so technically they didn't fail.
      return False

    attempts_a = self._attempts.get(self._changes[0], [])
    attempts_b = self._attempts.get(self._changes[-1], [])

    if attempts_a and all(a.failed for a in attempts_a):
      return True
    if attempts_b and all(a.failed for a in attempts_b):
      return True

    return False


def _ExecutionsPerQuest(attempts):
  executions = collections.defaultdict(list)
  for attempt in attempts:
    for quest, execution in zip(attempt.quests, attempt.executions):
      executions[str(quest)].append(execution)
  return executions


def Mean(values):
  values = [v for v in values if isinstance(v, (int, float))]
  if len(values) == 0:
    return float('nan')
  return float(sum(values)) / len(values)
