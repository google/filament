# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import multiprocessing.pool
import sys
import threading

from multiprocessing.pool import IMapIterator

from third_party import colorama


def wrapper(func):
    def wrap(self, timeout=None):
        return func(self, timeout=timeout or threading.TIMEOUT_MAX)

    return wrap


# Monkeypatch IMapIterator so that Ctrl-C can kill everything properly.
# Derived from https://gist.github.com/aljungberg/626518
IMapIterator.next = wrapper(IMapIterator.next)
IMapIterator.__next__ = IMapIterator.next
# TODO(iannucci): Monkeypatch all other 'wait' methods too.

import binascii
import collections
import contextlib
import functools
import logging
import os
import random
import re
import setup_color
import shutil
import signal
import tempfile
import textwrap
import time
import typing
from typing import Any
from typing import AnyStr
from typing import Callable
from typing import ContextManager
from typing import Optional
from typing import Tuple

import gclient_utils
import scm
import subprocess2

from io import BytesIO

ROOT = os.path.abspath(os.path.dirname(__file__))
IS_WIN = sys.platform == 'win32'
TEST_MODE = False


def win_find_git() -> str:
    for elem in os.environ.get('PATH', '').split(os.pathsep):
        for candidate in ('git.exe', 'git.bat'):
            path = os.path.join(elem, candidate)
            if os.path.isfile(path):
                # shell=True or invoking git.bat causes Windows to invoke
                # cmd.exe to run git.bat. The extra processes add significant
                # overhead (most visible in the "update" stage of gclient sync)
                # so we want to avoid it whenever possible, by extracting the
                # path to git.exe from git.bat in depot_tools.
                if candidate == 'git.bat':
                    path = _extract_git_path_from_git_bat(path)
                return path
    raise ValueError('Could not find Git on PATH.')


def _extract_git_path_from_git_bat(path: str) -> str:
    """Attempts to extract the path to git.exe from git.bat.

    Args:
        path: the absolute path to git.bat.

    Returns:
        The absolute path to git.exe if extraction succeeded,
        otherwise returns the input path to git.bat.
    """
    with open(path, 'r') as f:
        git_bat = f.readlines()
        if git_bat[-1].endswith('" %*\n'):
            if git_bat[-1].startswith('"%~dp0'):
                # Handle relative path.
                new_path = os.path.join(os.path.dirname(path),
                                        git_bat[-1][6:-5])
            elif git_bat[-1].startswith('"'):
                # Handle absolute path.
                new_path = git_bat[-1][1:-5]

            if new_path.endswith('.exe'):
                return new_path
    return path


GIT_EXE = 'git' if not IS_WIN else win_find_git()

# The recommended minimum version of Git, as (<major>, <minor>, <patch>).
GIT_MIN_VERSION = (2, 26, 0)

GIT_BLAME_IGNORE_REV_FILE = '.git-blame-ignore-revs'

FREEZE = 'FREEZE'
FREEZE_SECTIONS = {'indexed': 'soft', 'unindexed': 'mixed'}
FREEZE_MATCHER = re.compile(r'%s.(%s)' % (FREEZE, '|'.join(FREEZE_SECTIONS)))

# NOTE: This list is DEPRECATED in favor of the Infra Git wrapper:
# https://chromium.googlesource.com/infra/infra/+/HEAD/go/src/infra/tools/git
#
# New entries should be added to the Git wrapper, NOT to this list. "git_retry"
# is, similarly, being deprecated in favor of the Git wrapper.
#
# ---
#
# Retry a git operation if git returns a error response with any of these
# messages. It's all observed 'bad' GoB responses so far.
#
# This list is inspired/derived from the one in ChromiumOS's Chromite:
# <CHROMITE>/lib/git.py::GIT_TRANSIENT_ERRORS
#
# It was last imported from '7add3ac29564d98ac35ce426bc295e743e7c0c02'.
GIT_TRANSIENT_ERRORS = (
    # crbug.com/285832
    r'!.*\[remote rejected\].*\(error in hook\)',

    # crbug.com/289932
    r'!.*\[remote rejected\].*\(failed to lock\)',

    # crbug.com/307156
    r'!.*\[remote rejected\].*\(error in Gerrit backend\)',

    # crbug.com/285832
    r'remote error: Internal Server Error',

    # crbug.com/294449
    r'fatal: Couldn\'t find remote ref ',

    # crbug.com/220543
    r'git fetch_pack: expected ACK/NAK, got',

    # crbug.com/189455
    r'protocol error: bad pack header',

    # crbug.com/202807
    r'The remote end hung up unexpectedly',

    # crbug.com/298189
    r'TLS packet with unexpected length was received',

    # crbug.com/187444
    r'RPC failed; result=\d+, HTTP code = \d+',

    # crbug.com/388876
    r'Connection timed out',

    # crbug.com/430343
    # TODO(dnj): Resync with Chromite.
    r'The requested URL returned error: 5\d+',
    r'Connection reset by peer',
    r'Unable to look up',
    r'Couldn\'t resolve host',
)

GIT_TRANSIENT_ERRORS_RE = re.compile('|'.join(GIT_TRANSIENT_ERRORS),
                                     re.IGNORECASE)

# git's for-each-ref command first supported the upstream:track token in its
# format string in version 1.9.0, but some usages were broken until 2.3.0.
# See git commit b6160d95 for more information.
MIN_UPSTREAM_TRACK_GIT_VERSION = (2, 3)


class BadCommitRefException(Exception):
    def __init__(self, refs):
        msg = ('one of %s does not seem to be a valid commitref.' % str(refs))
        super(BadCommitRefException, self).__init__(msg)


class _MemoizeWrapper(object):

    def __init__(self, f: Callable[[Any], Any], *, threadsafe: bool):
        self._f: Callable[[Any], Any] = f
        self._cache: dict[Any, Any] = {}
        self._lock: ContextManager = contextlib.nullcontext()
        if threadsafe:
            self._lock = threading.Lock()

    def __call__(self, arg: Any) -> Any:
        ret = self.get(arg)
        if ret is None:
            ret = self._f(arg)
            if ret is not None:
                self.set(arg, ret)
        return ret

    def get(self, key: Any, default: Any = None) -> Any:
        with self._lock:
            return self._cache.get(key, default)

    def set(self, key: Any, value: Any) -> None:
        with self._lock:
            self._cache[key] = value

    def clear(self) -> None:
        with self._lock:
            self._cache.clear()

    def update(self, other: dict[Any, Any]) -> None:
        with self._lock:
            self._cache.update(other)


def memoize_one(*, threadsafe: bool):
    """Memoizes a single-argument pure function.

    Values of None are not cached.

    Kwargs:
        threadsafe (bool) - REQUIRED. Specifies whether to use locking around
            cache manipulation functions. This is a kwarg so that users of
            memoize_one are forced to explicitly and verbosely pick True or
            False.

    Adds three methods to the decorated function:
        * get(key, default=None) - Gets the value for this key from the cache.
        * set(key, value) - Sets the value for this key from the cache.
        * clear() - Drops the entire contents of the cache.  Useful for
            unittests.
        * update(other) - Updates the contents of the cache from another dict.
    """
    def decorator(f):
        # Instantiate the lock in decorator, in case users of memoize_one do:
        #
        # memoizer = memoize_one(threadsafe=True)
        #
        # @memoizer
        # def fn1(val): ...
        #
        # @memoizer
        # def fn2(val): ...
        wrapped = _MemoizeWrapper(f, threadsafe=threadsafe)
        return functools.wraps(f)(wrapped)

    return decorator


def _ScopedPool_initer(orig, orig_args):  # pragma: no cover
    """Initializer method for ScopedPool's subprocesses.

    This helps ScopedPool handle Ctrl-C's correctly.
    """
    signal.signal(signal.SIGINT, signal.SIG_IGN)
    if orig:
        orig(*orig_args)


@contextlib.contextmanager
def ScopedPool(*args, **kwargs):
    """Context Manager which returns a multiprocessing.pool instance which
    correctly deals with thrown exceptions.

    *args - Arguments to multiprocessing.pool

    Kwargs:
        kind ('threads', 'procs') - The type of underlying coprocess to use.
        **etc - Arguments to multiprocessing.pool
    """
    if kwargs.pop('kind', None) == 'threads':
        pool = multiprocessing.pool.ThreadPool(*args, **kwargs)
    else:
        orig, orig_args = kwargs.get('initializer'), kwargs.get('initargs', ())
        kwargs['initializer'] = _ScopedPool_initer
        kwargs['initargs'] = orig, orig_args
        pool = multiprocessing.pool.Pool(*args, **kwargs)

    try:
        yield pool
        pool.close()
    except:
        pool.terminate()
        raise
    finally:
        pool.join()


class ProgressPrinter(object):
    """Threaded single-stat status message printer."""
    def __init__(self, fmt, enabled=None, fout=sys.stderr, period=0.5):
        """Create a ProgressPrinter.

        Use it as a context manager which produces a simple 'increment' method:

        with ProgressPrinter('(%%(count)d/%d)' % 1000) as inc:
            for i in xrange(1000):
            # do stuff
            if i % 10 == 0:
                inc(10)

        Args:
        fmt - String format with a single '%(count)d' where the counter value
            should go.
        enabled (bool) - If this is None, will default to True if
            logging.getLogger() is set to INFO or more verbose.
        fout (file-like) - The stream to print status messages to.
        period (float) - The time in seconds for the printer thread to wait
            between printing.
        """
        self.fmt = fmt
        if enabled is None:  # pragma: no cover
            self.enabled = logging.getLogger().isEnabledFor(logging.INFO)
        else:
            self.enabled = enabled

        self._count = 0
        self._dead = False
        self._dead_cond = threading.Condition()
        self._stream = fout
        self._thread = threading.Thread(target=self._run)
        self._period = period

    def _emit(self, s):
        if self.enabled:
            self._stream.write('\r' + s)
            self._stream.flush()

    def _run(self):
        with self._dead_cond:
            while not self._dead:
                self._emit(self.fmt % {'count': self._count})
                self._dead_cond.wait(self._period)
                self._emit((self.fmt + '\n') % {'count': self._count})

    def inc(self, amount=1):
        self._count += amount

    def __enter__(self):
        self._thread.start()
        return self.inc

    def __exit__(self, _exc_type, _exc_value, _traceback):
        self._dead = True
        with self._dead_cond:
            self._dead_cond.notifyAll()
        self._thread.join()
        del self._thread


def once(function):
    """@Decorates |function| so that it only performs its action once, no matter
    how many times the decorated |function| is called."""
    has_run = [False]

    def _wrapper(*args, **kwargs):
        if not has_run[0]:
            has_run[0] = True
            function(*args, **kwargs)

    return _wrapper


def unicode_repr(s):
    result = repr(s)
    return result[1:] if result.startswith('u') else result


## Git functions


def die(message, *args):
    print(textwrap.dedent(message % args), file=sys.stderr)
    sys.exit(1)


def blame(filename, revision=None, porcelain=False, abbrev=None, *_args):
    command = ['blame']
    if porcelain:
        command.append('-p')
    if revision is not None:
        command.append(revision)
    if abbrev is not None:
        command.append('--abbrev=%d' % abbrev)
    command.extend(['--', filename])
    return run(*command)


def branch_config(branch: str,
                  option: str,
                  default: Optional[str] = None) -> Optional[str]:
    return get_config('branch.%s.%s' % (branch, option), default=default)


def branch_config_map(option):
    """Return {branch: <|option| value>} for all branches."""
    try:
        reg = re.compile(r'^branch\.(.*)\.%s$' % option)
        return {
            m.group(1): v
            for k, v in get_config_regexp(reg.pattern)
            if (m := reg.match(k)) is not None
        }
    except subprocess2.CalledProcessError:
        return {}


def branches(use_limit=True, *args):
    NO_BRANCH = ('* (no branch', '* (detached', '* (HEAD detached')

    key = 'depot-tools.branch-limit'
    limit = get_config_int(key, 20)

    output = run('branch', *args)
    assert isinstance(output, str)
    raw_branches = output.splitlines()

    num = len(raw_branches)

    if use_limit and num > limit:
        die(
            """\
      Your git repo has too many branches (%d/%d) for this tool to work well.

      You may adjust this limit by running:
      git config %s <new_limit>

      You may also try cleaning up your old branches by running:
      git cl archive
      """, num, limit, key)

    for line in raw_branches:
        if line.startswith(NO_BRANCH):
            continue
        yield line.split()[-1]


def get_config(option: str, default: Optional[str] = None) -> Optional[str]:
    return scm.GIT.GetConfig(os.getcwd(), option, default)


def get_config_int(option: str, default: int = 0) -> int:
    assert isinstance(default, int)
    val = get_config(option)
    if val is None:
        return default
    try:
        return int(val)
    except ValueError:
        return default


def get_config_list(option):
    return scm.GIT.GetConfigList(os.getcwd(), option)


def get_config_regexp(pattern):
    return scm.GIT.YieldConfigRegexp(os.getcwd(), pattern)


def is_fsmonitor_enabled():
    """Returns true if core.fsmonitor is enabled in git config."""
    fsmonitor = get_config('core.fsmonitor', 'False')
    assert isinstance(fsmonitor, str)
    return fsmonitor.strip().lower() == 'true'


def warn_submodule():
    """Print warnings for submodules."""
    # TODO(crbug.com/1475405): Warn users if the project uses submodules and
    # they have fsmonitor enabled.
    if sys.platform.startswith('darwin') and is_fsmonitor_enabled():
        version_string = run('--version')
        assert isinstance(version_string, str)
        if version_string.endswith('goog'):
            return
        version_tuple = _extract_git_tuple(version_string)
        if version_tuple >= (2, 43):
            return

        print(colorama.Fore.RED)
        print('WARNING: You have fsmonitor enabled. There is a major issue '
              'resulting in git diff-index returning wrong results. Please '
              'either disable it by running:')
        print('    git config core.fsmonitor false')
        print('or upgrade git to version >= 2.43.')
        print('See https://crbug.com/1475405 for details.')
        print(colorama.Style.RESET_ALL)


def current_branch():
    try:
        return run('rev-parse', '--abbrev-ref', 'HEAD')
    except subprocess2.CalledProcessError:
        return None


def del_branch_config(branch, option, scope: scm.GitConfigScope = 'local'):
    del_config('branch.%s.%s' % (branch, option), scope=scope)


def del_config(option, scope: scm.GitConfigScope = 'local'):
    try:
        scm.GIT.SetConfig(os.getcwd(), option, scope=scope)
    except subprocess2.CalledProcessError:
        pass


def diff(oldrev, newrev, *args):
    return run('diff', oldrev, newrev, *args)


def freeze():
    took_action = False
    key = 'depot-tools.freeze-size-limit'
    MB = 2**20
    limit_mb = get_config_int(key, 100)
    untracked_bytes = 0

    root_path = repo_root()

    # unindexed tracks all the files which are unindexed but we want to add to
    # the `FREEZE.unindexed` commit.
    unindexed = []

    # will be set to true if there are any indexed files to commit.
    have_indexed_files = False

    for f, s in status(ignore_submodules='all'):
        if is_unmerged(s):
            die("Cannot freeze unmerged changes!")
        if s.lstat not in ' ?':
            # This covers all changes to indexed files.
            # lstat = ' ' means that the file is tracked and modified, but
            # wasn't added yet. lstat = '?' means that the file is untracked.
            have_indexed_files = True

            # If the file has both indexed and unindexed changes.
            # rstat shows the status of the working tree. If the file also has
            # changes in the working tree, it should be tracked both in indexed
            # and unindexed changes.
            if s.rstat != ' ':
                unindexed.append(f.encode('utf-8'))
        else:
            unindexed.append(f.encode('utf-8'))

        if s.lstat == '?' and limit_mb > 0:
            untracked_bytes += os.lstat(os.path.join(root_path, f)).st_size

    if limit_mb > 0 and untracked_bytes > limit_mb * MB:
        die(
            """\
      You appear to have too much untracked+unignored data in your git
      checkout: %.1f / %d MB.

      Run `git status` to see what it is.

      In addition to making many git commands slower, this will prevent
      depot_tools from freezing your in-progress changes.

      You should add untracked data that you want to ignore to your repo's
        .git/info/exclude
      file. See `git help ignore` for the format of this file.

      If this data is intended as part of your commit, you may adjust the
      freeze limit by running:
        git config %s <new_limit>
      Where <new_limit> is an integer threshold in megabytes.""",
            untracked_bytes / (MB * 1.0), limit_mb, key)

    if have_indexed_files:
        try:
            run('commit', '--no-verify', '-m', f'{FREEZE}.indexed')
            took_action = True
        except subprocess2.CalledProcessError:
            pass

    add_errors = False
    if unindexed:
        try:
            run('add',
                '--pathspec-from-file',
                '-',
                '--ignore-errors',
                indata=b'\n'.join(unindexed),
                cwd=root_path)
        except subprocess2.CalledProcessError:
            add_errors = True

        try:
            run('commit', '--no-verify', '-m', f'{FREEZE}.unindexed')
            took_action = True
        except subprocess2.CalledProcessError:
            pass

    ret = []
    if add_errors:
        ret.append('Failed to index some unindexed files.')
    if not took_action:
        ret.append('Nothing to freeze.')
    return ' '.join(ret) or None


def get_branch_tree(use_limit=False):
    """Get the dictionary of {branch: parent}, compatible with topo_iter.

    Returns a tuple of (skipped, <branch_tree dict>) where skipped is a set of
    branches without upstream branches defined.
    """
    skipped = set()
    branch_tree = {}

    for branch in branches(use_limit=use_limit):
        parent = upstream(branch)
        if not parent:
            skipped.add(branch)
            continue
        branch_tree[branch] = parent

    return skipped, branch_tree


def get_or_create_merge_base(branch, parent=None) -> Optional[str]:
    """Finds the configured merge base for branch.

    If parent is supplied, it's used instead of calling upstream(branch).
    """
    base: Optional[str] = branch_config(branch, 'base')
    base_upstream = branch_config(branch, 'base-upstream')
    parent = parent or upstream(branch)
    if parent is None or branch is None:
        return None
    actual_merge_base = run('merge-base', parent, branch)
    assert isinstance(actual_merge_base, str)

    if base_upstream != parent:
        base = None
        base_upstream = None

    def is_ancestor(a, b):
        return run_with_retcode('merge-base', '--is-ancestor', a, b) == 0

    if base and base != actual_merge_base:
        if not is_ancestor(base, branch):
            logging.debug('Found WRONG pre-set merge-base for %s: %s', branch,
                          base)
            base = None
        elif is_ancestor(base, actual_merge_base):
            logging.debug('Found OLD pre-set merge-base for %s: %s', branch,
                          base)
            base = None
        else:
            logging.debug('Found pre-set merge-base for %s: %s', branch, base)

    if not base:
        base = actual_merge_base
        manual_merge_base(branch, base, parent)

    assert isinstance(base, str)
    return base


def hash_multi(*reflike):
    return run('rev-parse', *reflike).splitlines()


def hash_one(reflike, short=False):
    args = ['rev-parse', reflike]
    if short:
        args.insert(1, '--short')
    return run(*args)


def in_rebase():
    git_dir = run('rev-parse', '--git-dir')
    assert isinstance(git_dir, str)
    return (os.path.exists(os.path.join(git_dir, 'rebase-merge'))
            or os.path.exists(os.path.join(git_dir, 'rebase-apply')))


def intern_f(f, kind='blob'):
    """Interns a file object into the git object store.

    Args:
        f (file-like object) - The file-like object to intern
        kind (git object type) - One of 'blob', 'commit', 'tree', 'tag'.

    Returns the git hash of the interned object (hex encoded).
    """
    ret = run('hash-object', '-t', kind, '-w', '--stdin', stdin=f)
    f.close()
    return ret


def is_dormant(branch):
    # TODO(iannucci): Do an oldness check?
    return branch_config(branch, 'dormant', 'false') != 'false'


def is_unmerged(stat_value):
    return ('U' in (stat_value.lstat, stat_value.rstat)
            or ((stat_value.lstat == stat_value.rstat)
                and stat_value.lstat in 'AD'))


def manual_merge_base(branch, base, parent):
    set_branch_config(branch, 'base', base)
    set_branch_config(branch, 'base-upstream', parent)


def mktree(treedict):
    """Makes a git tree object and returns its hash.

    See |tree()| for the values of mode, type, and ref.

    Args:
        treedict - { name: (mode, type, ref) }
    """
    with tempfile.TemporaryFile() as f:
        for name, (mode, typ, ref) in treedict.items():
            f.write(('%s %s %s\t%s\0' % (mode, typ, ref, name)).encode('utf-8'))
        f.seek(0)
        return run('mktree', '-z', stdin=f)


def parse_commitrefs(*commitrefs):
    """Returns binary encoded commit hashes for one or more commitrefs.

    A commitref is anything which can resolve to a commit. Popular examples:
        * 'HEAD'
        * 'origin/main'
        * 'cool_branch~2'
    """
    hashes = []
    try:
        hashes = hash_multi(*commitrefs)
        return [binascii.unhexlify(h) for h in hashes]
    except subprocess2.CalledProcessError:
        raise BadCommitRefException(commitrefs)
    except binascii.Error as e:
        raise binascii.Error(f'{e}. Invalid hashes are {hashes}')


RebaseRet = collections.namedtuple('RebaseRet', 'success stdout stderr')


def rebase(parent, start, branch, abort=False, allow_gc=False):
    """Rebases |start|..|branch| onto the branch |parent|.

    Sets 'gc.auto=0' for the duration of this call to prevent the rebase from
    running a potentially slow garbage collection cycle.

    Args:
        parent - The new parent ref for the rebased commits.
        start  - The commit to start from
        branch - The branch to rebase
        abort  - If True, will call git-rebase --abort in the event that the
            rebase doesn't complete successfully.
        allow_gc - If True, sets "-c gc.auto=1" on the rebase call, rather than
            "-c gc.auto=0". Usually if you're doing a series of rebases,
            you'll only want to run a single gc pass at the end of all the
            rebase activity.

    Returns a namedtuple with fields:
        success - a boolean indicating that the rebase command completed
            successfully.
        message - if the rebase failed, this contains the stdout of the failed
            rebase.
    """
    try:
        args = [
            '-c',
            'gc.auto={}'.format('1' if allow_gc else '0'),
            'rebase',
        ]
        if TEST_MODE:
            args.append('--committer-date-is-author-date')
        args += [
            '--onto',
            parent,
            start,
            branch,
        ]
        run(*args)
        return RebaseRet(True, '', '')
    except subprocess2.CalledProcessError as cpe:
        if abort:
            run_with_retcode('rebase', '--abort')  # ignore failure
        return RebaseRet(False, cpe.stdout.decode('utf-8', 'replace'),
                         cpe.stderr.decode('utf-8', 'replace'))


def remove_merge_base(branch):
    del_branch_config(branch, 'base')
    del_branch_config(branch, 'base-upstream')


def repo_root():
    """Returns the absolute path to the repository root."""
    return run('rev-parse', '--show-toplevel')


def upstream_default() -> str:
    """Returns the default branch name of the origin repository."""
    try:
        ret = run('rev-parse', '--abbrev-ref', 'origin/HEAD')
        # Detect if the repository migrated to main branch
        if ret == 'origin/master':
            try:
                ret = run('rev-parse', '--abbrev-ref', 'origin/main')
                run('remote', 'set-head', '-a', 'origin')
                ret = run('rev-parse', '--abbrev-ref', 'origin/HEAD')
            except subprocess2.CalledProcessError:
                pass
        assert isinstance(ret, str)
        return ret
    except subprocess2.CalledProcessError:
        return 'origin/main'


def root():
    return get_config('depot-tools.upstream', upstream_default())


@contextlib.contextmanager
def less():  # pragma: no cover
    """Runs 'less' as context manager yielding its stdin as a PIPE.

    Automatically checks if sys.stdout is a non-TTY stream. If so, it avoids
    running less and just yields sys.stdout.

    The returned PIPE is opened on binary mode.
    """
    if not setup_color.IS_TTY:
        # On Python 3, sys.stdout doesn't accept bytes, and sys.stdout.buffer
        # must be used.
        yield getattr(sys.stdout, 'buffer', sys.stdout)
        return

    # Run with the same options that git uses (see setup_pager in git repo).
    # -F: Automatically quit if the output is less than one screen.
    # -R: Don't escape ANSI color codes.
    # -X: Don't clear the screen before starting.
    cmd = ('less', '-FRX')
    proc = subprocess2.Popen(cmd, stdin=subprocess2.PIPE)
    try:
        yield proc.stdin
    finally:
        assert proc.stdin is not None
        try:
            proc.stdin.close()
        except BrokenPipeError:
            # BrokenPipeError is raised if proc has already completed,
            pass
        proc.wait()


def run(*cmd, **kwargs) -> str | bytes:
    """The same as run_with_stderr, except it only returns stdout."""
    return run_with_stderr(*cmd, **kwargs)[0]


def run_with_retcode(*cmd, **kwargs):
    """Run a command but only return the status code."""
    try:
        run(*cmd, **kwargs)
        return 0
    except subprocess2.CalledProcessError as cpe:
        return cpe.returncode


def run_stream(*cmd, **kwargs) -> typing.IO[AnyStr]:
    """Runs a git command. Returns stdout as a PIPE (file-like object).

    stderr is dropped to avoid races if the process outputs to both stdout and
    stderr.
    """
    kwargs.setdefault('stderr', subprocess2.DEVNULL)
    kwargs.setdefault('stdout', subprocess2.PIPE)
    kwargs.setdefault('shell', False)
    cmd = (GIT_EXE, '-c', 'color.ui=never') + cmd
    proc = subprocess2.Popen(cmd, **kwargs)
    assert proc.stdout is not None
    return proc.stdout


@contextlib.contextmanager
def run_stream_with_retcode(*cmd, **kwargs):
    """Runs a git command as context manager yielding stdout as a PIPE.

    stderr is dropped to avoid races if the process outputs to both stdout and
    stderr.

    Raises subprocess2.CalledProcessError on nonzero return code.
    """
    kwargs.setdefault('stderr', subprocess2.DEVNULL)
    kwargs.setdefault('stdout', subprocess2.PIPE)
    kwargs.setdefault('shell', False)
    cmd = (GIT_EXE, '-c', 'color.ui=never') + cmd
    proc = subprocess2.Popen(cmd, **kwargs)
    try:
        yield proc.stdout
    finally:
        retcode = proc.wait()
        if retcode != 0:
            raise subprocess2.CalledProcessError(retcode, cmd, os.getcwd(), b'',
                                                 b'')


def run_with_stderr(*cmd, **kwargs) -> Tuple[str, str] | Tuple[bytes, bytes]:
    """Runs a git command.

    Returns (stdout, stderr) as a pair of strings.
    If the command is `config` and the execution fails due to a lock failure,
    retry the execution at most 5 times with 0.2 interval.

    kwargs
        autostrip (bool) - Strip the output. Defaults to True.
        indata (str) - Specifies stdin data for the process.
        retry_lock (bool) - If true and the command is `config`,
            retry on lock failures. Defaults to True.
    """
    retry_cnt = 0
    if kwargs.pop('retry_lock', True) and len(cmd) > 0 and cmd[0] == 'config':
        retry_cnt = 5

    while True:
        try:
            return _run_with_stderr(*cmd, **kwargs)
        except subprocess2.CalledProcessError as ex:
            lock_err = 'could not lock config file .git/config: File exists'
            if retry_cnt > 0 and lock_err in str(ex):
                logging.error(ex)
                jitter = random.uniform(0, 0.2)
                time.sleep(0.1 + jitter)
                retry_cnt -= 1
                continue

            raise ex


def _run_with_stderr(*cmd, **kwargs) -> Tuple[str, str] | Tuple[bytes, bytes]:
    """Runs a git command.

    Returns (stdout, stderr) as a pair of bytes or strings.

    kwargs
        autostrip (bool) - Strip the output. Defaults to True.
        decode (bool) - Whether to return strings. Defaults to True.
        indata (str) - Specifies stdin data for the process.
    """
    kwargs.setdefault('stdin', subprocess2.PIPE)
    kwargs.setdefault('stdout', subprocess2.PIPE)
    kwargs.setdefault('stderr', subprocess2.PIPE)
    kwargs.setdefault('shell', False)
    autostrip = kwargs.pop('autostrip', True)
    indata = kwargs.pop('indata', None)
    decode = kwargs.pop('decode', True)
    accepted_retcodes = kwargs.pop('accepted_retcodes', [0])

    cmd = (GIT_EXE, '-c', 'color.ui=never') + cmd
    proc = subprocess2.Popen(cmd, **kwargs)
    stdout, stderr = proc.communicate(indata)
    retcode = proc.wait()
    if retcode not in accepted_retcodes:
        raise subprocess2.CalledProcessError(retcode, cmd, os.getcwd(), stdout,
                                             stderr)

    if autostrip:
        stdout = (stdout or b'').strip()
        stderr = (stderr or b'').strip()

    if decode:
        return stdout.decode('utf-8',
                             'replace'), stderr.decode('utf-8', 'replace')

    return stdout, stderr


def set_branch_config(branch,
                      option,
                      value,
                      scope: scm.GitConfigScope = 'local'):
    set_config('branch.%s.%s' % (branch, option), value, scope=scope)


def set_config(option, value, scope: scm.GitConfigScope = 'local'):
    scm.GIT.SetConfig(os.getcwd(), option, value, scope=scope)


def get_dirty_files():
    # Make sure index is up-to-date before running diff-index.
    run_with_retcode('update-index', '--refresh', '-q')
    return run('diff-index', '--ignore-submodules', '--name-status', 'HEAD',
               '--')


def is_dirty_git_tree(cmd):
    w = lambda s: sys.stderr.write(s + "\n")

    dirty = get_dirty_files()
    if dirty:
        w('Cannot %s with a dirty tree. Commit%s or stash your changes first.' %
          (cmd, '' if cmd == 'upload' else ', freeze'))
        w('Uncommitted files: (git diff-index --name-status HEAD)')
        w(dirty[:4096])
        if len(dirty) > 4096:  # pragma: no cover
            w('... (run "git diff-index --name-status HEAD" to see full '
              'output).')
        return True
    return False


def status(ignore_submodules=None):
    """Returns a parsed version of git-status.

    Args:
        ignore_submodules (str|None): "all", "none", or None.
            None is equivalent to "none".

    Returns a generator of (current_name, (lstat, rstat, src)) pairs where:
        * current_name is the name of the file
        * lstat is the left status code letter from git-status
        * rstat is the right status code letter from git-status
        * src is the current name of the file, or the original name of the file
        if lstat == 'R'
    """

    ignore_submodules = ignore_submodules or 'none'
    assert ignore_submodules in (
        'all',
        'none'), f'ignore_submodules value {ignore_submodules} is invalid'

    stat_entry = collections.namedtuple('stat_entry', 'lstat rstat src')

    def tokenizer(stream):
        acc = BytesIO()
        c = None
        while c != b'':
            c = stream.read(1)
            if c in (None, b'', b'\0'):
                if len(acc.getvalue()) > 0:
                    yield acc.getvalue()
                    acc = BytesIO()
            else:
                acc.write(c)

    def parser(tokens):
        while True:
            try:
                status_dest = next(tokens).decode('utf-8')
            except StopIteration:
                return
            stat, dest = status_dest[:2], status_dest[3:]
            lstat, rstat = stat
            if lstat == 'R':
                src = next(tokens).decode('utf-8')
            else:
                src = dest
            yield (dest, stat_entry(lstat, rstat, src))

    return parser(
        tokenizer(
            run_stream('status',
                       '-z',
                       f'--ignore-submodules={ignore_submodules}',
                       bufsize=-1)))


def squash_current_branch(header=None, merge_base=None):
    header = header or 'git squash commit for %s.' % current_branch()
    merge_base = merge_base or get_or_create_merge_base(current_branch())
    log_msg = header + '\n'
    if log_msg:
        log_msg += '\n'
    output = run('log', '--reverse', '--format=%H%n%B', '%s..HEAD' % merge_base)
    assert isinstance(output, str)
    log_msg += output
    run('reset', '--soft', merge_base)

    if not get_dirty_files():
        # Sometimes the squash can result in the same tree, meaning that there
        # is nothing to commit at this point.
        print('Nothing to commit; squashed branch is empty')
        return False

    # git reset --soft will stage all changes so we can just commit those.
    # Note: Just before reset --soft is called, we may have git submodules
    # checked to an old commit (not latest state). We don't want to include
    # those in our commit.
    run('commit',
        '--no-verify',
        '-F',
        '-',
        indata=log_msg.encode('utf-8'))
    return True


def tags(*args):
    return run('tag', *args).splitlines()


def thaw():
    took_action = False
    with run_stream('rev-list', 'HEAD', '--') as stream:
        for sha in stream:
            sha = sha.strip().decode('utf-8')
            msg = run('show', '--format=%f%b', '-s', 'HEAD', '--')
            assert isinstance(msg, str)
            match = FREEZE_MATCHER.match(msg)
            if not match:
                if not took_action:
                    return 'Nothing to thaw.'
                break

            run('reset', '--' + FREEZE_SECTIONS[match.group(1)], sha)
            took_action = True


def topo_iter(branch_tree, top_down=True):
    """Generates (branch, parent) in topographical order for a branch tree.

    Given a tree:

                A1
            B1      B2
        C1  C2    C3
                    D1

    branch_tree would look like: {
        'D1': 'C3',
        'C3': 'B2',
        'B2': 'A1',
        'C1': 'B1',
        'C2': 'B1',
        'B1': 'A1',
    }

    It is OK to have multiple 'root' nodes in your graph.

    if top_down is True, items are yielded from A->D. Otherwise they're yielded
    from D->A. Within a layer the branches will be yielded in sorted order.
    """
    branch_tree = branch_tree.copy()

    # TODO(iannucci): There is probably a more efficient way to do these.
    if top_down:
        while branch_tree:
            this_pass = [(b, p) for b, p in branch_tree.items()
                         if p not in branch_tree]
            assert this_pass, "Branch tree has cycles: %r" % branch_tree
            for branch, parent in sorted(this_pass):
                yield branch, parent
                del branch_tree[branch]
    else:
        parent_to_branches = collections.defaultdict(set)
        for branch, parent in branch_tree.items():
            parent_to_branches[parent].add(branch)

        while branch_tree:
            this_pass = [(b, p) for b, p in branch_tree.items()
                         if not parent_to_branches[b]]
            assert this_pass, "Branch tree has cycles: %r" % branch_tree
            for branch, parent in sorted(this_pass):
                yield branch, parent
                parent_to_branches[parent].discard(branch)
                del branch_tree[branch]


def tree(treeref, recurse=False):
    """Returns a dict representation of a git tree object.

    Args:
        treeref (str) - a git ref which resolves to a tree (commits count as
            trees).
        recurse (bool) - include all of the tree's descendants too. File names
            will take the form of 'some/path/to/file'.

    Return format:
        { 'file_name': (mode, type, ref) }

        mode is an integer where:
        * 0040000 - Directory
        * 0100644 - Regular non-executable file
        * 0100664 - Regular non-executable group-writeable file
        * 0100755 - Regular executable file
        * 0120000 - Symbolic link
        * 0160000 - Gitlink

        type is a string where it's one of 'blob', 'commit', 'tree', 'tag'.

        ref is the hex encoded hash of the entry.
    """
    ret = {}
    opts = ['ls-tree', '--full-tree']
    if recurse:
        opts.append('-r')
    opts.append(treeref)
    try:
        for line in run(*opts).splitlines():
            mode, typ, ref, name = line.split(None, 3)
            ret[name] = (mode, typ, ref)
    except subprocess2.CalledProcessError:
        return None
    return ret


def get_remote_url(remote='origin'):
    return scm.GIT.GetConfig(os.getcwd(), 'remote.%s.url' % remote)


def upstream(branch):
    try:
        return run('rev-parse', '--abbrev-ref', '--symbolic-full-name',
                   branch + '@{upstream}')
    except subprocess2.CalledProcessError:
        return None


@functools.lru_cache
def check_git_version(
        min_version: Tuple[int] = GIT_MIN_VERSION) -> Optional[str]:
    """Checks whether git is installed, and its version meets the recommended
    minimum version, which defaults to GIT_MIN_VERSION if not specified.

    Returns:
        - the remediation action to take.
    """
    if gclient_utils.IsEnvCog():
        # No remediation action required in a non-git environment.
        return None

    min_tag = '.'.join(str(x) for x in min_version)
    if shutil.which(GIT_EXE) is None:
        # git command was not found.
        return ('git command not found.\n'
                f'Please install version >={min_tag} of git.\n'
                'See instructions at\n'
                'https://git-scm.com/book/en/v2/Getting-Started-Installing-Git')

    if meets_git_version(min_version):
        # git version is sufficient; no remediation action necessary.
        return None

    # git is installed but older than the recommended version.
    tag = '.'.join(str(x) for x in get_git_version()) or 'unknown'
    return ('git update is recommended.\n'
            f'Installed git version is {tag};\n'
            f'depot_tools recommends version {min_tag} or later.')


def meets_git_version(min_version: Tuple[int]) -> bool:
    """Returns whether the current git version meets the minimum specified."""
    return get_git_version() >= min_version


@functools.lru_cache(maxsize=1)
def get_git_version():
    """Returns a tuple that contains the numeric components of the current git
    version."""
    version_string = run('--version')
    return _extract_git_tuple(version_string)


def _extract_git_tuple(version_string):
    version_match = re.search(r'(\d+.)+(\d+)', version_string)
    if version_match:
        version = version_match.group()
        return tuple(int(x) for x in version.split('.'))
    return tuple()


def get_num_commits(branch):
    base = get_or_create_merge_base(branch)
    if base:
        commits_list = run('rev-list', '--count', branch, '^%s' % base, '--')
        return int(commits_list) or None
    return None


def get_branches_info(include_tracking_status):
    format_string = (
        '--format=%(refname:short):%(objectname:short):%(upstream:short):')

    # This is not covered by the depot_tools CQ which only has git version 1.8.
    if (include_tracking_status and get_git_version() >=
            MIN_UPSTREAM_TRACK_GIT_VERSION):  # pragma: no cover
        format_string += '%(upstream:track)'

    info_map = {}
    data = run('for-each-ref', format_string, 'refs/heads')
    assert isinstance(data, str)
    BranchesInfo = collections.namedtuple('BranchesInfo',
                                          'hash upstream commits behind')
    for line in data.splitlines():
        (branch, branch_hash, upstream_branch,
         tracking_status) = line.split(':')

        commits = None
        if include_tracking_status:
            commits = get_num_commits(branch)

        behind_match = re.search(r'behind (\d+)', tracking_status)
        behind = int(behind_match.group(1)) if behind_match else None

        info_map[branch] = BranchesInfo(hash=branch_hash,
                                        upstream=upstream_branch,
                                        commits=commits,
                                        behind=behind)

    # Set None for upstreams which are not branches (e.g empty upstream, remotes
    # and deleted upstream branches).
    missing_upstreams = {}
    for info in info_map.values():
        if (info.upstream not in info_map
                and info.upstream not in missing_upstreams):
            missing_upstreams[info.upstream] = None

    result = info_map.copy()
    result.update(missing_upstreams)
    return result


def make_workdir_common(repository,
                        new_workdir,
                        files_to_symlink,
                        files_to_copy,
                        symlink=None):
    if not symlink:
        symlink = os.symlink
    os.makedirs(new_workdir)
    for entry in files_to_symlink:
        clone_file(repository, new_workdir, entry, symlink)
    for entry in files_to_copy:
        clone_file(repository, new_workdir, entry, shutil.copy)


def make_workdir(repository, new_workdir):
    GIT_DIRECTORY_WHITELIST = [
        'config',
        'info',
        'hooks',
        'logs/refs',
        'objects',
        'packed-refs',
        'refs',
        'remotes',
        'rr-cache',
        'shallow',
    ]
    make_workdir_common(repository, new_workdir, GIT_DIRECTORY_WHITELIST,
                        ['HEAD'])


def clone_file(repository, new_workdir, link, operation):
    if not os.path.exists(os.path.join(repository, link)):
        return
    link_dir = os.path.dirname(os.path.join(new_workdir, link))
    if not os.path.exists(link_dir):
        os.makedirs(link_dir)
    src = os.path.join(repository, link)
    if os.path.islink(src):
        src = os.path.realpath(src)
    operation(src, os.path.join(new_workdir, link))
