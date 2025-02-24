# Licensed under the Apache License: http://www.apache.org/licenses/LICENSE-2.0
# For details: https://bitbucket.org/ned/coveragepy/src/default/NOTICE.txt

"""Reporter foundation for coverage.py."""

import os

from coverage.files import prep_patterns, FnmatchMatcher
from coverage.misc import CoverageException, NoSource, NotPython, isolate_module

os = isolate_module(os)


class Reporter(object):
    """A base class for all reporters."""

    def __init__(self, coverage, config):
        """Create a reporter.

        `coverage` is the coverage instance. `config` is an instance  of
        CoverageConfig, for controlling all sorts of behavior.

        """
        self.coverage = coverage
        self.config = config

        # The FileReporters to report on.  Set by find_file_reporters.
        self.file_reporters = []

        # The directory into which to place the report, used by some derived
        # classes.
        self.directory = None

    def find_file_reporters(self, morfs):
        """Find the FileReporters we'll report on.

        `morfs` is a list of modules or file names.

        """
        reporters = self.coverage._get_file_reporters(morfs)

        if self.config.include:
            matcher = FnmatchMatcher(prep_patterns(self.config.include))
            reporters = [fr for fr in reporters if matcher.match(fr.filename)]

        if self.config.omit:
            matcher = FnmatchMatcher(prep_patterns(self.config.omit))
            reporters = [fr for fr in reporters if not matcher.match(fr.filename)]

        self.file_reporters = sorted(reporters)

    def report_files(self, report_fn, morfs, directory=None):
        """Run a reporting function on a number of morfs.

        `report_fn` is called for each relative morf in `morfs`.  It is called
        as::

            report_fn(file_reporter, analysis)

        where `file_reporter` is the `FileReporter` for the morf, and
        `analysis` is the `Analysis` for the morf.

        """
        self.find_file_reporters(morfs)

        if not self.file_reporters:
            raise CoverageException("No data to report.")

        self.directory = directory
        if self.directory and not os.path.exists(self.directory):
            os.makedirs(self.directory)

        for fr in self.file_reporters:
            try:
                report_fn(fr, self.coverage._analyze(fr))
            except NoSource:
                if not self.config.ignore_errors:
                    raise
            except NotPython:
                # Only report errors for .py files, and only if we didn't
                # explicitly suppress those errors.
                # NotPython is only raised by PythonFileReporter, which has a
                # should_be_python() method.
                if fr.should_be_python() and not self.config.ignore_errors:
                    raise
