#!/usr/bin/env vpython3
# Copyright (c) 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import logging
import os
import re
import sys

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GCLIENT_PATH = os.path.join(ROOT_DIR, 'gclient')
sys.path.insert(0, ROOT_DIR)

import subprocess2
from testing_support import fake_repos


class GClientSmokeBase(fake_repos.FakeReposTestBase):
    def setUp(self):
        super(GClientSmokeBase, self).setUp()
        # Make sure it doesn't try to auto update when testing!
        self.env = os.environ.copy()
        self.env['DEPOT_TOOLS_UPDATE'] = '0'
        self.env['DEPOT_TOOLS_METRICS'] = '0'
        # Suppress Python 3 warnings and other test undesirables.
        self.env['GCLIENT_TEST'] = '1'
        self.maxDiff = None

    def gclient(self, cmd, cwd=None, error_ok=False):
        if not cwd:
            cwd = self.root_dir
        cmd = [GCLIENT_PATH] + cmd
        process = subprocess2.Popen(cmd,
                                    cwd=cwd,
                                    env=self.env,
                                    stdout=subprocess2.PIPE,
                                    stderr=subprocess2.PIPE,
                                    universal_newlines=True)
        (stdout, stderr) = process.communicate()
        logging.debug("XXX: %s\n%s\nXXX" % (' '.join(cmd), stdout))
        logging.debug("YYY: %s\n%s\nYYY" % (' '.join(cmd), stderr))

        if not error_ok:
            self.assertEqual(0, process.returncode, stderr)

        return (stdout.replace('\r\n',
                               '\n'), stderr.replace('\r\n',
                                                     '\n'), process.returncode)

    def untangle(self, stdout):
        """Separates output based on thread IDs."""
        tasks = {}
        remaining = []
        task_id = 0
        for line in stdout.splitlines(False):
            m = re.match(r'^(\d)+>(.*)$', line)
            if not m:
                if task_id:
                    # Lines broken with carriage breaks don't have a thread ID,
                    # but belong to the last seen thread ID.
                    tasks.setdefault(task_id, []).append(line)
                else:
                    remaining.append(line)
            else:
                self.assertEqual([], remaining)
                task_id = int(m.group(1))
                tasks.setdefault(task_id, []).append(m.group(2))
        out = []
        for key in sorted(tasks.keys()):
            out.extend(tasks[key])
        out.extend(remaining)
        return '\n'.join(out)

    def parseGclient(self, cmd, items, expected_stderr='', untangle=False):
        """Parse gclient's output to make it easier to test.
    If untangle is True, tries to sort out the output from parallel checkout."""
        (stdout, stderr, _) = self.gclient(cmd)
        if untangle:
            stdout = self.untangle(stdout)
        self.checkString(expected_stderr, stderr)
        return self.checkBlock(stdout, items)

    def splitBlock(self, stdout):
        """Split gclient's output into logical execution blocks.
        ___ running 'foo' at '/bar'
        (...)
        ___ running 'baz' at '/bar'
        (...)

        will result in 2 items of len((...).splitlines()) each.
        """
        results = []
        for line in stdout.splitlines(False):
            # Intentionally skips empty lines.
            if not line:
                continue
            if not line.startswith('__'):
                if results:
                    results[-1].append(line)
                else:
                    # TODO(maruel): gclient's git stdout is inconsistent.
                    # This should fail the test instead!!
                    pass
                continue

            match = re.match(r'^________ ([a-z]+) \'(.*)\' in \'(.*)\'$', line)
            if match:
                results.append(
                    [[match.group(1),
                      match.group(2),
                      match.group(3)]])
                continue

            match = re.match(r'^_____ (.*) is missing, syncing instead$', line)
            if match:
                # Blah, it's when a dependency is deleted, we should probably
                # not output this message.
                results.append([line])
                continue

            # These two regexps are a bit too broad, they are necessary only for
            # git checkouts.
            if (re.match(r'_____ [^ ]+ at [^ ]+', line) or re.match(
                    r'_____ [^ ]+ : Attempting rebase onto [0-9a-f]+...',
                    line)):
                continue

            # Fail for any unrecognized lines that start with '__'.
            self.fail(line)
        return results

    def checkBlock(self, stdout, items):
        results = self.splitBlock(stdout)
        for i in range(min(len(results), len(items))):
            if isinstance(items[i], (list, tuple)):
                verb = items[i][0]
                path = items[i][1]
            else:
                verb = items[i]
                path = self.root_dir
            self.checkString(results[i][0][0], verb,
                             (i, results[i][0][0], verb))
            if sys.platform == 'win32':
                # Make path lower case since casing can change randomly.
                self.checkString(results[i][0][2].lower(), path.lower(),
                                 (i, results[i][0][2].lower(), path.lower()))
            else:
                self.checkString(results[i][0][2], path,
                                 (i, results[i][0][2], path))
        self.assertEqual(len(results), len(items),
                         (stdout, items, len(results)))
        return results
