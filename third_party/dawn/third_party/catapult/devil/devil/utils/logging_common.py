# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import logging
import sys
import time


def AddLoggingArguments(parser):
  """Adds standard logging flags to the parser.

  After parsing args, remember to invoke InitializeLogging() with the parsed
  args, to configure the log level.
  """
  group = parser.add_mutually_exclusive_group()
  group.add_argument(
      '-v',
      '--verbose',
      action='count',
      default=0,
      help='Log more. Use multiple times for even more logging.')
  group.add_argument(
      '-q',
      '--quiet',
      action='count',
      default=0,
      help=('Log less (suppress output). Use multiple times for even less '
            'output.'))


def InitializeLogging(args, handler=None):
  """Initialized the log level based on commandline flags.

  This expects to be given an "args" object with the options defined by
  AddLoggingArguments().
  """
  if args.quiet >= 2:
    log_level = logging.CRITICAL
  elif args.quiet == 1:
    log_level = logging.ERROR
  elif args.verbose == 0:
    log_level = logging.WARNING
  elif args.verbose == 1:
    log_level = logging.INFO
  else:
    log_level = logging.DEBUG
  logger = logging.getLogger()
  logger.setLevel(log_level)
  if not handler:
    handler = logging.StreamHandler(sys.stdout)
    handler.setFormatter(CustomFormatter())
  logger.addHandler(handler)


class CustomFormatter(logging.Formatter):
  """Custom log formatter."""

  # override
  def __init__(self, fmt='%(threadName)-4s  %(message)s'):
    # Can't use super() because in older Python versions logging.Formatter does
    # not inherit from object.
    logging.Formatter.__init__(self, fmt=fmt)
    self._creation_time = time.time()

  # override
  def format(self, record):
    # Can't use super() because in older Python versions logging.Formatter does
    # not inherit from object.
    msg = logging.Formatter.format(self, record)
    if 'MainThread' in msg[:19]:
      msg = msg.replace('MainThread', 'Main', 1)
    timelocal = time.strftime("%H:%M:%S", time.localtime(record.created))
    timelocal = "%s.%03d" % (timelocal, record.msecs)
    timediff = record.created - self._creation_time
    return '%s %s %8.3fs %s' % (record.levelname[0], timelocal, timediff, msg)
