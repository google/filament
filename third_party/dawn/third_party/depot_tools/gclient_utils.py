# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Generic utils."""

import codecs
import collections
import contextlib
import datetime
import errno
import functools
import io
import logging
import operator
import os
import platform
import queue
import re
import shlex
import shutil
import stat
import subprocess
import sys
import tempfile
import threading
import time
import urllib.parse

import subprocess2

# Git wrapper retries on a transient error, and some callees do retries too,
# such as GitWrapper.update (doing clone). One retry attempt should be
# sufficient to help with any transient errors at this level.
RETRY_MAX = 1
RETRY_INITIAL_SLEEP = 2  # in seconds
START = datetime.datetime.now()

_WARNINGS = []

# These repos are known to cause OOM errors on 32-bit platforms, due the the
# very large objects they contain.  It is not safe to use threaded index-pack
# when cloning/fetching them.
THREADED_INDEX_PACK_BLOCKLIST = [
    'https://chromium.googlesource.com/chromium/reference_builds/chrome_win.git'
]


def reraise(typ, value, tb=None):
    """To support rethrowing exceptions with tracebacks."""
    if value is None:
        value = typ()
    if value.__traceback__ is not tb:
        raise value.with_traceback(tb)
    raise value


class Error(Exception):
    """gclient exception class."""
    def __init__(self, msg, *args, **kwargs):
        index = getattr(threading.current_thread(), 'index', 0)
        if index:
            msg = '\n'.join('%d> %s' % (index, l) for l in msg.splitlines())
        super(Error, self).__init__(msg, *args, **kwargs)


def Elapsed(until=None):
    if until is None:
        until = datetime.datetime.now()
    return str(until - START).partition('.')[0]


def PrintWarnings():
    """Prints any accumulated warnings."""
    if _WARNINGS:
        print('\n\nWarnings:', file=sys.stderr)
        for warning in _WARNINGS:
            print(warning, file=sys.stderr)


def AddWarning(msg):
    """Adds the given warning message to the list of accumulated warnings."""
    _WARNINGS.append(msg)


def FuzzyMatchRepo(repo, candidates):
    # type: (str, Union[Collection[str], Mapping[str, Any]]) -> Optional[str]
    """Attempts to find a representation of repo in the candidates.

    Args:
        repo: a string representation of a repo in the form of a url or the
            name and path of the solution it represents.
        candidates: The candidates to look through which may contain `repo` in
            in any of the forms mentioned above.
    Returns:
        The matching string, if any, which may be in a different form from
        `repo`.
    """
    if repo in candidates:
        return repo
    if repo.endswith('.git') and repo[:-len('.git')] in candidates:
        return repo[:-len('.git')]
    if repo + '.git' in candidates:
        return repo + '.git'
    return None


def SplitUrlRevision(url):
    """Splits url and returns a two-tuple: url, rev."""
    if url.startswith('ssh:'):
        # Make sure ssh://user-name@example.com/~/test.git@stable works
        regex = r'(ssh://(?:[-.\w]+@)?[-\w:\.]+/[-~\w\./]+)(?:@(.+))?'
        components = re.search(regex, url).groups()
    else:
        components = url.rsplit('@', 1)
        if re.match(r'^\w+\@', url) and '@' not in components[0]:
            components = [url]

        if len(components) == 1:
            components += [None]
    return tuple(components)


def ExtractRefName(remote, full_refs_str):
    """Returns the ref name if full_refs_str is a valid ref."""
    result = re.compile(
        r'^refs(\/.+)?\/((%s)|(heads)|(tags))\/(?P<ref_name>.+)' %
        remote).match(full_refs_str)
    if result:
        return result.group('ref_name')
    return None


def IsGitSha(revision):
    """Returns true if the given string is a valid hex-encoded sha."""
    return re.match('^[a-fA-F0-9]{6,40}$', revision) is not None


def IsFullGitSha(revision):
    """Returns true if the given string is a valid hex-encoded full sha."""
    return re.match('^[a-fA-F0-9]{40}$', revision) is not None


def IsDateRevision(revision):
    """Returns true if the given revision is of the form "{ ... }"."""
    return bool(revision and re.match(r'^\{.+\}$', str(revision)))


def MakeDateRevision(date):
    """Returns a revision representing the latest revision before the given
    date."""
    return "{" + date + "}"


def SyntaxErrorToError(filename, e):
    """Raises a gclient_utils.Error exception with a human readable message."""
    try:
        # Try to construct a human readable error message
        if filename:
            error_message = 'There is a syntax error in %s\n' % filename
        else:
            error_message = 'There is a syntax error\n'
        error_message += 'Line #%s, character %s: "%s"' % (
            e.lineno, e.offset, re.sub(r'[\r\n]*$', '', e.text))
    except:
        # Something went wrong, re-raise the original exception
        raise e
    else:
        raise Error(error_message)


class PrintableObject(object):
    def __str__(self):
        output = ''
        for i in dir(self):
            if i.startswith('__'):
                continue
            output += '%s = %s\n' % (i, str(getattr(self, i, '')))
        return output


def AskForData(message):
    # Try to load the readline module, so that "elaborate line editing" features
    # such as backspace work for `raw_input` / `input`.
    try:
        import readline
    except ImportError:
        # The readline module does not exist in all Python distributions, e.g.
        # on Windows. Fall back to simple input handling.
        pass

    # Use this so that it can be mocked in tests.
    try:
        return input(message)
    except KeyboardInterrupt:
        # Hide the exception.
        sys.exit(1)


def FileRead(filename, mode='rbU'):
    # mode is ignored now; we always return unicode strings.
    with open(filename, mode='rb') as f:
        s = f.read()
    try:
        return s.decode('utf-8', 'replace')
    except (UnicodeDecodeError, AttributeError):
        return s


def FileWrite(filename, content, mode='w', encoding='utf-8'):
    with codecs.open(filename, mode=mode, encoding=encoding) as f:
        f.write(content)


@contextlib.contextmanager
def temporary_directory(**kwargs):
    tdir = tempfile.mkdtemp(**kwargs)
    try:
        yield tdir
    finally:
        if tdir:
            rmtree(tdir)


@contextlib.contextmanager
def temporary_file():
    """Creates a temporary file.

    On Windows, a file must be closed before it can be opened again. This
    function allows to write something like:

        with gclient_utils.temporary_file() as tmp:
            gclient_utils.FileWrite(tmp, foo)
            useful_stuff(tmp)

    Instead of something like:

        with tempfile.NamedTemporaryFile(delete=False) as tmp:
            tmp.write(foo)
            tmp.close()
            try:
                useful_stuff(tmp)
            finally:
                os.remove(tmp.name)
    """
    handle, name = tempfile.mkstemp()
    os.close(handle)
    try:
        yield name
    finally:
        os.remove(name)


def safe_rename(old, new):
    """Renames a file reliably.

    Sometimes os.rename does not work because a dying git process keeps a handle
    on it for a few seconds. An exception is then thrown, which make the program
    give up what it was doing and remove what was deleted.
    The only solution is to catch the exception and try again until it works.
    """
    # roughly 10s
    retries = 100
    for i in range(retries):
        try:
            os.rename(old, new)
            break
        except OSError:
            if i == (retries - 1):
                # Give up.
                raise
            # retry
            logging.debug("Renaming failed from %s to %s. Retrying ..." %
                          (old, new))
            time.sleep(0.1)


def rm_file_or_tree(path):
    if os.path.isfile(path) or os.path.islink(path):
        os.remove(path)
    else:
        rmtree(path)


def rmtree(path):
    """shutil.rmtree() on steroids.

    Recursively removes a directory, even if it's marked read-only.

    shutil.rmtree() doesn't work on Windows if any of the files or directories
    are read-only. We need to be able to force the files to be writable (i.e.,
    deletable) as we traverse the tree.

    Even with all this, Windows still sometimes fails to delete a file, citing
    a permission error (maybe something to do with antivirus scans or disk
    indexing).  The best suggestion any of the user forums had was to wait a
    bit and try again, so we do that too.  It's hand-waving, but sometimes it
    works. :/

    On POSIX systems, things are a little bit simpler.  The modes of the files
    to be deleted doesn't matter, only the modes of the directories containing
    them are significant.  As the directory tree is traversed, each directory
    has its mode set appropriately before descending into it.  This should
    result in the entire tree being removed, with the possible exception of
    *path itself, because nothing attempts to change the mode of its parent.
    Doing so would be hazardous, as it's not a directory slated for removal.
    In the ordinary case, this is not a problem: for our purposes, the user
    will never lack write permission on *path's parent.
    """
    if not os.path.exists(path):
        return

    if os.path.islink(path) or not os.path.isdir(path):
        raise Error('Called rmtree(%s) in non-directory' % path)

    if sys.platform == 'win32':
        # Give up and use cmd.exe's rd command.
        path = os.path.normcase(path)
        for _ in range(3):
            exitcode = subprocess.call(
                ['cmd.exe', '/c', 'rd', '/q', '/s', path])
            if exitcode == 0:
                return

            print('rd exited with code %d' % exitcode, file=sys.stderr)
            time.sleep(3)
        raise Exception('Failed to remove path %s' % path)

    # On POSIX systems, we need the x-bit set on the directory to access it,
    # the r-bit to see its contents, and the w-bit to remove files from it.
    # The actual modes of the files within the directory is irrelevant.
    os.chmod(path, stat.S_IRUSR | stat.S_IWUSR | stat.S_IXUSR)

    def remove(func, subpath):
        func(subpath)

    for fn in os.listdir(path):
        # If fullpath is a symbolic link that points to a directory, isdir will
        # be True, but we don't want to descend into that as a directory, we
        # just want to remove the link.  Check islink and treat links as
        # ordinary files would be treated regardless of what they reference.
        fullpath = os.path.join(path, fn)
        if os.path.islink(fullpath) or not os.path.isdir(fullpath):
            remove(os.remove, fullpath)
        else:
            # Recurse.
            rmtree(fullpath)

    remove(os.rmdir, path)


def safe_makedirs(tree):
    """Creates the directory in a safe manner.

    Because multiple threads can create these directories concurrently, trap the
    exception and pass on.
    """
    count = 0
    while not os.path.exists(tree):
        count += 1
        try:
            os.makedirs(tree)
        except OSError as e:
            # 17 POSIX, 183 Windows
            if e.errno not in (17, 183):
                raise
            if count > 40:
                # Give up.
                raise


def CommandToStr(args):
    """Converts an arg list into a shell escaped string."""
    return ' '.join(shlex.quote(arg) for arg in args)


class Wrapper(object):
    """Wraps an object, acting as a transparent proxy for all properties by
    default.
    """
    def __init__(self, wrapped):
        self._wrapped = wrapped

    def __getattr__(self, name):
        return getattr(self._wrapped, name)


class AutoFlush(Wrapper):
    """Creates a file object clone to automatically flush after N seconds."""
    def __init__(self, wrapped, delay):
        super(AutoFlush, self).__init__(wrapped)
        if not hasattr(self, 'lock'):
            self.lock = threading.Lock()
        self.__last_flushed_at = time.time()
        self.delay = delay

    @property
    def autoflush(self):
        return self

    def write(self, out, *args, **kwargs):
        self._wrapped.write(out, *args, **kwargs)
        should_flush = False
        self.lock.acquire()
        try:
            if self.delay and (time.time() -
                               self.__last_flushed_at) > self.delay:
                should_flush = True
                self.__last_flushed_at = time.time()
        finally:
            self.lock.release()
        if should_flush:
            self.flush()


class Annotated(Wrapper):
    """Creates a file object clone to automatically prepends every line in
    worker threads with a NN> prefix.
    """
    def __init__(self, wrapped, include_zero=False):
        super(Annotated, self).__init__(wrapped)
        if not hasattr(self, 'lock'):
            self.lock = threading.Lock()
        self.__output_buffers = {}
        self.__include_zero = include_zero
        self._wrapped_write = getattr(self._wrapped, 'buffer',
                                      self._wrapped).write

    @property
    def annotated(self):
        return self

    def write(self, out):
        # Store as bytes to ensure Unicode characters get output correctly.
        if not isinstance(out, bytes):
            out = out.encode('utf-8')

        index = getattr(threading.current_thread(), 'index', 0)
        if not index and not self.__include_zero:
            # Unindexed threads aren't buffered.
            return self._wrapped_write(out)

        self.lock.acquire()
        try:
            # Use a dummy array to hold the string so the code can be lockless.
            # Strings are immutable, requiring to keep a lock for the whole
            # dictionary otherwise. Using an array is faster than using a dummy
            # object.
            if not index in self.__output_buffers:
                obj = self.__output_buffers[index] = [b'']
            else:
                obj = self.__output_buffers[index]
        finally:
            self.lock.release()

        # Continue lockless.
        obj[0] += out
        while True:
            cr_loc = obj[0].find(b'\r')
            lf_loc = obj[0].find(b'\n')
            if cr_loc == lf_loc == -1:
                break

            if cr_loc == -1 or (0 <= lf_loc < cr_loc):
                line, remaining = obj[0].split(b'\n', 1)
                if line:
                    self._wrapped_write(b'%d>%s\n' % (index, line))
            elif lf_loc == -1 or (0 <= cr_loc < lf_loc):
                line, remaining = obj[0].split(b'\r', 1)
                if line:
                    self._wrapped_write(b'%d>%s\r' % (index, line))
            obj[0] = remaining

    def flush(self):
        """Flush buffered output."""
        orphans = []
        self.lock.acquire()
        try:
            # Detect threads no longer existing.
            indexes = (getattr(t, 'index', None) for t in threading.enumerate())
            indexes = filter(None, indexes)
            for index in self.__output_buffers:
                if not index in indexes:
                    orphans.append((index, self.__output_buffers[index][0]))
            for orphan in orphans:
                del self.__output_buffers[orphan[0]]
        finally:
            self.lock.release()

        # Don't keep the lock while writing. Will append \n when it shouldn't.
        for orphan in orphans:
            if orphan[1]:
                self._wrapped_write(b'%d>%s\n' % (orphan[0], orphan[1]))
        return self._wrapped.flush()


def MakeFileAutoFlush(fileobj, delay=10):
    autoflush = getattr(fileobj, 'autoflush', None)
    if autoflush:
        autoflush.delay = delay
        return fileobj
    return AutoFlush(fileobj, delay)


def MakeFileAnnotated(fileobj, include_zero=False):
    if getattr(fileobj, 'annotated', None):
        return fileobj
    return Annotated(fileobj, include_zero)


GCLIENT_CHILDREN = []
GCLIENT_CHILDREN_LOCK = threading.Lock()


class GClientChildren(object):
    @staticmethod
    def add(popen_obj):
        with GCLIENT_CHILDREN_LOCK:
            GCLIENT_CHILDREN.append(popen_obj)

    @staticmethod
    def remove(popen_obj):
        with GCLIENT_CHILDREN_LOCK:
            GCLIENT_CHILDREN.remove(popen_obj)

    @staticmethod
    def _attemptToKillChildren():
        global GCLIENT_CHILDREN
        with GCLIENT_CHILDREN_LOCK:
            zombies = [c for c in GCLIENT_CHILDREN if c.poll() is None]

        for zombie in zombies:
            try:
                zombie.kill()
            except OSError:
                pass

        with GCLIENT_CHILDREN_LOCK:
            GCLIENT_CHILDREN = [
                k for k in GCLIENT_CHILDREN if k.poll() is not None
            ]

    @staticmethod
    def _areZombies():
        with GCLIENT_CHILDREN_LOCK:
            return bool(GCLIENT_CHILDREN)

    @staticmethod
    def KillAllRemainingChildren():
        GClientChildren._attemptToKillChildren()

        if GClientChildren._areZombies():
            time.sleep(0.5)
            GClientChildren._attemptToKillChildren()

        with GCLIENT_CHILDREN_LOCK:
            if GCLIENT_CHILDREN:
                print('Could not kill the following subprocesses:',
                      file=sys.stderr)
                for zombie in GCLIENT_CHILDREN:
                    print('  ', zombie.pid, file=sys.stderr)


def CheckCallAndFilter(args,
                       print_stdout=False,
                       filter_fn=None,
                       show_header=False,
                       always_show_header=False,
                       retry=False,
                       **kwargs):
    """Runs a command and calls back a filter function if needed.

    Accepts all subprocess2.Popen() parameters plus:
        print_stdout: If True, the command's stdout is forwarded to stdout.
        filter_fn: A function taking a single string argument called with each
            line of the subprocess2's output. Each line has the trailing
            newline character trimmed.
        show_header: Whether to display a header before the command output.
        always_show_header: Show header even when the command produced no
            output.
        retry: If the process exits non-zero, sleep for a brief interval and
            try again, up to RETRY_MAX times.

    stderr is always redirected to stdout.

    Returns the output of the command as a binary string.
    """
    def show_header_if_necessary(needs_header, attempt):
        """Show the header at most once."""
        if not needs_header[0]:
            return

        needs_header[0] = False
        # Automatically generated header. We only prepend a newline if
        # always_show_header is false, since it usually indicates there's an
        # external progress display, and it's better not to clobber it in that
        # case.
        header = '' if always_show_header else '\n'
        header += '________ running \'%s\' in \'%s\'' % (' '.join(args),
                                                         kwargs.get('cwd', '.'))
        if attempt:
            header += ' attempt %s / %s' % (attempt + 1, RETRY_MAX + 1)
        header += '\n'

        if print_stdout:
            stdout_write = getattr(sys.stdout, 'buffer', sys.stdout).write
            stdout_write(header.encode())
        if filter_fn:
            filter_fn(header)

    def filter_line(command_output, line_start):
        """Extract the last line from command output and filter it."""
        if not filter_fn or line_start is None:
            return
        command_output.seek(line_start)
        filter_fn(command_output.read().decode('utf-8'))

    # Initialize stdout writer if needed. On Python 3, sys.stdout does not
    # accept byte inputs and sys.stdout.buffer must be used instead.
    if print_stdout:
        sys.stdout.flush()
        stdout_write = getattr(sys.stdout, 'buffer', sys.stdout).write
    else:
        stdout_write = lambda _: None

    sleep_interval = RETRY_INITIAL_SLEEP
    run_cwd = kwargs.get('cwd', os.getcwd())

    # Store the output of the command regardless of the value of print_stdout or
    # filter_fn.
    command_output = io.BytesIO()
    for attempt in range(RETRY_MAX + 1):
        # If our stdout is a terminal, then pass in a psuedo-tty pipe to our
        # subprocess when filtering its output. This makes the subproc believe
        # it was launched from a terminal, which will preserve ANSI color codes.
        os_type = GetOperatingSystem()
        if sys.stdout.isatty() and os_type not in ['win', 'aix', 'zos']:
            pipe_reader, pipe_writer = os.openpty()
        else:
            pipe_reader, pipe_writer = os.pipe()

        # subprocess2 will use pseudoterminals (ptys) for the child process,
        # and ptys do not support a terminal size directly, so if we want
        # a hook to be able to customize what it does based on the terminal
        # size, we need to explicitly pass the size information down to
        # the subprocess. Setting COLUMNS and LINES explicitly in the
        # environment will cause those values to be picked up by
        # `shutil.get_terminal_size()` in the subprocess (and of course
        # anything that checks for the env vars direcstly as well).
        if 'env' not in kwargs:
            kwargs['env'] = os.environ.copy()
        if 'COLUMNS' not in kwargs['env'] or 'LINES' not in kwargs['env']:
            size = shutil.get_terminal_size()
            if size.columns and size.lines:
                kwargs['env']['COLUMNS'] = str(size.columns)
                kwargs['env']['LINES'] = str(size.lines)

        kid = subprocess2.Popen(args,
                                bufsize=0,
                                stdout=pipe_writer,
                                stderr=subprocess2.STDOUT,
                                **kwargs)
        # Close the write end of the pipe once we hand it off to the child proc.
        os.close(pipe_writer)

        GClientChildren.add(kid)

        # Passed as a list for "by ref" semantics.
        needs_header = [show_header]
        if always_show_header:
            show_header_if_necessary(needs_header, attempt)

        # Also, we need to forward stdout to prevent weird re-ordering of
        # output. This has to be done on a per byte basis to make sure it is not
        # buffered: normally buffering is done for each line, but if the process
        # requests input, no end-of-line character is output after the prompt
        # and it would not show up.
        try:
            line_start = None
            while True:
                try:
                    in_byte = os.read(pipe_reader, 1)
                except (IOError, OSError) as e:
                    if e.errno == errno.EIO:
                        # An errno.EIO means EOF?
                        in_byte = None
                    else:
                        raise e
                is_newline = in_byte in (b'\n', b'\r')
                if not in_byte:
                    break

                show_header_if_necessary(needs_header, attempt)

                if is_newline:
                    filter_line(command_output, line_start)
                    line_start = None
                elif line_start is None:
                    line_start = command_output.tell()

                stdout_write(in_byte)
                if print_stdout and is_newline:
                    sys.stdout.flush()
                command_output.write(in_byte)

            # Flush the rest of buffered output.
            sys.stdout.flush()
            if line_start is not None:
                filter_line(command_output, line_start)

            os.close(pipe_reader)
            rv = kid.wait()

            # Don't put this in a 'finally,' since the child may still run if we
            # get an exception.
            GClientChildren.remove(kid)

        except KeyboardInterrupt:
            print('Failed while running "%s"' % ' '.join(args), file=sys.stderr)
            raise

        if rv == 0:
            return command_output.getvalue()

        if not retry:
            break

        print("WARNING: subprocess '%s' in %s failed; will retry after a short "
              'nap...' % (' '.join('"%s"' % x for x in args), run_cwd))
        command_output = io.BytesIO()
        time.sleep(sleep_interval)
        sleep_interval *= 2

    raise subprocess2.CalledProcessError(rv, args, kwargs.get('cwd', None),
                                         command_output.getvalue(), None)


class GitFilter(object):
    """A filter_fn implementation for quieting down git output messages.

    Allows a custom function to skip certain lines (predicate), and will
    throttle the output of percentage completed lines to only output every X
    seconds.
    """
    PERCENT_RE = re.compile('(.*) ([0-9]{1,3})% .*')

    def __init__(self, time_throttle=0, predicate=None, out_fh=None):
        """
        Args:
        time_throttle (int): GitFilter will throttle 'noisy' output (such as the
            XX% complete messages) to only be printed at least |time_throttle|
            seconds apart.
        predicate (f(line)): An optional function which is invoked for every
            line. The line will be skipped if predicate(line) returns False.
        out_fh: File handle to write output to.
        """
        self.first_line = True
        self.last_time = 0
        self.time_throttle = time_throttle
        self.predicate = predicate
        self.out_fh = out_fh or sys.stdout
        self.progress_prefix = None

    def __call__(self, line):
        # git uses an escape sequence to clear the line; elide it.
        esc = line.find(chr(0o33))
        if esc > -1:
            line = line[:esc]
        if self.predicate and not self.predicate(line):
            return
        now = time.time()
        match = self.PERCENT_RE.match(line)
        if match:
            if match.group(1) != self.progress_prefix:
                self.progress_prefix = match.group(1)
            elif now - self.last_time < self.time_throttle:
                return
        self.last_time = now
        if not self.first_line:
            self.out_fh.write('[%s] ' % Elapsed())
        self.first_line = False
        print(line, file=self.out_fh)


def FindFileUpwards(filename, path=None):
    """Search upwards from the a directory (default: current) to find a file.

    Returns nearest upper-level directory with the passed in file.
    """
    if not path:
        path = os.getcwd()
    path = os.path.realpath(path)
    while True:
        file_path = os.path.join(path, filename)
        if os.path.exists(file_path):
            return path
        (new_path, _) = os.path.split(path)
        if new_path == path:
            return None
        path = new_path


def GetOperatingSystem():
    """Returns 'mac', 'win', 'linux', or the name of the current platform."""
    if sys.platform.startswith(('cygwin', 'win')):
        return 'win'

    if sys.platform.startswith('linux'):
        return 'linux'

    if sys.platform == 'darwin':
        return 'mac'

    if sys.platform.startswith('aix'):
        return 'aix'

    try:
        return os.uname().sysname.lower()
    except AttributeError:
        return sys.platform


def GetGClientRootAndEntries(path=None):
    """Returns the gclient root and the dict of entries."""
    config_file = '.gclient_entries'
    root = FindFileUpwards(config_file, path)
    if not root:
        print("Can't find %s" % config_file)
        return None
    config_path = os.path.join(root, config_file)
    env = {}
    with open(config_path) as config:
        exec(config.read(), env)
    config_dir = os.path.dirname(config_path)
    return config_dir, env['entries']


def lockedmethod(method):
    """Method decorator that holds self.lock for the duration of the call."""
    def inner(self, *args, **kwargs):
        try:
            try:
                self.lock.acquire()
            except KeyboardInterrupt:
                print('Was deadlocked', file=sys.stderr)
                raise
            return method(self, *args, **kwargs)
        finally:
            self.lock.release()

    return inner


class WorkItem(object):
    """One work item."""
    # On cygwin, creating a lock throwing randomly when nearing ~100 locks.
    # As a workaround, use a single lock. Yep you read it right. Single lock for
    # all the 100 objects.
    lock = threading.Lock()

    def __init__(self, name):
        # A unique string representing this work item.
        self._name = name
        self.outbuf = io.StringIO()
        self.start = self.finish = None
        self.resources = []  # List of resources this work item requires.

    def run(self, work_queue):
        """work_queue is passed as keyword argument so it should be
        the last parameters of the function when you override it."""

    @property
    def name(self):
        return self._name


class ExecutionQueue(object):
    """Runs a set of WorkItem that have interdependencies and were WorkItem are
    added as they are processed.

    This class manages that all the required dependencies are run
    before running each one.

    Methods of this class are thread safe.
    """
    def __init__(self, jobs, progress, ignore_requirements, verbose=False):
        """jobs specifies the number of concurrent tasks to allow. progress is a
        Progress instance."""
        # Set when a thread is done or a new item is enqueued.
        self.ready_cond = threading.Condition()
        # Maximum number of concurrent tasks.
        self.jobs = jobs
        # List of WorkItem, for gclient, these are Dependency instances.
        self.queued = []
        # List of strings representing each Dependency.name that was run.
        self.ran = []
        # List of items currently running.
        self.running = []
        # Exceptions thrown if any.
        self.exceptions = queue.Queue()
        # Progress status
        self.progress = progress
        if self.progress:
            self.progress.update(0)

        self.ignore_requirements = ignore_requirements
        self.verbose = verbose
        self.last_join = None
        self.last_subproc_output = None

    def enqueue(self, d):
        """Enqueue one Dependency to be executed later once its requirements are
        satisfied.
        """
        assert isinstance(d, WorkItem)
        self.ready_cond.acquire()
        try:
            self.queued.append(d)
            total = len(self.queued) + len(self.ran) + len(self.running)
            if self.jobs == 1:
                total += 1
            logging.debug('enqueued(%s)' % d.name)
            if self.progress:
                self.progress._total = total
                self.progress.update(0)
            self.ready_cond.notifyAll()
        finally:
            self.ready_cond.release()

    def out_cb(self, _):
        self.last_subproc_output = datetime.datetime.now()
        return True

    @staticmethod
    def format_task_output(task, comment=''):
        if comment:
            comment = ' (%s)' % comment
        if task.start and task.finish:
            elapsed = ' (Elapsed: %s)' % (str(task.finish -
                                              task.start).partition('.')[0])
        else:
            elapsed = ''
        return """
%s%s%s
----------------------------------------
%s
----------------------------------------""" % (task.name, comment, elapsed,
                                               task.outbuf.getvalue().strip())

    def _is_conflict(self, job):
        """Checks to see if a job will conflict with another running job."""
        for running_job in self.running:
            for used_resource in running_job.item.resources:
                logging.debug('Checking resource %s' % used_resource)
                if used_resource in job.resources:
                    return True
        return False

    def flush(self, *args, **kwargs):
        """Runs all enqueued items until all are executed."""
        kwargs['work_queue'] = self
        self.last_subproc_output = self.last_join = datetime.datetime.now()
        self.ready_cond.acquire()
        try:
            while True:
                # Check for task to run first, then wait.
                while True:
                    if not self.exceptions.empty():
                        # Systematically flush the queue when an exception
                        # logged.
                        self.queued = []
                    self._flush_terminated_threads()
                    if (not self.queued and not self.running
                            or self.jobs == len(self.running)):
                        logging.debug(
                            'No more worker threads or can\'t queue anything.')
                        break

                    # Check for new tasks to start.
                    for i in range(len(self.queued)):
                        # Verify its requirements.
                        if (self.ignore_requirements
                                or not (set(self.queued[i].requirements) -
                                        set(self.ran))):
                            if not self._is_conflict(self.queued[i]):
                                # Start one work item: all its requirements are
                                # satisfied.
                                self._run_one_task(self.queued.pop(i), args,
                                                   kwargs)
                                break
                    else:
                        # Couldn't find an item that could run. Break out the
                        # outher loop.
                        break

                if not self.queued and not self.running:
                    # We're done.
                    break
                # We need to poll here otherwise Ctrl-C isn't processed.
                try:
                    self.ready_cond.wait(10)
                    # If we haven't printed to terminal for a while, but we have
                    # received spew from a suprocess, let the user know we're
                    # still progressing.
                    now = datetime.datetime.now()
                    if (now - self.last_join > datetime.timedelta(seconds=60)
                            and self.last_subproc_output > self.last_join):
                        if self.progress:
                            print('')
                            sys.stdout.flush()
                        elapsed = Elapsed()
                        print('[%s] Still working on:' % elapsed)
                        sys.stdout.flush()
                        for task in self.running:
                            print('[%s]   %s' % (elapsed, task.item.name))
                            sys.stdout.flush()
                except KeyboardInterrupt:
                    # Help debugging by printing some information:
                    print(
                        ('\nAllowed parallel jobs: %d\n# queued: %d\nRan: %s\n'
                         'Running: %d') %
                        (self.jobs, len(self.queued), ', '.join(
                            self.ran), len(self.running)),
                        file=sys.stderr)
                    for i in self.queued:
                        print('%s (not started): %s' %
                              (i.name, ', '.join(i.requirements)),
                              file=sys.stderr)
                    for i in self.running:
                        print(self.format_task_output(i.item, 'interrupted'),
                              file=sys.stderr)
                    raise
                # Something happened: self.enqueue() or a thread terminated.
                # Loop again.
        finally:
            self.ready_cond.release()

        assert not self.running, 'Now guaranteed to be single-threaded'
        if not self.exceptions.empty():
            if self.progress:
                print('')
            # To get back the stack location correctly, the raise a, b, c form
            # must be used, passing a tuple as the first argument doesn't work.
            e, task = self.exceptions.get()
            print(self.format_task_output(task.item, 'ERROR'), file=sys.stderr)
            reraise(e[0], e[1], e[2])
        elif self.progress:
            self.progress.end()

    def _flush_terminated_threads(self):
        """Flush threads that have terminated."""
        running = self.running
        self.running = []
        for t in running:
            if t.is_alive():
                self.running.append(t)
            else:
                t.join()
                self.last_join = datetime.datetime.now()
                sys.stdout.flush()
                if self.verbose:
                    print(self.format_task_output(t.item))
                if self.progress:
                    self.progress.update(1, t.item.name)
                if t.item.name in self.ran:
                    raise Error('gclient is confused, "%s" is already in "%s"' %
                                (t.item.name, ', '.join(self.ran)))
                if not t.item.name in self.ran:
                    self.ran.append(t.item.name)

    def _run_one_task(self, task_item, args, kwargs):
        if self.jobs > 1:
            # Start the thread.
            index = len(self.ran) + len(self.running) + 1
            new_thread = self._Worker(task_item, index, args, kwargs)
            self.running.append(new_thread)
            new_thread.start()
        else:
            # Run the 'thread' inside the main thread. Don't try to catch any
            # exception.
            try:
                task_item.start = datetime.datetime.now()
                print('[%s] Started.' % Elapsed(task_item.start),
                      file=task_item.outbuf)
                task_item.run(*args, **kwargs)
                task_item.finish = datetime.datetime.now()
                print('[%s] Finished.' % Elapsed(task_item.finish),
                      file=task_item.outbuf)
                self.ran.append(task_item.name)
                if self.verbose:
                    if self.progress:
                        print('')
                    print(self.format_task_output(task_item))
                if self.progress:
                    self.progress.update(
                        1, ', '.join(t.item.name for t in self.running))
            except KeyboardInterrupt:
                print(self.format_task_output(task_item, 'interrupted'),
                      file=sys.stderr)
                raise
            except Exception:
                print(self.format_task_output(task_item, 'ERROR'),
                      file=sys.stderr)
                raise

    class _Worker(threading.Thread):
        """One thread to execute one WorkItem."""
        def __init__(self, item, index, args, kwargs):
            threading.Thread.__init__(self, name=item.name or 'Worker')
            logging.info('_Worker(%s) reqs:%s' % (item.name, item.requirements))
            self.item = item
            self.index = index
            self.args = args
            self.kwargs = kwargs
            self.daemon = True

        def run(self):
            """Runs in its own thread."""
            logging.debug('_Worker.run(%s)' % self.item.name)
            work_queue = self.kwargs['work_queue']
            try:
                self.item.start = datetime.datetime.now()
                print('[%s] Started.' % Elapsed(self.item.start),
                      file=self.item.outbuf)
                self.item.run(*self.args, **self.kwargs)
                self.item.finish = datetime.datetime.now()
                print('[%s] Finished.' % Elapsed(self.item.finish),
                      file=self.item.outbuf)
            except KeyboardInterrupt:
                logging.info('Caught KeyboardInterrupt in thread %s',
                             self.item.name)
                logging.info(str(sys.exc_info()))
                work_queue.exceptions.put((sys.exc_info(), self))
                raise
            except Exception:
                # Catch exception location.
                logging.info('Caught exception in thread %s', self.item.name)
                logging.info(str(sys.exc_info()))
                work_queue.exceptions.put((sys.exc_info(), self))
            finally:
                logging.info('_Worker.run(%s) done', self.item.name)
                work_queue.ready_cond.acquire()
                try:
                    work_queue.ready_cond.notifyAll()
                finally:
                    work_queue.ready_cond.release()


def GetEditor(git_editor=None):
    """Returns the most plausible editor to use.

    In order of preference:
    - GIT_EDITOR environment variable
    - core.editor git configuration variable (if supplied by git-cl)
    - VISUAL environment variable
    - EDITOR environment variable
    - vi (non-Windows) or notepad (Windows)

    In the case of git-cl, this matches git's behaviour, except that it does not
    include dumb terminal detection.
    """
    editor = os.environ.get('GIT_EDITOR') or git_editor
    if not editor:
        editor = os.environ.get('VISUAL')
    if not editor:
        editor = os.environ.get('EDITOR')
    if not editor:
        if sys.platform.startswith('win'):
            editor = 'notepad'
        else:
            editor = 'vi'
    return editor


def RunEditor(content, git, git_editor=None):
    """Opens up the default editor in the system to get the CL description."""
    editor = GetEditor(git_editor=git_editor)
    if not editor:
        return None
    # Make sure CRLF is handled properly by requiring none.
    if '\r' in content:
        print('!! Please remove \\r from your change description !!',
              file=sys.stderr)

    file_handle, filename = tempfile.mkstemp(text=True,
                                             prefix='cl_description.')
    fileobj = os.fdopen(file_handle, 'wb')
    # Still remove \r if present.
    content = re.sub('\r?\n', '\n', content)
    # Some editors complain when the file doesn't end in \n.
    if not content.endswith('\n'):
        content += '\n'

    if 'vim' in editor or editor == 'vi':
        # If the user is using vim and has 'modelines' enabled, this will change
        # the filetype from a generic auto-detected 'conf' to 'gitcommit', which
        # is used to activate proper column wrapping, spell checking, syntax
        # highlighting for git footers, etc.
        #
        # Because of the implementation of GetEditor above, we also check for
        # the exact string 'vi' here, to help users get a sane default when they
        # have vi symlink'd to vim (or something like vim).
        fileobj.write('# vim: ft=gitcommit\n'.encode('utf-8'))

    fileobj.write(content.encode('utf-8'))
    fileobj.close()

    try:
        cmd = '%s %s' % (editor, filename)
        if sys.platform == 'win32' and os.environ.get('TERM') == 'msys':
            # Msysgit requires the usage of 'env' to be present.
            cmd = 'env ' + cmd
        try:
            # shell=True to allow the shell to handle all forms of quotes in
            # $EDITOR.
            subprocess2.check_call(cmd, shell=True)
        except subprocess2.CalledProcessError:
            return None
        return FileRead(filename)
    finally:
        os.remove(filename)


def IsEnvCog():
    """Returns whether the command is running in a Cog environment."""
    return os.getcwd().startswith('/google/cog/cloud')


def UpgradeToHttps(url):
    """Upgrades random urls to https://.

    Do not touch unknown urls like ssh:// or git://.
    Do not touch http:// urls with a port number,
    Fixes invalid GAE url.
    """
    if not url:
        return url
    if not re.match(r'[a-z\-]+\://.*', url):
        # Make sure it is a valid uri. Otherwise, urlparse() will consider it a
        # relative url and will use http:///foo. Note that it defaults to
        # http:// for compatibility with naked url like "localhost:8080".
        url = 'http://%s' % url
    parsed = list(urllib.parse.urlparse(url))
    # Do not automatically upgrade http to https if a port number is provided.
    if parsed[0] == 'http' and not re.match(r'^.+?\:\d+$', parsed[1]):
        parsed[0] = 'https'
    return urllib.parse.urlunparse(parsed)


def ParseCodereviewSettingsContent(content):
    """Process a codereview.settings file properly."""
    lines = (l for l in content.splitlines() if not l.strip().startswith("#"))
    try:
        keyvals = dict([x.strip() for x in l.split(':', 1)] for l in lines if l)
    except ValueError:
        raise Error('Failed to process settings, please fix. Content:\n\n%s' %
                    content)

    def fix_url(key):
        if keyvals.get(key):
            keyvals[key] = UpgradeToHttps(keyvals[key])

    fix_url('CODE_REVIEW_SERVER')
    fix_url('VIEW_VC')
    return keyvals


def NumLocalCpus():
    """Returns the number of processors.

    multiprocessing.cpu_count() is permitted to raise NotImplementedError, and
    is known to do this on some Windows systems and OSX 10.6. If we can't get
    the CPU count, we will fall back to '1'.
    """
    # Surround the entire thing in try/except; no failure here should stop
    # gclient from working.
    try:
        # Use multiprocessing to get CPU count. This may raise
        # NotImplementedError.
        try:
            import multiprocessing
            return multiprocessing.cpu_count()
        except NotImplementedError:  # pylint: disable=bare-except
            # (UNIX) Query 'os.sysconf'.
            # pylint: disable=no-member
            if hasattr(os,
                       'sysconf') and 'SC_NPROCESSORS_ONLN' in os.sysconf_names:
                return int(os.sysconf('SC_NPROCESSORS_ONLN'))

            # (Windows) Query 'NUMBER_OF_PROCESSORS' environment variable.
            if 'NUMBER_OF_PROCESSORS' in os.environ:
                return int(os.environ['NUMBER_OF_PROCESSORS'])
    except Exception as e:
        logging.exception("Exception raised while probing CPU count: %s", e)

    logging.debug('Failed to get CPU count. Defaulting to 1.')
    return 1


def DefaultDeltaBaseCacheLimit():
    """Return a reasonable default for the git config core.deltaBaseCacheLimit.

    The primary constraint is the address space of virtual memory.  The cache
    size limit is per-thread, and 32-bit systems can hit OOM errors if this
    parameter is set too high.
    """
    if platform.architecture()[0].startswith('64'):
        return '2g'

    return '512m'


def DefaultIndexPackConfig(url=''):
    """Return reasonable default values for configuring git-index-pack.

    Experiments suggest that higher values for pack.threads don't improve
    performance."""
    cache_limit = DefaultDeltaBaseCacheLimit()
    result = ['-c', 'core.deltaBaseCacheLimit=%s' % cache_limit]
    if url in THREADED_INDEX_PACK_BLOCKLIST:
        result.extend(['-c', 'pack.threads=1'])
    return result


def FindExecutable(executable):
    """This mimics the "which" utility."""
    path_folders = os.environ.get('PATH').split(os.pathsep)

    for path_folder in path_folders:
        target = os.path.join(path_folder, executable)
        # Just in case we have some ~/blah paths.
        target = os.path.abspath(os.path.expanduser(target))
        if os.path.isfile(target) and os.access(target, os.X_OK):
            return target
        if sys.platform.startswith('win'):
            for suffix in ('.bat', '.cmd', '.exe'):
                alt_target = target + suffix
                if os.path.isfile(alt_target) and os.access(
                        alt_target, os.X_OK):
                    return alt_target
    return None


def freeze(obj):
    """Takes a generic object ``obj``, and returns an immutable version of it.

    Supported types:
        * dict / OrderedDict -> FrozenDict
        * list -> tuple
        * set -> frozenset
        * any object with a working __hash__ implementation (assumes that
            hashable means immutable)

    Will raise TypeError if you pass an object which is not hashable.
    """
    if isinstance(obj, collections.abc.Mapping):
        return FrozenDict((freeze(k), freeze(v)) for k, v in obj.items())

    if isinstance(obj, (list, tuple)):
        return tuple(freeze(i) for i in obj)

    if isinstance(obj, set):
        return frozenset(freeze(i) for i in obj)

    hash(obj)
    return obj


class FrozenDict(collections.abc.Mapping):
    """An immutable OrderedDict.

    Modified From: http://stackoverflow.com/a/2704866
    """
    def __init__(self, *args, **kwargs):
        self._d = collections.OrderedDict(*args, **kwargs)

        # Calculate the hash immediately so that we know all the items are
        # hashable too.
        self._hash = functools.reduce(operator.xor,
                                      (hash(i)
                                       for i in enumerate(self._d.items())), 0)

    def __eq__(self, other):
        if not isinstance(other, collections.abc.Mapping):
            return NotImplemented
        if self is other:
            return True
        if len(self) != len(other):
            return False
        for k, v in self.items():
            if k not in other or other[k] != v:
                return False
        return True

    def __iter__(self):
        return iter(self._d)

    def __len__(self):
        return len(self._d)

    def __getitem__(self, key):
        return self._d[key]

    def __hash__(self):
        return self._hash

    def __repr__(self):
        return 'FrozenDict(%r)' % (self._d.items(), )


def merge_conditions(*conditions):
    """combine multiple conditions into one expression"""
    condition = None
    for current_condition in conditions:
        if not current_condition:
            continue
        if not condition:
            condition = current_condition
            continue
        condition = f'({condition}) and ({current_condition})'
    return condition
