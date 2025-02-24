# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""More dummy exception subclasses used by core/discover.py's unit tests."""

# Import class instead of module explicitly so that inspect.getmembers() returns
# two Exception subclasses in this current file.
# Suppress complaints about unable to import class.  The directory path is
# added at runtime by telemetry test runner.
#pylint: disable=import-error
from __future__ import absolute_import
from discoverable_classes import discover_dummyclass


class _PrivateDummyException(discover_dummyclass.DummyException):
  def __init__(self):
    discover_dummyclass.DummyException.__init__(self)


class DummyExceptionImpl1(_PrivateDummyException):
  def __init__(self):
    _PrivateDummyException.__init__(self)


class DummyExceptionImpl2(_PrivateDummyException):
  def __init__(self):
    _PrivateDummyException.__init__(self)


class DummyExceptionWithParameterImpl1(_PrivateDummyException):
  def __init__(self, parameter):
    super().__init__()
    del parameter
