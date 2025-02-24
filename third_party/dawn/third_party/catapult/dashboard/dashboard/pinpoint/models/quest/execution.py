# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import traceback

from dashboard.pinpoint.models import errors
import logging
import six

from google.auth import exceptions
TokenRefreshError = exceptions.RefreshError

from google.appengine.api import datastore_errors

DataStoreTimeoutError = datastore_errors.Timeout


class Execution:
  """Object tracking the execution of a Quest.

  An Execution object is created for each Quest when it starts running.
  Therefore, each Attempt consists of a list of Executions. The Attempt is
  finished when an Execution fails or when the number of completed Executions is
  equal to the number of Quests. If an Execution fails, the Attempt will have
  fewer Executions in the end.

  Because the Execution is created when the Quest starts running, the lifecycle
  of the Execution object is tied to the work being done. There isn't a state
  where the Execution hasn't started running; it's either running or completed.
  """

  def __init__(self):
    self._completed = False
    self._exception = None
    self._result_values = ()
    self._result_arguments = {}

  @property
  def completed(self):
    """Returns True iff the Execution is completed. Otherwise, it's running.

    This accessor doesn't contact external servers. Call Poll() to update the
    Execution's completed status.
    """
    return self._completed

  @property
  def failed(self):
    """Returns True iff the Execution is completed and has failed.

    This accessor doesn't contact external servers. Call Poll() to update the
    Execution's failed status.
    """
    return bool(self._exception)

  @property
  def exception(self):
    """Returns the stack trace if the Execution failed, or None otherwise."""
    return self._exception

  @property
  def result_values(self):
    """Data used to determine if two Execution results differ.

    Currently it's just a list of integers or floats. In the future, it will be
    a Catapult Value. For a Build or Test Execution, this is a list containing 0
    or 1 representing success or failure. For a ReadValue Execution, this is a
    list of numbers with the values.
    """
    return self._result_values

  @property
  def result_arguments(self):
    """A dict of information passed on to the next Execution.

    For example, the Build Execution passes the isolate hash to the Test
    Execution.
    """
    return self._result_arguments

  # TODO(simonhatch): After migrating all Pinpoint entities, this can be
  # removed.
  # crbug.com/971370
  def __setstate__(self, state):
    self.__dict__ = state  # pylint: disable=attribute-defined-outside-init
    if isinstance(self._exception, six.string_types):
      self._exception = {
          'message': self._exception.splitlines()[-1],
          'traceback': self._exception
      }

  def AsDict(self):
    d = {
        'completed': self._completed,
        'exception': self._exception,
        'details': self._AsDict(),
    }

    if isinstance(self._exception, six.string_types):
      d['exception'] = {
          'message': self._exception.splitlines()[-1],
          'traceback': self._exception
      }

    return d

  def _AsDict(self):
    raise NotImplementedError()

  def Poll(self):
    """Update the Execution status, with error handling.

    Pinpoint has two kinds of errors: Job-level errors, which are fatal and
    cause the Job to fail; and Execution-level errors, which only cause that
    one execution to fail. If something might pass at one commit and fail at
    another, it should be an Execution-level error. The type of Exception
    indicates the error level. StandardErrors are Job-level and all other
    errors are Execution-level.
    """
    assert not self.completed

    try:
      self._Poll()
    except (TokenRefreshError, DataStoreTimeoutError) as e:
      logging.warning('Execution failed with exception: %s', str(e))
      raise errors.RecoverableError(e)
    except (errors.FatalError, RuntimeError) as e:
      # Some built-in exceptions are derived from RuntimeError which we'd like
      # to treat as errors.
      logging.warning('Execution failed with exception: %s', str(e))
      raise
    except Exception as e:  # pylint: disable=broad-except
      # We allow broad exception handling here, because we log the exception and
      # display it in the UI.
      self._completed = True
      tb = traceback.format_exc()
      if hasattr(e, 'task_output'):
        tb += '\n%s' % getattr(e, 'task_output')
      self._exception = {'message': str(e), 'traceback': tb}
      logging.warning('Execution failed with exception: %s', str(e))

  def _Poll(self):
    raise NotImplementedError()

  def _Complete(self, result_values=(), result_arguments=None):
    self._completed = True
    self._result_values = result_values
    self._result_arguments = result_arguments or {}
