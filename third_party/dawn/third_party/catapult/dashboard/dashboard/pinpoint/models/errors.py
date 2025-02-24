# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Various exceptions used in Pinpoint.

These are typically user-facing, i.e. when these failures happen in a job they
are reported to the user in the UI in some form.  In general they represent some
reason why a job/task/step/etc was unable to find a result.

Here's the exception hierarchy:

  JobError
   +-- FatalError
   |    +-- AllRunsFailed
   |    +-- BuildCancelledFatal
   |    +-- BuildFailedFatal
   |    +-- BuildIsolateNotFound
   |    +-- ExecutionEngineErrors
   |    +-- SwarmingExpired
   +-- InformationalError
   |    +-- BuildFailed
   |    +-- BuildCancelled
   |    +-- BuildNumberExceeded
   |    +-- BuildGerritUrlNotFound
   |    +-- BuildGerritURLInvalid
   |    +-- CancelError
   |    +-- SwarmingTaskError
   |    +-- SwarmingTaskFailed
   |    +-- SwarmingNoBots
   |    +-- ReadValidNoValues
   |    +-- ReadValueNotFound
   |    +-- ReadValueUnknownStat
   |    +-- ReadValueChartNotFound
   |    +-- ReadValueTraceNotFound
   |    +-- ReadValueNoFile
   |    +-- ReadValueUnknownFormat
   +-- JobRetryError
        +-- JobRetryLimitExceededError
        +-- JobRetryFailed
  RecoverableError

"""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import traceback
import pprint


class JobError(Exception):
  """Base exception for errors in this module."""

  # Classification of the error for analytics purposes, and potentially a rough
  # indication of where the fault lies.  One of 'build', 'request', 'pinpoint',
  # 'test', or None (indicating unknown).
  category = None


class FatalError(JobError):
  pass


class InformationalError(JobError):
  pass


# Not a JobError because this is only used for internal control flow -- this
# should never be a user-visible exception.
class RecoverableError(Exception):
  """An error that is usually transient, so the operation should be retried."""

  def __init__(self, wrapped_exc):
    super().__init__()
    self.wrapped_exc = wrapped_exc

  def __str__(self):
    return ('Retriable operation failed with: ' +
            _FormatException(self.wrapped_exc))


class BuildIsolateNotFound(FatalError):
  category = 'build'

  def __init__(self):
    super().__init__(
        'The build was reported to have completed successfully, but Pinpoint '\
        'is unable to find the isolate that was produced and will be unable '\
        'to run any tests against this revision.')


class BuildFailed(InformationalError):
  category = 'build'

  def __init__(self, reason):
    super().__init__(
        'Encountered an %s error while attempting to build this revision. '\
        'Pinpoint will be unable to run any tests against this '\
        'revision.' % reason)


class BuildFailedFatal(BuildFailed, FatalError):
  pass


class BuildCancelled(InformationalError):

  def __init__(self, reason):
    super().__init__('The build was cancelled with reason: %s. "\
        "Pinpoint will be unable to run any tests against this "\
        "revision.' % reason)


class BuildCancelledFatal(BuildCancelled, FatalError):
  pass


class BuildNumberExceeded(InformationalError):

  def __init__(self, reason):
    super().__init__('Bisected max number of times: %d. To bisect further, '\
                     'consult https://chromium.googlesource.com/catapult/+/'\
                     'HEAD/dashboard/dashboard/pinpoint/docs/abort_error.md'
                     % reason)


class BuildGerritUrlNotFound(InformationalError):

  def __init__(self, reason):
    super().__init__(
        'Unable to find gerrit url for commit %s. Pinpoint will be unable '\
        'to run any tests against this revision.' % reason)


class BuildGerritURLInvalid(InformationalError):
  category = 'request'

  def __init__(self, reason):
    super().__init__(
        'Invalid url: %s. Pinpoint currently only supports the fully '\
        'redirected patch URL, ie. https://chromium-review.googlesource.com/'\
        'c/chromium/src/+/12345' % reason)


class CancelError(InformationalError):

  def __init__(self, reason):
    super().__init__('Cancellation request failed: {}'.format(reason))


class SwarmingExpired(FatalError):
  category = 'pinpoint'

  def __init__(self):
    super().__init__(
        'The test was successfully queued in swarming, but expired. This is '\
        'likely due to the bots being overloaded, dead, or misconfigured. '\
        'Pinpoint will stop this job to avoid potentially overloading the '\
        'bots further.')


class SwarmingTaskError(InformationalError):
  category = 'test'

  def __init__(self, reason):
    super().__init__(
        'The swarming task failed with state "%s". This generally indicates '\
        'that the test was successfully started, but was stopped prematurely. '\
        'This error could be something like the bot died, the test timed out, '\
        'or the task was manually canceled.' % reason)


class SwarmingTaskFailed(InformationalError):
  """Raised when the test fails."""

  category = 'test'

  def __init__(self, task_output):
    super().__init__(
        'The test ran but failed. This is likely to a problem with the test '
        'itself either being broken or flaky in the range specified.\n\n'
        'Please click through to the task isolate output at:'
        ' %s' % (task_output,))
    self.task_output = task_output


class SwarmingNoBots(InformationalError):
  category = 'request'

  def __init__(self):
    super().__init__(
        "There doesn't appear to be any bots available to run the "\
        'performance test. Either all the swarming devices are offline, or '\
        "they're misconfigured.")


class ReadValueNoValues(InformationalError):
  category = 'test'

  def __init__(self):
    super().__init__(
        'The test ran successfully, but the output failed to contain any '\
        'valid values. This is likely due to a problem with the test itself '\
        'in this range.')


class ReadValueNotFound(InformationalError):
  category = 'request'

  def __init__(self, reason):
    super().__init__(
        "The test ran successfully, but the metric specified (%s) wasn't "\
        'found in the output. Either the metric specified was invalid, or '\
        "there's a problem with the test itself in this range." % reason)


class ReadValueUnknownStat(InformationalError):
  category = 'pinpoint'

  def __init__(self, reason):
    super().__init__(
        "The test ran successfully, but the statistic specified (%s) wasn't "\
        'found in the output. Either the metric specified was invalid, '\
        "or there's a problem with the test itself in this range." % reason)


class ReadValueChartNotFound(InformationalError):
  category = 'request'

  def __init__(self, reason):
    super().__init__(
        "The test ran successfully, but the chart specified (%s) wasn't "\
        'found in the output. Either the chart specified was invalid, or '\
        "there's a problem with the test itself in this range." % reason)


class ReadValueTraceNotFound(InformationalError):
  category = 'request'

  def __init__(self, reason):
    super().__init__(
        "The test ran successfully, but the trace specified (%s) wasn't "\
        'found in the output. Either the trace specified was invalid, or '\
        "there's a problem with the test itself in this range." % reason)


class ReadValueNoFile(InformationalError):
  category = 'test'

  def __init__(self, reason):
    super().__init__(
        'The test ran successfully but failed to produce an expected '\
        'output file: %s. This is likely due to a problem with the test '\
        'itself in this range.' % reason)


class ReadValueUnknownFormat(InformationalError):
  category = 'request'

  def __init__(self, reason):
    super().__init__(
        'The test ran successfully but produced a format that Pinpoint could '
        'not parse properly. Please see the file "%s" and potentially file an '
        'issue in Speed>Bisection for further debugging.' % reason)


class AllRunsFailed(FatalError):

  def __init__(self, exc_count, att_count, exc):
    super().__init__(
        'All of the runs failed. The most common error (%d/%d runs) '\
        'was:\n%s' % (exc_count, att_count, exc))
    exc_category = getattr(exc, 'category', None)
    if exc_category is not None:
      self.category = exc_category


class JobRetryError(JobError):

  def __init__(self, message, category=None, wrapped_exc=None):
    super().__init__()
    self.message = message
    self.category = category
    self.wrapped_exc = wrapped_exc

  def __str__(self):
    return 'Job retry failed with: ' + self.message


class JobRetryLimitExceededError(JobRetryError):

  def __init__(self, wrapped_exc=None):
    message = ('Pinpoint has hit its retry limit and will terminate this job.\n'
               'Most recent failure:\n' + _FormatException(wrapped_exc))
    # wrapped_exc is always a RecoverableError (never JobError), so we don't
    # have a category.
    category = None
    JobRetryError.__init__(self, message, category, wrapped_exc)


class JobRetryFailed(JobRetryError):

  def __init__(self, wrapped_exc=None):
    message = ("Pinpoint wasn't able to reschedule itself to run again.\n"
               'Most recent failure:\n' + _FormatException(wrapped_exc))
    # wrapped_exc is always a RecoverableError (never JobError), so we don't
    # have a category.
    category = None
    JobRetryError.__init__(self, message, category, wrapped_exc)


# TODO(dberris): Create more granular error mappings for the execution engine
# here.
class ExecutionEngineErrors(FatalError):
  category = 'pinpoint'

  def __init__(self, errors):
    super().__init__(
        'Encountered fatal errors executing under the execution engine.\n'
        'All errors:\n %s' % (pprint.pformat(errors),),)


def _FormatException(exc):
  """Format an Exception the way it would be in a traceback.

  >>> err = ValueError('bad data')
  >>> _FormatException(err)
  'ValueError: bad data\n'
  """
  return ''.join(traceback.format_exception_only(type(exc), exc))


REFRESH_FAILURE = 'An unknown failure occurred during the run.\n'\
                  'Please file a bug under Speed>Bisection with this job.'

TRANSIENT_ERROR_MSG = 'Pinpoint encountered an error while connecting to an '\
                      'external service. The service is either down, '\
                      'unresponsive, or the problem was transient. These are '\
                      'typically retried by Pinpoint, so if you see this, '\
                      'please file a bug.'

FATAL_ERROR_MSG = 'Pinpoint encountered a fatal internal error and cannot '\
                  'continue. Please file an issue with Speed>Bisection.'
