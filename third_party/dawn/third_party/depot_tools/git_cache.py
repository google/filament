#!/usr/bin/env python3
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""A git command for managing a local cache of git repositories."""

import contextlib
import logging
import optparse
import os
import re
import subprocess
import sys
import tempfile
import threading
import time
import urllib.parse

from download_from_google_storage import Gsutil
import gclient_utils
import lockfile
import metrics
import subcommand

# Analogous to gc.autopacklimit git config.
GC_AUTOPACKLIMIT = 50

GIT_CACHE_CORRUPT_MESSAGE = 'WARNING: The Git cache is corrupt.'
INIT_SENTIENT_FILE = ".mirror_init"

# gsutil creates many processes and threads. Creating too many gsutil cp
# processes may result in running out of resources, and may perform worse due to
# contextr switching. This limits how many concurrent gsutil cp processes
# git_cache runs.
GSUTIL_CP_SEMAPHORE = threading.Semaphore(2)

try:
    # pylint: disable=undefined-variable
    WinErr = WindowsError
except NameError:

    class WinErr(Exception):
        pass


class ClobberNeeded(Exception):
    pass


def exponential_backoff_retry(fn,
                              excs=(Exception, ),
                              name=None,
                              count=10,
                              sleep_time=0.25,
                              printerr=None):
    """Executes |fn| up to |count| times, backing off exponentially.

    Args:
        fn (callable): The function to execute. If this raises a handled
            exception, the function will retry with exponential backoff.
        excs (tuple): A tuple of Exception types to handle. If one of these is
            raised by |fn|, a retry will be attempted. If |fn| raises an
            Exception that is not in this list, it will immediately pass
            through. If |excs| is empty, the Exception base class will be used.
        name (str): Optional operation name to print in the retry string.
        count (int): The number of times to try before allowing the exception
            to pass through.
        sleep_time (float): The initial number of seconds to sleep in between
            retries. This will be doubled each retry.
        printerr (callable): Function that will be called with the error string
            upon failures. If None, |logging.warning| will be used.

    Returns: The return value of the successful fn.
    """
    printerr = printerr or logging.warning
    for i in range(count):
        try:
            return fn()
        except excs as e:
            if (i + 1) >= count:
                raise

            printerr('Retrying %s in %.2f second(s) (%d / %d attempts): %s' %
                     ((name or 'operation'), sleep_time, (i + 1), count, e))
            time.sleep(sleep_time)
            sleep_time *= 2


class Mirror(object):

    git_exe = 'git.bat' if sys.platform.startswith('win') else 'git'
    gsutil_exe = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                              'gsutil.py')
    cachepath_lock = threading.Lock()

    UNSET_CACHEPATH = object()

    # Used for tests
    _GIT_CONFIG_LOCATION = []

    @staticmethod
    def parse_fetch_spec(spec):
        """Parses and canonicalizes a fetch spec.

        Returns (fetchspec, value_regex), where value_regex can be used
        with 'git config --replace-all'.
        """
        parts = spec.split(':', 1)
        src = parts[0].lstrip('+').rstrip('/')
        if not src.startswith('refs/'):
            src = 'refs/heads/%s' % src
        dest = parts[1].rstrip('/') if len(parts) > 1 else src
        regex = r'\+%s:.*' % src.replace('*', r'\*')
        return ('+%s:%s' % (src, dest), regex)

    def __init__(self, url, refs=None, commits=None, print_func=None):
        self.url = url
        self.fetch_specs = {self.parse_fetch_spec(ref) for ref in (refs or [])}
        self.fetch_commits = set(commits or [])
        self.basedir = self.UrlToCacheDir(url)
        self.mirror_path = os.path.join(self.GetCachePath(), self.basedir)
        if print_func:
            self.print = self.print_without_file
            self.print_func = print_func
        else:
            self.print = print

    def print_without_file(self, message, **_kwargs):
        self.print_func(message)

    @contextlib.contextmanager
    def print_duration_of(self, what):
        start = time.time()
        try:
            yield
        finally:
            self.print('%s took %.1f minutes' % (what,
                                                 (time.time() - start) / 60.0))

    @property
    def _init_sentient_file(self):
        return os.path.join(self.mirror_path, INIT_SENTIENT_FILE)

    @property
    def bootstrap_bucket(self):
        b = os.getenv('OVERRIDE_BOOTSTRAP_BUCKET')
        if b:
            return b
        u = urllib.parse.urlparse(self.url)
        if u.netloc == 'chromium.googlesource.com':
            return 'chromium-git-cache'
        # Not recognized.
        return None

    @property
    def _gs_path(self):
        return 'gs://%s/v2/%s' % (self.bootstrap_bucket, self.basedir)

    @classmethod
    def FromPath(cls, path):
        return cls(cls.CacheDirToUrl(path))

    @staticmethod
    def UrlToCacheDir(url):
        """Convert a git url to a normalized form for the cache dir path."""
        if os.path.isdir(url):
            # Ignore the drive letter in Windows
            url = os.path.splitdrive(url)[1]
            return url.replace('-', '--').replace(os.sep, '-')

        parsed = urllib.parse.urlparse(url)
        norm_url = parsed.netloc + parsed.path
        if norm_url.endswith('.git'):
            norm_url = norm_url[:-len('.git')]

        # Use the same dir for authenticated URLs and unauthenticated URLs.
        norm_url = norm_url.replace('googlesource.com/a/', 'googlesource.com/')

        norm_url = norm_url.replace(':', '__')

        return norm_url.replace('-', '--').replace('/', '-').lower()

    @staticmethod
    def CacheDirToUrl(path):
        """Convert a cache dir path to its corresponding url."""
        netpath = re.sub(r'\b-\b', '/',
                         os.path.basename(path)).replace('--', '-')

        netpath = netpath.replace('__', ':')

        if netpath.startswith('git@'):
            return netpath

        return 'https://%s' % netpath

    @classmethod
    def SetCachePath(cls, cachepath):
        with cls.cachepath_lock:
            setattr(cls, 'cachepath', cachepath)

    @classmethod
    def GetCachePath(cls):
        with cls.cachepath_lock:
            if not hasattr(cls, 'cachepath'):
                try:
                    cachepath = subprocess.check_output(
                        [cls.git_exe, 'config'] + cls._GIT_CONFIG_LOCATION +
                        ['--type', 'path', 'cache.cachepath']).decode(
                            'utf-8', 'ignore').strip()
                except subprocess.CalledProcessError:
                    cachepath = os.environ.get('GIT_CACHE_PATH',
                                               cls.UNSET_CACHEPATH)
                setattr(cls, 'cachepath', cachepath)

            ret = getattr(cls, 'cachepath')
            if ret is cls.UNSET_CACHEPATH:
                raise RuntimeError('No cache.cachepath git configuration or '
                                   '$GIT_CACHE_PATH is set.')
            return ret

    @staticmethod
    def _GetMostRecentCacheDirectory(ls_out_set):
        ready_file_pattern = re.compile(r'.*/(\d+).ready$')
        ready_dirs = []

        for name in ls_out_set:
            m = ready_file_pattern.match(name)
            # Given <path>/<number>.ready,
            # we are interested in <path>/<number> directory
            if m and (name[:-len('.ready')] + '/') in ls_out_set:
                ready_dirs.append((int(m.group(1)), name[:-len('.ready')]))

        if not ready_dirs:
            return None

        return max(ready_dirs)[1]

    def Rename(self, src, dst):
        # This is somehow racy on Windows.
        # Catching OSError because WindowsError isn't portable and
        # pylint complains.
        exponential_backoff_retry(lambda: os.rename(src, dst),
                                  excs=(OSError, ),
                                  name='rename [%s] => [%s]' % (src, dst),
                                  printerr=self.print)

    def RunGit(self, cmd, print_stdout=True, **kwargs):
        """Run git in a subprocess."""
        cwd = kwargs.setdefault('cwd', self.mirror_path)
        if "--git-dir" not in cmd:
            cmd = ['--git-dir', os.path.abspath(cwd)] + cmd

        kwargs.setdefault('print_stdout', False)
        if print_stdout:
            kwargs.setdefault('filter_fn', self.print)
        env = kwargs.get('env') or kwargs.setdefault('env', os.environ.copy())
        env.setdefault('GIT_ASKPASS', 'true')
        env.setdefault('SSH_ASKPASS', 'true')
        self.print('running "git %s" in "%s"' % (' '.join(cmd), cwd))
        return gclient_utils.CheckCallAndFilter([self.git_exe] + cmd, **kwargs)

    def config(self, reset_fetch_config=False):
        if reset_fetch_config:
            try:
                self.RunGit(['config', '--unset-all', 'remote.origin.fetch'])
            except subprocess.CalledProcessError as e:
                # If exit code was 5, it means we attempted to unset a config
                # that didn't exist. Ignore it.
                if e.returncode != 5:
                    raise

        # Don't run git-gc in a daemon.  Bad things can happen if it gets
        # killed.
        try:
            self.RunGit(['config', 'gc.autodetach', '0'])
        except subprocess.CalledProcessError:
            # Hard error, need to clobber.
            raise ClobberNeeded()

        # Don't combine pack files into one big pack file.  It's really slow for
        # repositories, and there's no way to track progress and make sure it's
        # not stuck.
        if self.supported_project():
            self.RunGit(['config', 'gc.autopacklimit', '0'])

        # Allocate more RAM for cache-ing delta chains, for better performance
        # of "Resolving deltas".
        self.RunGit([
            'config', 'core.deltaBaseCacheLimit',
            gclient_utils.DefaultDeltaBaseCacheLimit()
        ])

        self.RunGit(['config', 'remote.origin.url', self.url])
        self.RunGit([
            'config', '--replace-all', 'remote.origin.fetch',
            '+refs/heads/*:refs/heads/*', r'\+refs/heads/\*:.*'
        ])
        for spec, value_regex in self.fetch_specs:
            self.RunGit([
                'config', '--replace-all', 'remote.origin.fetch', spec,
                value_regex
            ])

    def bootstrap_repo(self, directory):
        """Bootstrap the repo from Google Storage if possible.

        More apt-ly named
        bootstrap_repo_from_cloud_if_possible_else_do_nothing().
        """
        if not self.bootstrap_bucket:
            return False

        gsutil = Gsutil(self.gsutil_exe, boto_path=None)

        # Get the most recent version of the directory.
        # This is determined from the most recent version of a .ready file.
        # The .ready file is only uploaded when an entire directory has been
        # uploaded to GS.
        _, ls_out, ls_err = gsutil.check_call('ls', self._gs_path)
        ls_out_set = set(ls_out.strip().splitlines())
        latest_dir = self._GetMostRecentCacheDirectory(ls_out_set)

        if not latest_dir:
            self.print('No bootstrap file for %s found in %s, stderr:\n  %s' %
                       (self.mirror_path, self.bootstrap_bucket, '  '.join(
                           (ls_err or '').splitlines(True))))
            return False

        try:
            # create new temporary directory locally
            tempdir = tempfile.mkdtemp(prefix='_cache_tmp',
                                       dir=self.GetCachePath())
            self.RunGit(['init', '-b', 'main', '--bare'], cwd=tempdir)
            self.print('Downloading files in %s/* into %s.' %
                       (latest_dir, tempdir))
            with self.print_duration_of('download'):
                with GSUTIL_CP_SEMAPHORE:
                    code = gsutil.call('-m', 'cp', '-r', latest_dir + "/*",
                                       tempdir)
            if code:
                return False
            # A quick validation that all references are valid.
            self.RunGit(['for-each-ref'], print_stdout=False, cwd=tempdir)
        except Exception as e:
            self.print('Encountered error: %s' % str(e), file=sys.stderr)
            gclient_utils.rmtree(tempdir)
            return False
        # delete the old directory
        if os.path.exists(directory):
            gclient_utils.rmtree(directory)
        self.Rename(tempdir, directory)
        return True

    def contains_revision(self, revision):
        if not self.exists():
            return False

        if sys.platform.startswith('win'):
            # Windows .bat scripts use ^ as escape sequence, which means we have
            # to escape it with itself for every .bat invocation.
            needle = '%s^^^^{commit}' % revision
        else:
            needle = '%s^{commit}' % revision
        try:
            # cat-file exits with 0 on success, that is git object of given hash
            # was found.
            self.RunGit(['cat-file', '-e', needle])
            return True
        except subprocess.CalledProcessError:
            self.print('Commit with hash "%s" not found' % revision,
                       file=sys.stderr)
            return False

    def exists(self):
        return os.path.isfile(os.path.join(self.mirror_path, 'config'))

    def supported_project(self):
        """Returns true if this repo is known to have a bootstrap zip file."""
        u = urllib.parse.urlparse(self.url)
        return u.netloc in [
            'chromium.googlesource.com', 'chrome-internal.googlesource.com'
        ]

    def _preserve_fetchspec(self):
        """Read and preserve remote.origin.fetch from an existing mirror.

        This modifies self.fetch_specs.
        """
        if not self.exists():
            return
        try:
            config_fetchspecs = subprocess.check_output([
                self.git_exe, '--git-dir', self.mirror_path, 'config',
                '--get-all', 'remote.origin.fetch'
            ]).decode('utf-8', 'ignore')
            for fetchspec in config_fetchspecs.splitlines():
                self.fetch_specs.add(self.parse_fetch_spec(fetchspec))
        except subprocess.CalledProcessError:
            logging.warning(
                'Tried and failed to preserve remote.origin.fetch from the '
                'existing cache directory.  You may need to manually edit '
                '%s and "git cache fetch" again.' %
                os.path.join(self.mirror_path, 'config'))

    def _ensure_bootstrapped(self,
                             depth,
                             bootstrap,
                             reset_fetch_config,
                             force=False):
        pack_dir = os.path.join(self.mirror_path, 'objects', 'pack')
        pack_files = []
        if os.path.isdir(pack_dir):
            pack_files = [
                f for f in os.listdir(pack_dir) if f.endswith('.pack')
            ]
            self.print('%s has %d .pack files, re-bootstrapping if >%d or ==0' %
                       (self.mirror_path, len(pack_files), GC_AUTOPACKLIMIT))

        # master->main branch migration left the cache in some builders to have
        # its HEAD still pointing to refs/heads/master. This causes bot_update
        # to fail. If in this state, delete the cache and force bootstrap.
        try:
            with open(os.path.join(self.mirror_path, 'HEAD')) as f:
                head_ref = f.read()
        except FileNotFoundError:
            head_ref = ''

        # Check only when HEAD points to master.
        if 'master' in head_ref:
            # Some repos could still have master so verify if the ref exists
            # first.
            show_ref_master_cmd = subprocess.run([
                Mirror.git_exe, '--git-dir', self.mirror_path, 'show-ref',
                '--verify', 'refs/heads/master'
            ])

            if show_ref_master_cmd.returncode != 0:
                # Remove mirror
                gclient_utils.rmtree(self.mirror_path)

                # force bootstrap
                force = True

        should_bootstrap = (force or not self.exists()
                            or len(pack_files) > GC_AUTOPACKLIMIT
                            or len(pack_files) == 0)

        if not should_bootstrap:
            if depth and os.path.exists(
                    os.path.join(self.mirror_path, 'shallow')):
                logging.warning(
                    'Shallow fetch requested, but repo cache already exists.')
            return

        if not self.exists():
            if os.path.exists(self.mirror_path):
                # If the mirror path exists but self.exists() returns false,
                # we're in an unexpected state. Nuke the previous mirror
                # directory and start fresh.
                gclient_utils.rmtree(self.mirror_path)
            os.mkdir(self.mirror_path)
        elif not reset_fetch_config:
            # Re-bootstrapping an existing mirror; preserve existing fetch spec.
            self._preserve_fetchspec()

        bootstrapped = (not depth and bootstrap
                        and self.bootstrap_repo(self.mirror_path))

        if not bootstrapped:
            if not self.exists() or not self.supported_project():
                # Bootstrap failed due to:
                # 1. No previous cache.
                # 2. Project doesn't have a bootstrap folder.
                # Start with a bare git dir.
                self.RunGit(['init', '--bare'])
                with open(self._init_sentient_file, 'w'):
                    # Create sentient file
                    pass
                self._set_symbolic_ref()
            else:
                # Bootstrap failed, previous cache exists; warn and continue.
                logging.warning(
                    'Git cache has a lot of pack files (%d). Tried to '
                    're-bootstrap but failed. Continuing with non-optimized '
                    'repository.' % len(pack_files))

    def _set_symbolic_ref(self):
        remote_info = exponential_backoff_retry(lambda: subprocess.check_output(
            [
                self.git_exe, '--git-dir',
                os.path.abspath(self.mirror_path), 'remote', 'show', self.url
            ],
            cwd=self.mirror_path).decode('utf-8', 'ignore').strip())
        default_branch_regexp = re.compile(r'HEAD branch: (.*)')
        m = default_branch_regexp.search(remote_info, re.MULTILINE)
        if m:
            self.RunGit(['symbolic-ref', 'HEAD', 'refs/heads/' + m.groups()[0]])


    def _fetch(self,
               verbose,
               depth,
               no_fetch_tags,
               reset_fetch_config,
               prune=True):
        self.config(reset_fetch_config)

        fetch_cmd = ['fetch']
        if verbose:
            fetch_cmd.extend(['-v', '--progress'])
        if depth:
            fetch_cmd.extend(['--depth', str(depth)])
        if no_fetch_tags:
            fetch_cmd.append('--no-tags')
        if prune:
            fetch_cmd.append('--prune')
        fetch_cmd.append('origin')

        fetch_specs = subprocess.check_output(
            [
                self.git_exe, '--git-dir',
                os.path.abspath(self.mirror_path), 'config', '--get-all',
                'remote.origin.fetch'
            ],
            cwd=self.mirror_path).decode('utf-8',
                                         'ignore').strip().splitlines()
        for spec in fetch_specs:
            try:
                self.print('Fetching %s' % spec)
                with self.print_duration_of('fetch %s' % spec):
                    self.RunGit(fetch_cmd + [spec], retry=True)
            except subprocess.CalledProcessError:
                if spec == '+refs/heads/*:refs/heads/*':
                    raise ClobberNeeded()  # Corrupted cache.
                logging.warning('Fetch of %s failed' % spec)
        for commit in self.fetch_commits:
            self.print('Fetching %s' % commit)
            try:
                with self.print_duration_of('fetch %s' % commit):
                    self.RunGit(['fetch', 'origin', commit], retry=True)
            except subprocess.CalledProcessError:
                logging.warning('Fetch of %s failed' % commit)
        if os.path.isfile(self._init_sentient_file):
            os.remove(self._init_sentient_file)

        # Since --prune is used, it's possible that HEAD no longer exists (e.g.
        # a repo uses new HEAD and old is removed). This ensures that HEAD still
        # points to a valid commit, otherwise gets a new HEAD.
        out = self.RunGit(['rev-parse', 'HEAD'], print_stdout=False)
        if out.startswith(b'HEAD'):
            self._set_symbolic_ref()

    def populate(self,
                 depth=None,
                 no_fetch_tags=False,
                 shallow=False,
                 bootstrap=False,
                 verbose=False,
                 lock_timeout=0,
                 reset_fetch_config=False):
        assert self.GetCachePath()
        if shallow and not depth:
            depth = 10000
        gclient_utils.safe_makedirs(self.GetCachePath())

        def bootstrap_cache(force=False):
            self._ensure_bootstrapped(depth,
                                      bootstrap,
                                      reset_fetch_config,
                                      force=force)
            self._fetch(verbose, depth, no_fetch_tags, reset_fetch_config)

        def wipe_cache():
            self.print(GIT_CACHE_CORRUPT_MESSAGE)
            gclient_utils.rmtree(self.mirror_path)

        with lockfile.lock(self.mirror_path, lock_timeout):
            if os.path.isfile(self._init_sentient_file):
                # Previous bootstrap didn't finish
                wipe_cache()

            try:
                bootstrap_cache()
            except ClobberNeeded:
                # This is a major failure, we need to clean and force a
                # bootstrap.
                wipe_cache()
                bootstrap_cache(force=True)

    def update_bootstrap(self, prune=False, gc_aggressive=False):
        # NOTE: There have been cases where repos were being recursively
        # uploaded to google storage. E.g.
        # `<host_url>-<repo>/<gen_number>/<host_url>-<repo>/` in GS and
        # <host_url>-<repo>/<host_url>-<repo>/ on the bot. Check for recursed
        # files on the bot here and remove them if found before we upload to GS.
        # See crbug.com/1370443; keep this check until root cause is found.
        recursed_dir = os.path.join(self.mirror_path,
                                    self.mirror_path.split(os.path.sep)[-1])
        if os.path.exists(recursed_dir):
            self.print('Deleting unexpected directory: %s' % recursed_dir)
            gclient_utils.rmtree(recursed_dir)

        # The folder is <git number>
        gen_number = subprocess.check_output(
            [self.git_exe, '--git-dir', self.mirror_path,
             'number']).decode('utf-8', 'ignore').strip()
        gsutil = Gsutil(path=self.gsutil_exe, boto_path=None)

        dest_prefix = '%s/%s' % (self._gs_path, gen_number)

        # ls_out lists contents in the format: gs://blah/blah/123...
        self.print('running "gsutil ls %s":' % self._gs_path)
        ls_code, ls_out, ls_error = gsutil.check_call_with_retries(
            'ls', self._gs_path)
        if ls_code != 0:
            self.print(ls_error)
        else:
            self.print(ls_out)

        # Check to see if folder already exists in gs
        ls_out_set = set(ls_out.strip().splitlines())
        if (dest_prefix + '/' in ls_out_set
                and dest_prefix + '.ready' in ls_out_set):
            print('Cache %s already exists.' % dest_prefix)
            return

        # Reduce the number of individual files to download & write on disk.
        self.RunGit(['pack-refs', '--all'])

        # Run Garbage Collect to compress packfile.
        gc_args = ['gc', '--prune=all']
        if gc_aggressive:
            # The default "gc --aggressive" is often too aggressive for some
            # machines, since it attempts to create as many threads as there are
            # CPU cores, while not limiting per-thread memory usage, which puts
            # too much pressure on RAM on high-core machines, causing them to
            # thrash. Using lower-level commands gives more control over those
            # settings.

            # This might not be strictly necessary, but it's fast and is
            # normally run by 'gc --aggressive', so it shouldn't hurt.
            self.RunGit(['reflog', 'expire', '--all'])

            # These are the default repack settings for 'gc --aggressive'.
            gc_args = [
                'repack', '-d', '-l', '-f', '--depth=50', '--window=250', '-A',
                '--unpack-unreachable=all'
            ]
            # A 1G memory limit seems to provide comparable pack results as the
            # default, even for our largest repos, while preventing runaway
            # memory (at least on current Chromium builders which have about 4G
            # RAM per core).
            gc_args.append('--window-memory=1g')
            # NOTE: It might also be possible to avoid thrashing with a larger
            # window (e.g. "--window-memory=2g") by limiting the number of
            # threads created (e.g. "--threads=[cores/2]"). Some limited testing
            # didn't show much difference in outcomes on our current repos, but
            # it might be worth trying if the repos grow much larger and the
            # packs don't seem to be getting compressed enough.
        self.RunGit(gc_args)

        self.print('running "gsutil -m rsync -r -d %s %s"' %
                   (self.mirror_path, dest_prefix))
        gsutil.call('-m', 'rsync', '-r', '-d', self.mirror_path, dest_prefix)

        # Create .ready file and upload
        _, ready_file_name = tempfile.mkstemp(suffix='.ready')
        try:
            self.print('running "gsutil cp %s %s.ready"' %
                       (ready_file_name, dest_prefix))
            gsutil.call('cp', ready_file_name, '%s.ready' % (dest_prefix))
        finally:
            os.remove(ready_file_name)

        # remove all other directory/.ready files in the same gs_path
        # except for the directory/.ready file previously created
        # which can be used for bootstrapping while the current one is
        # being uploaded
        if not prune:
            return
        prev_dest_prefix = self._GetMostRecentCacheDirectory(ls_out_set)
        if not prev_dest_prefix:
            return
        for path in ls_out_set:
            if path in (prev_dest_prefix + '/', prev_dest_prefix + '.ready'):
                continue
            if path.endswith('.ready'):
                gsutil.call('rm', path)
                continue
            gsutil.call('-m', 'rm', '-r', path)

    @staticmethod
    def DeleteTmpPackFiles(path):
        pack_dir = os.path.join(path, 'objects', 'pack')
        if not os.path.isdir(pack_dir):
            return
        pack_files = [
            f for f in os.listdir(pack_dir)
            if f.startswith('.tmp-') or f.startswith('tmp_pack_')
        ]
        for f in pack_files:
            f = os.path.join(pack_dir, f)
            try:
                os.remove(f)
                logging.warning('Deleted stale temporary pack file %s' % f)
            except OSError:
                logging.warning('Unable to delete temporary pack file %s' % f)


@subcommand.usage('[url of repo to check for caching]')
@metrics.collector.collect_metrics('git cache exists')
def CMDexists(parser, args):
    """Check to see if there already is a cache of the given repo."""
    _, args = parser.parse_args(args)
    if not len(args) == 1:
        parser.error('git cache exists only takes exactly one repo url.')
    url = args[0]
    mirror = Mirror(url)
    if mirror.exists():
        print(mirror.mirror_path)
        return 0
    return 1


@subcommand.usage('[url of repo to create a bootstrap zip file]')
@metrics.collector.collect_metrics('git cache update-bootstrap')
def CMDupdate_bootstrap(parser, args):
    """Create and uploads a bootstrap tarball."""
    # Lets just assert we can't do this on Windows.
    if sys.platform.startswith('win'):
        print('Sorry, update bootstrap will not work on Windows.',
              file=sys.stderr)
        return 1

    if gclient_utils.IsEnvCog():
        print('updating bootstrap is not supported in non-git environment.',
              file=sys.stderr)
        return 1

    parser.add_option('--skip-populate',
                      action='store_true',
                      help='Skips "populate" step if mirror already exists.')
    parser.add_option('--gc-aggressive',
                      action='store_true',
                      help='Run aggressive repacking of the repo.')
    parser.add_option('--prune',
                      action='store_true',
                      help='Prune all other cached bundles of the same repo.')

    populate_args = args[:]
    options, args = parser.parse_args(args)
    url = args[0]
    mirror = Mirror(url)
    if not options.skip_populate or not mirror.exists():
        CMDpopulate(parser, populate_args)
    else:
        print('Skipped populate step.')

    # Get the repo directory.
    _, args2 = parser.parse_args(args)
    url = args2[0]
    mirror = Mirror(url)
    mirror.update_bootstrap(options.prune, options.gc_aggressive)
    return 0


@subcommand.usage('[url of repo to add to or update in cache]')
@metrics.collector.collect_metrics('git cache populate')
def CMDpopulate(parser, args):
    """Ensure that the cache has all up-to-date objects for the given repo."""
    if gclient_utils.IsEnvCog():
        print('populating cache is not supported in non-git environment.',
              file=sys.stderr)
        return 1

    parser.add_option('--depth',
                      type='int',
                      help='Only cache DEPTH commits of history')
    parser.add_option(
        '--no-fetch-tags',
        action='store_true',
        help=('Don\'t fetch tags from the server. This can speed up '
              'fetch considerably when there are many tags.'))
    parser.add_option('--shallow',
                      '-s',
                      action='store_true',
                      help='Only cache 10000 commits of history')
    parser.add_option('--ref',
                      action='append',
                      help='Specify additional refs to be fetched')
    parser.add_option('--commit',
                      action='append',
                      help='Specify additional commits to be fetched')
    parser.add_option('--no_bootstrap',
                      '--no-bootstrap',
                      action='store_true',
                      help='Don\'t bootstrap from Google Storage')
    parser.add_option('--ignore_locks',
                      '--ignore-locks',
                      action='store_true',
                      help='NOOP. This flag will be removed in the future.')
    parser.add_option(
        '--break-locks',
        action='store_true',
        help='Break any existing lock instead of just ignoring it')
    parser.add_option(
        '--reset-fetch-config',
        action='store_true',
        default=False,
        help='Reset the fetch config before populating the cache.')

    options, args = parser.parse_args(args)
    if not len(args) == 1:
        parser.error('git cache populate only takes exactly one repo url.')
    if options.ignore_locks:
        print('ignore_locks is no longer used. Please remove its usage.')
    if options.break_locks:
        print('break_locks is no longer used. Please remove its usage.')
    url = args[0]

    mirror = Mirror(url, refs=options.ref, commits=options.commit)
    kwargs = {
        'no_fetch_tags': options.no_fetch_tags,
        'verbose': options.verbose,
        'shallow': options.shallow,
        'bootstrap': not options.no_bootstrap,
        'lock_timeout': options.timeout,
        'reset_fetch_config': options.reset_fetch_config,
    }
    if options.depth:
        kwargs['depth'] = options.depth
    mirror.populate(**kwargs)


@subcommand.usage('Fetch new commits into cache and current checkout')
@metrics.collector.collect_metrics('git cache fetch')
def CMDfetch(parser, args):
    """Update mirror, and fetch in cwd."""
    if gclient_utils.IsEnvCog():
        print(
            'fetching new commits into cache is not supported in non-git '
            'environment.',
            file=sys.stderr)
        return 1

    parser.add_option('--all', action='store_true', help='Fetch all remotes')
    parser.add_option('--no_bootstrap',
                      '--no-bootstrap',
                      action='store_true',
                      help='Don\'t (re)bootstrap from Google Storage')
    parser.add_option(
        '--no-fetch-tags',
        action='store_true',
        help=('Don\'t fetch tags from the server. This can speed up '
              'fetch considerably when there are many tags.'))
    options, args = parser.parse_args(args)

    # Figure out which remotes to fetch.  This mimics the behavior of regular
    # 'git fetch'.  Note that in the case of "stacked" or "pipelined" branches,
    # this will NOT try to traverse up the branching structure to find the
    # ultimate remote to update.
    remotes = []
    if options.all:
        assert not args, 'fatal: fetch --all does not take repository argument'
        remotes = subprocess.check_output([Mirror.git_exe, 'remote'])
        remotes = remotes.decode('utf-8', 'ignore').splitlines()
    elif args:
        remotes = args
    else:
        current_branch = subprocess.check_output(
            [Mirror.git_exe, 'rev-parse', '--abbrev-ref', 'HEAD'])
        current_branch = current_branch.decode('utf-8', 'ignore').strip()
        if current_branch != 'HEAD':
            upstream = subprocess.check_output(
                [Mirror.git_exe, 'config',
                 'branch.%s.remote' % current_branch])
            upstream = upstream.decode('utf-8', 'ignore').strip()
            if upstream and upstream != '.':
                remotes = [upstream]
    if not remotes:
        remotes = ['origin']

    cachepath = Mirror.GetCachePath()
    git_dir = os.path.abspath(
        subprocess.check_output([Mirror.git_exe, 'rev-parse',
                                 '--git-dir']).decode('utf-8', 'ignore'))
    git_dir = os.path.abspath(git_dir)
    if git_dir.startswith(cachepath):
        mirror = Mirror.FromPath(git_dir)
        mirror.populate(bootstrap=not options.no_bootstrap,
                        no_fetch_tags=options.no_fetch_tags,
                        lock_timeout=options.timeout)
        return 0
    for remote in remotes:
        remote_url = subprocess.check_output(
            [Mirror.git_exe, 'config',
             'remote.%s.url' % remote])
        remote_url = remote_url.decode('utf-8', 'ignore').strip()
        if remote_url.startswith(cachepath):
            mirror = Mirror.FromPath(remote_url)
            mirror.print = lambda *args: None
            print('Updating git cache...')
            mirror.populate(bootstrap=not options.no_bootstrap,
                            no_fetch_tags=options.no_fetch_tags,
                            lock_timeout=options.timeout)
        subprocess.check_call([Mirror.git_exe, 'fetch', remote])
    return 0


class OptionParser(optparse.OptionParser):
    """Wrapper class for OptionParser to handle global options."""
    def __init__(self, *args, **kwargs):
        optparse.OptionParser.__init__(self, *args, prog='git cache', **kwargs)
        self.add_option(
            '-c',
            '--cache-dir',
            help=('Path to the directory containing the caches. Normally '
                  'deduced from git config cache.cachepath or '
                  '$GIT_CACHE_PATH.'))
        self.add_option(
            '-v',
            '--verbose',
            action='count',
            default=1,
            help='Increase verbosity (can be passed multiple times)')
        self.add_option('-q',
                        '--quiet',
                        action='store_true',
                        help='Suppress all extraneous output')
        self.add_option('--timeout',
                        type='int',
                        default=0,
                        help='Timeout for acquiring cache lock, in seconds')

    def parse_args(self, args=None, values=None):
        # Create an optparse.Values object that will store only the actual
        # passed options, without the defaults.
        actual_options = optparse.Values()
        _, args = optparse.OptionParser.parse_args(self, args, actual_options)
        # Create an optparse.Values object with the default options.
        options = optparse.Values(self.get_default_values().__dict__)
        # Update it with the options passed by the user.
        options._update_careful(actual_options.__dict__)
        # Store the options passed by the user in an _actual_options attribute.
        # We store only the keys, and not the values, since the values can
        # contain arbitrary information, which might be PII.
        metrics.collector.add('arguments', list(actual_options.__dict__.keys()))

        if options.quiet:
            options.verbose = 0

        levels = [logging.ERROR, logging.WARNING, logging.INFO, logging.DEBUG]
        logging.basicConfig(level=levels[min(options.verbose, len(levels) - 1)])

        try:
            global_cache_dir = Mirror.GetCachePath()
        except RuntimeError:
            global_cache_dir = None
        if options.cache_dir:
            if global_cache_dir and (os.path.abspath(options.cache_dir) !=
                                     os.path.abspath(global_cache_dir)):
                logging.warning(
                    'Overriding globally-configured cache directory.')
            Mirror.SetCachePath(options.cache_dir)

        return options, args


def main(argv):
    dispatcher = subcommand.CommandDispatcher(__name__)
    return dispatcher.execute(OptionParser(), argv)


if __name__ == '__main__':
    try:
        with metrics.collector.print_notice_and_exit():
            sys.exit(main(sys.argv[1:]))
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
