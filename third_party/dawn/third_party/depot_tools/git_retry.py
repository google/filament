#!/usr/bin/env python3
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Generic retry wrapper for Git operations.

This is largely DEPRECATED in favor of the Infra Git wrapper:
https://chromium.googlesource.com/infra/infra/+/HEAD/go/src/infra/tools/git
"""

import logging
import optparse
import os
import subprocess
import sys
import threading
import time

from git_common import GIT_EXE, GIT_TRANSIENT_ERRORS_RE


class TeeThread(threading.Thread):
    def __init__(self, fd, out_fd, name):
        super(TeeThread, self).__init__(name='git-retry.tee.%s' % (name, ))
        self.data = None
        self.fd = fd
        self.out_fd = out_fd

    def run(self):
        chunks = []
        for line in self.fd:
            line = line.decode('utf-8')
            chunks.append(line)
            self.out_fd.write(line)
        self.data = ''.join(chunks)


class GitRetry(object):

    logger = logging.getLogger('git-retry')
    DEFAULT_DELAY_SECS = 3.0
    DEFAULT_RETRY_COUNT = 5

    def __init__(self, retry_count=None, delay=None, delay_factor=None):
        self.retry_count = retry_count or self.DEFAULT_RETRY_COUNT
        self.delay = max(delay, 0) if delay else 0
        self.delay_factor = max(delay_factor, 0) if delay_factor else 0

    def shouldRetry(self, stderr):
        m = GIT_TRANSIENT_ERRORS_RE.search(stderr)
        if not m:
            return False
        self.logger.info("Encountered known transient error: [%s]",
                         stderr[m.start():m.end()])
        return True

    @staticmethod
    def execute(*args):
        args = (GIT_EXE, ) + args
        proc = subprocess.Popen(
            args,
            stderr=subprocess.PIPE,
        )
        stderr_tee = TeeThread(proc.stderr, sys.stderr, 'stderr')

        # Start our process. Collect/tee 'stdout' and 'stderr'.
        stderr_tee.start()
        try:
            proc.wait()
        except KeyboardInterrupt:
            proc.kill()
            raise
        finally:
            stderr_tee.join()
        return proc.returncode, None, stderr_tee.data

    def computeDelay(self, iteration):
        """Returns: the delay (in seconds) for a given iteration

        The first iteration has a delay of '0'.

        Args:
            iteration: (int) The iteration index (starting with zero as the
                first iteration)
        """
        if (not self.delay) or (iteration == 0):
            return 0
        if self.delay_factor == 0:
            # Linear delay
            return iteration * self.delay
        # Exponential delay
        return (self.delay_factor**(iteration - 1)) * self.delay

    def __call__(self, *args):
        returncode = 0
        for i in range(self.retry_count):
            # If the previous run failed and a delay is configured, delay before
            # the next run.
            delay = self.computeDelay(i)
            if delay > 0:
                self.logger.info("Delaying for [%s second(s)] until next retry",
                                 delay)
                time.sleep(delay)

            self.logger.debug("Executing subprocess (%d/%d) with arguments: %s",
                              (i + 1), self.retry_count, args)
            returncode, _, stderr = self.execute(*args)

            self.logger.debug("Process terminated with return code: %d",
                              returncode)
            if returncode == 0:
                break

            if not self.shouldRetry(stderr):
                self.logger.error(
                    "Process failure was not known to be transient; "
                    "terminating with return code %d", returncode)
                break
        return returncode


def main(args):
    # If we're using the Infra Git wrapper, do nothing here.
    # https://chromium.googlesource.com/infra/infra/+/HEAD/go/src/infra/tools/git
    if 'INFRA_GIT_WRAPPER' in os.environ:
        # Remove Git's execution path from PATH so that our call-through
        # re-invokes the Git wrapper. See crbug.com/721450
        env = os.environ.copy()
        git_exec = subprocess.check_output([GIT_EXE, '--exec-path']).strip()
        env['PATH'] = os.pathsep.join([
            elem for elem in env.get('PATH', '').split(os.pathsep)
            if elem != git_exec
        ])
        return subprocess.call([GIT_EXE] + args, env=env)

    parser = optparse.OptionParser()
    parser.disable_interspersed_args()
    parser.add_option(
        '-v',
        '--verbose',
        action='count',
        default=0,
        help="Increase verbosity; can be specified multiple times")
    parser.add_option('-c',
                      '--retry-count',
                      metavar='COUNT',
                      type=int,
                      default=GitRetry.DEFAULT_RETRY_COUNT,
                      help="Number of times to retry (default=%default)")
    parser.add_option('-d',
                      '--delay',
                      metavar='SECONDS',
                      type=float,
                      default=GitRetry.DEFAULT_DELAY_SECS,
                      help="Specifies the amount of time (in seconds) to wait "
                      "between successive retries (default=%default). This "
                      "can be zero.")
    parser.add_option(
        '-D',
        '--delay-factor',
        metavar='FACTOR',
        type=int,
        default=2,
        help="The exponential factor to apply to delays in between "
        "successive failures (default=%default). If this is "
        "zero, delays will increase linearly. Set this to "
        "one to have a constant (non-increasing) delay.")

    opts, args = parser.parse_args(args)

    # Configure logging verbosity
    if opts.verbose == 0:
        logging.getLogger().setLevel(logging.WARNING)
    elif opts.verbose == 1:
        logging.getLogger().setLevel(logging.INFO)
    else:
        logging.getLogger().setLevel(logging.DEBUG)

    # Execute retries
    retry = GitRetry(
        retry_count=opts.retry_count,
        delay=opts.delay,
        delay_factor=opts.delay_factor,
    )
    return retry(*args)


if __name__ == '__main__':
    logging.basicConfig()
    logging.getLogger().setLevel(logging.WARNING)
    try:
        sys.exit(main(sys.argv[2:]))
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
