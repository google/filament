# Licensed under the Apache License: http://www.apache.org/licenses/LICENSE-2.0
# For details: https://bitbucket.org/ned/coveragepy/src/default/NOTICE.txt

"""Mixin classes to help make good tests."""

import atexit
import collections
import contextlib
import os
import random
import shutil
import sys
import tempfile
import textwrap

from coverage.backunittest import TestCase
from coverage.backward import StringIO, to_bytes


class Tee(object):
    """A file-like that writes to all the file-likes it has."""

    def __init__(self, *files):
        """Make a Tee that writes to all the files in `files.`"""
        self._files = files
        if hasattr(files[0], "encoding"):
            self.encoding = files[0].encoding

    def write(self, data):
        """Write `data` to all the files."""
        for f in self._files:
            f.write(data)

    def flush(self):
        """Flush the data on all the files."""
        for f in self._files:
            f.flush()

    if 0:
        # Use this if you need to use a debugger, though it makes some tests
        # fail, I'm not sure why...
        def __getattr__(self, name):
            return getattr(self._files[0], name)


@contextlib.contextmanager
def change_dir(new_dir):
    """Change directory, and then change back.

    Use as a context manager, it will give you the new directory, and later
    restore the old one.

    """
    old_dir = os.getcwd()
    os.chdir(new_dir)
    try:
        yield os.getcwd()
    finally:
        os.chdir(old_dir)


@contextlib.contextmanager
def saved_sys_path():
    """Save sys.path, and restore it later."""
    old_syspath = sys.path[:]
    try:
        yield
    finally:
        sys.path = old_syspath


def setup_with_context_manager(testcase, cm):
    """Use a contextmanager to setUp a test case.

    If you have a context manager you like::

        with ctxmgr(a, b, c) as v:
            # do something with v

    and you want to have that effect for a test case, call this function from
    your setUp, and it will start the context manager for your test, and end it
    when the test is done::

        def setUp(self):
            self.v = setup_with_context_manager(self, ctxmgr(a, b, c))

        def test_foo(self):
            # do something with self.v

    """
    val = cm.__enter__()
    testcase.addCleanup(cm.__exit__, None, None, None)
    return val


class ModuleAwareMixin(TestCase):
    """A test case mixin that isolates changes to sys.modules."""

    def setUp(self):
        super(ModuleAwareMixin, self).setUp()

        # Record sys.modules here so we can restore it in cleanup_modules.
        self.old_modules = list(sys.modules)
        self.addCleanup(self.cleanup_modules)

    def cleanup_modules(self):
        """Remove any new modules imported during the test run.

        This lets us import the same source files for more than one test.

        """
        for m in [m for m in sys.modules if m not in self.old_modules]:
            del sys.modules[m]


class SysPathAwareMixin(TestCase):
    """A test case mixin that isolates changes to sys.path."""

    def setUp(self):
        super(SysPathAwareMixin, self).setUp()
        setup_with_context_manager(self, saved_sys_path())


class EnvironmentAwareMixin(TestCase):
    """A test case mixin that isolates changes to the environment."""

    def setUp(self):
        super(EnvironmentAwareMixin, self).setUp()

        # Record environment variables that we changed with set_environ.
        self.environ_undos = {}

        self.addCleanup(self.cleanup_environ)

    def set_environ(self, name, value):
        """Set an environment variable `name` to be `value`.

        The environment variable is set, and record is kept that it was set,
        so that `cleanup_environ` can restore its original value.

        """
        if name not in self.environ_undos:
            self.environ_undos[name] = os.environ.get(name)
        os.environ[name] = value

    def cleanup_environ(self):
        """Undo all the changes made by `set_environ`."""
        for name, value in self.environ_undos.items():
            if value is None:
                del os.environ[name]
            else:
                os.environ[name] = value


class StdStreamCapturingMixin(TestCase):
    """A test case mixin that captures stdout and stderr."""

    def setUp(self):
        super(StdStreamCapturingMixin, self).setUp()

        # Capture stdout and stderr so we can examine them in tests.
        # nose keeps stdout from littering the screen, so we can safely Tee it,
        # but it doesn't capture stderr, so we don't want to Tee stderr to the
        # real stderr, since it will interfere with our nice field of dots.
        self.old_stdout = sys.stdout
        self.captured_stdout = StringIO()
        sys.stdout = Tee(sys.stdout, self.captured_stdout)

        self.old_stderr = sys.stderr
        self.captured_stderr = StringIO()
        sys.stderr = self.captured_stderr

        self.addCleanup(self.cleanup_std_streams)

    def cleanup_std_streams(self):
        """Restore stdout and stderr."""
        sys.stdout = self.old_stdout
        sys.stderr = self.old_stderr

    def stdout(self):
        """Return the data written to stdout during the test."""
        return self.captured_stdout.getvalue()

    def stderr(self):
        """Return the data written to stderr during the test."""
        return self.captured_stderr.getvalue()


class TempDirMixin(SysPathAwareMixin, ModuleAwareMixin, TestCase):
    """A test case mixin that creates a temp directory and files in it.

    Includes SysPathAwareMixin and ModuleAwareMixin, because making and using
    temp directories like this will also need that kind of isolation.

    """

    # Our own setting: most of these tests run in their own temp directory.
    # Set this to False in your subclass if you don't want a temp directory
    # created.
    run_in_temp_dir = True

    # Set this if you aren't creating any files with make_file, but still want
    # the temp directory.  This will stop the test behavior checker from
    # complaining.
    no_files_in_temp_dir = False

    def setUp(self):
        super(TempDirMixin, self).setUp()

        if self.run_in_temp_dir:
            # Create a temporary directory.
            self.temp_dir = self.make_temp_dir("test_cover")
            self.chdir(self.temp_dir)

            # Modules should be importable from this temp directory.  We don't
            # use '' because we make lots of different temp directories and
            # nose's caching importer can get confused.  The full path prevents
            # problems.
            sys.path.insert(0, os.getcwd())

        class_behavior = self.class_behavior()
        class_behavior.tests += 1
        class_behavior.temp_dir = self.run_in_temp_dir
        class_behavior.no_files_ok = self.no_files_in_temp_dir

        self.addCleanup(self.check_behavior)

    def make_temp_dir(self, slug="test_cover"):
        """Make a temp directory that is cleaned up when the test is done."""
        name = "%s_%08d" % (slug, random.randint(0, 99999999))
        temp_dir = os.path.join(tempfile.gettempdir(), name)
        os.makedirs(temp_dir)
        self.addCleanup(shutil.rmtree, temp_dir)
        return temp_dir

    def chdir(self, new_dir):
        """Change directory, and change back when the test is done."""
        old_dir = os.getcwd()
        os.chdir(new_dir)
        self.addCleanup(os.chdir, old_dir)

    def check_behavior(self):
        """Check that we did the right things."""

        class_behavior = self.class_behavior()
        if class_behavior.test_method_made_any_files:
            class_behavior.tests_making_files += 1

    def make_file(self, filename, text="", newline=None):
        """Create a file for testing.

        `filename` is the relative path to the file, including directories if
        desired, which will be created if need be.

        `text` is the content to create in the file, a native string (bytes in
        Python 2, unicode in Python 3).

        If `newline` is provided, it is a string that will be used as the line
        endings in the created file, otherwise the line endings are as provided
        in `text`.

        Returns `filename`.

        """
        # Tests that call `make_file` should be run in a temp environment.
        assert self.run_in_temp_dir
        self.class_behavior().test_method_made_any_files = True

        text = textwrap.dedent(text)
        if newline:
            text = text.replace("\n", newline)

        # Make sure the directories are available.
        dirs, _ = os.path.split(filename)
        if dirs and not os.path.exists(dirs):
            os.makedirs(dirs)

        # Create the file.
        with open(filename, 'wb') as f:
            f.write(to_bytes(text))

        return filename

    # We run some tests in temporary directories, because they may need to make
    # files for the tests. But this is expensive, so we can change per-class
    # whether a temp directory is used or not.  It's easy to forget to set that
    # option properly, so we track information about what the tests did, and
    # then report at the end of the process on test classes that were set
    # wrong.

    class ClassBehavior(object):
        """A value object to store per-class."""
        def __init__(self):
            self.tests = 0
            self.skipped = 0
            self.temp_dir = True
            self.no_files_ok = False
            self.tests_making_files = 0
            self.test_method_made_any_files = False

    # Map from class to info about how it ran.
    class_behaviors = collections.defaultdict(ClassBehavior)

    @classmethod
    def report_on_class_behavior(cls):
        """Called at process exit to report on class behavior."""
        for test_class, behavior in cls.class_behaviors.items():
            bad = ""
            if behavior.tests <= behavior.skipped:
                bad = ""
            elif behavior.temp_dir and behavior.tests_making_files == 0:
                if not behavior.no_files_ok:
                    bad = "Inefficient"
            elif not behavior.temp_dir and behavior.tests_making_files > 0:
                bad = "Unsafe"

            if bad:
                if behavior.temp_dir:
                    where = "in a temp directory"
                else:
                    where = "without a temp directory"
                print(
                    "%s: %s ran %d tests, %d made files %s" % (
                        bad,
                        test_class.__name__,
                        behavior.tests,
                        behavior.tests_making_files,
                        where,
                    )
                )

    def class_behavior(self):
        """Get the ClassBehavior instance for this test."""
        return self.class_behaviors[self.__class__]

# When the process ends, find out about bad classes.
atexit.register(TempDirMixin.report_on_class_behavior)
