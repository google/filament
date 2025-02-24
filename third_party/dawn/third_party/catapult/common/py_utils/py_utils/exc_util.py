# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import functools
import logging
import sys


def BestEffort(func):
  """Decorator to log and dismiss exceptions if one if already being handled.

  Note: This is largely a workaround for the lack of support of exception
  chaining in Python 2.7, this decorator will no longer be needed in Python 3.

  Typical usage would be in |Close| or |Disconnect| methods, to dismiss but log
  any further exceptions raised if the current execution context is already
  handling an exception. For example:

      class Client(object):
        def Connect(self):
          # code to connect ...

        @exc_util.BestEffort
        def Disconnect(self):
          # code to disconnect ...

      client = Client()
      try:
        client.Connect()
      except:
        client.Disconnect()
        raise

  If an exception is raised by client.Connect(), and then a second exception
  is raised by client.Disconnect(), the decorator will log the second exception
  and let the original one be re-raised.

  Otherwise, in Python 2.7 and without the decorator, the second exception is
  the one propagated to the caller; while information about the original one,
  usually more important, is completely lost.

  Note that if client.Disconnect() is called in a context where an exception
  is *not* being handled, then any exceptions raised within the method will
  get through and be passed on to callers for them to handle in the usual way.

  The decorator can also be used on cleanup functions meant to be called on
  a finally block, however you must also include an except-raise clause to
  properly signal (in Python 2.7) whether an exception is being handled; e.g.:

      @exc_util.BestEffort
      def cleanup():
        # do cleanup things ...

      try:
        process(thing)
      except:
        raise  # Needed to let cleanup know if an exception is being handled.
      finally:
        cleanup()

  Failing to include the except-raise block has the same effect as not
  including the decorator at all. Namely: exceptions during |cleanup| are
  raised and swallow any prior exceptions that occurred during |process|.
  """
  @functools.wraps(func)
  def Wrapper(*args, **kwargs):
    exc_type = sys.exc_info()[0]
    if exc_type is None:
      # Not currently handling an exception; let any errors raise exceptions
      # as usual.
      func(*args, **kwargs)
    else:
      # Otherwise, we are currently handling an exception, dismiss and log
      # any further cascading errors. Callers are responsible to handle the
      # original exception.
      try:
        func(*args, **kwargs)
      except Exception:  # pylint: disable=broad-except
        logging.exception(
            'While handling a %s, the following exception was also raised:',
            exc_type.__name__)

  return Wrapper
