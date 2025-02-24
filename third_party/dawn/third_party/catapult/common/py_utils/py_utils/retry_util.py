# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
import functools
import logging
import time
from six.moves import range # pylint: disable=redefined-builtin


def RetryOnException(exc_type, retries):
  """Decorator to retry running a function if an exception is raised.

  Implements exponential backoff to wait between each retry attempt, starting
  with 1 second.

  Note: the default number of retries is defined on the decorator, the decorated
  function *must* also receive a "retries" argument (although its assigned
  default value is ignored), and clients of the funtion may override the actual
  number of retries at the call site.

  The "unused" retries argument on the decorated function must be given to
  keep pylint happy and to avoid breaking the Principle of Least Astonishment
  if the decorator were to change the signature of the function.

  For example:

    @retry_util.RetryOnException(OSError, retries=3)  # default no. of retries
    def ProcessSomething(thing, retries=None):  # this default value is ignored
      del retries  # Unused. Handled by the decorator.
      # Do your thing processing here, maybe sometimes raising exeptions.

    ProcessSomething(a_thing)  # retries 3 times.
    ProcessSomething(b_thing, retries=5)  # retries 5 times.

  Args:
    exc_type: An exception type (or a tuple of them), on which to retry.
    retries: Default number of extra attempts to try, the caller may also
      override this number. If an exception is raised during the last try,
      then the exception is not caught and passed back to the caller.
  """
  def Decorator(f):
    @functools.wraps(f)
    def Wrapper(*args, **kwargs):
      wait = 1
      kwargs.setdefault('retries', retries)
      for _ in range(kwargs['retries']):
        try:
          return f(*args, **kwargs)
        except exc_type as exc:
          logging.warning(
              '%s raised %s, will retry in %d second%s ...',
              f.__name__, type(exc).__name__, wait, '' if wait == 1 else 's')
          time.sleep(wait)
          wait *= 2
      # Last try with no exception catching.
      return f(*args, **kwargs)
    return Wrapper
  return Decorator
