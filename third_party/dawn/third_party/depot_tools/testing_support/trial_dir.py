# Copyright (c) 2011 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import atexit
import logging
import os
import sys
import tempfile
import unittest

import gclient_utils


class TrialDir(object):
    """Manages a temporary directory.

    On first object creation, TrialDir.TRIAL_ROOT will be set to a new temporary
    directory created in /tmp or the equivalent. It will be deleted on process
    exit unless TrialDir.SHOULD_LEAK is set to True.
    """
    # When SHOULD_LEAK is set to True, temporary directories created while the
    # tests are running aren't deleted at the end of the tests. Expect failures
    # when running more than one test due to inter-test side-effects. Helps with
    # debugging.
    SHOULD_LEAK = False

    # Main root directory.
    TRIAL_ROOT = None

    def __init__(self, subdir, leak=False):
        self.leak = self.SHOULD_LEAK or leak
        self.subdir = subdir
        self.root_dir = None

    def set_up(self):
        """All late initialization comes here."""
        # You can override self.TRIAL_ROOT.
        if not self.TRIAL_ROOT:
            # Was not yet initialized.
            TrialDir.TRIAL_ROOT = os.path.realpath(
                tempfile.mkdtemp(prefix='trial'))
            atexit.register(self._clean)
        self.root_dir = os.path.join(TrialDir.TRIAL_ROOT, self.subdir)
        gclient_utils.rmtree(self.root_dir)
        os.makedirs(self.root_dir)

    def tear_down(self):
        """Cleans the trial subdirectory for this instance."""
        if not self.leak:
            logging.debug('Removing %s' % self.root_dir)
            gclient_utils.rmtree(self.root_dir)
        else:
            logging.error('Leaking %s' % self.root_dir)
        self.root_dir = None

    @staticmethod
    def _clean():
        """Cleans the root trial directory."""
        if not TrialDir.SHOULD_LEAK:
            logging.debug('Removing %s' % TrialDir.TRIAL_ROOT)
            gclient_utils.rmtree(TrialDir.TRIAL_ROOT)
        else:
            logging.error('Leaking %s' % TrialDir.TRIAL_ROOT)


class TrialDirMixIn(object):
    def setUp(self):
        # Create a specific directory just for the test.
        self.trial = TrialDir(self.id())
        self.trial.set_up()

    def tearDown(self):
        self.trial.tear_down()

    @property
    def root_dir(self):
        return self.trial.root_dir


class TestCase(unittest.TestCase, TrialDirMixIn):
    """Base unittest class that cleans off a trial directory in tearDown()."""
    def setUp(self):
        unittest.TestCase.setUp(self)
        TrialDirMixIn.setUp(self)

    def tearDown(self):
        TrialDirMixIn.tearDown(self)
        unittest.TestCase.tearDown(self)


if '-l' in sys.argv:
    # See SHOULD_LEAK definition in TrialDir for its purpose.
    TrialDir.SHOULD_LEAK = True
    print('Leaking!')
    sys.argv.remove('-l')
