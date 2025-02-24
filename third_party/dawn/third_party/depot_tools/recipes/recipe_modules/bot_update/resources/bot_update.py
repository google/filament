#!/usr/bin/env python3
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# TODO(hinoka): Use logging.

import codecs
from contextlib import contextmanager
import copy
import ctypes
from datetime import datetime
import functools
import json
import optparse
import os
import pprint
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import time
import uuid

import os.path as path

from io import BytesIO
from urllib.parse import urlparse

# How many bytes at a time to read from pipes.
BUF_SIZE = 256

# How many seconds of no stdout activity before process is considered stale. Can
# be overridden via environment variable `STALE_PROCESS_DURATION`. If set to 0,
# process won't be terminated.
STALE_PROCESS_DURATION = 1200

# Define a bunch of directory paths.
# Relative to this script's filesystem path.
THIS_DIR = path.dirname(path.abspath(__file__))

DEPOT_TOOLS_DIR = path.abspath(path.join(THIS_DIR, '..', '..', '..', '..'))

CHROMIUM_GIT_HOST = 'https://chromium.googlesource.com'
CHROMIUM_SRC_URL = CHROMIUM_GIT_HOST + '/chromium/src.git'

BRANCH_HEADS_REFSPEC = '+refs/branch-heads/*'
TAGS_REFSPEC = '+refs/tags/*'

# Regular expression to match sha1 git revision.
COMMIT_HASH_RE = re.compile(r'[0-9a-f]{5,40}', re.IGNORECASE)

# Regular expression that matches a single commit footer line.
COMMIT_FOOTER_ENTRY_RE = re.compile(r'([^:]+):\s*(.*)')

# Footer metadata keys for regular and gsubtreed mirrored commit positions.
COMMIT_POSITION_FOOTER_KEY = 'Cr-Commit-Position'
COMMIT_ORIGINAL_POSITION_FOOTER_KEY = 'Cr-Original-Commit-Position'

# Copied from scripts/recipes/chromium.py.
GOT_REVISION_MAPPINGS = {
    CHROMIUM_SRC_URL: {
        'got_revision': 'src/',
        'got_nacl_revision': 'src/native_client/',
        'got_v8_revision': 'src/v8/',
        'got_webkit_revision': 'src/third_party/WebKit/',
        'got_webrtc_revision': 'src/third_party/webrtc/',
    }
}

# List of bot update experiments
# Gclient will skip deps syncing if there have been no DEPS changes
# since the last sync on the bot.
EXP_NO_SYNC = 'no-sync'

GCLIENT_TEMPLATE = """solutions = %(solutions)s

cache_dir = r%(cache_dir)s
%(target_os)s
%(target_os_only)s
%(target_cpu)s
"""


GIT_CACHE_PATH = path.join(DEPOT_TOOLS_DIR, 'git_cache.py')
GCLIENT_PATH = path.join(DEPOT_TOOLS_DIR, 'gclient.py')

class SubprocessFailed(Exception):
  def __init__(self, message, code, output):
    self.message = message
    self.code = code
    self.output = output


class PatchFailed(SubprocessFailed):
  pass


class GclientSyncFailed(SubprocessFailed):
  pass


class InvalidDiff(Exception):
  pass


RETRY = object()
OK = object()
FAIL = object()


class RepeatingTimer(threading.Thread):
  """Call a function every n seconds, unless reset."""
  def __init__(self, interval, function, args=None, kwargs=None):
    threading.Thread.__init__(self)
    self.interval = interval
    self.function = function
    self.args = args if args is not None else []
    self.kwargs = kwargs if kwargs is not None else {}
    self.cond = threading.Condition()
    self.is_shutdown = False
    self.is_reset = False

  def reset(self):
    """Resets timer interval."""
    with self.cond:
      self.is_reset = True
      self.cond.notify_all()

  def shutdown(self):
    """Stops repeating timer."""
    with self.cond:
      self.is_shutdown = True
      self.cond.notify_all()

  def run(self):
    with self.cond:
      while not self.is_shutdown:
        self.cond.wait(self.interval)
        if not self.is_reset and not self.is_shutdown:
          self.function(*self.args, **self.kwargs)
        self.is_reset = False


def _print_pstree():
  """Debugging function used to print "ps auxwwf" for stuck processes."""
  if sys.platform.startswith('linux'):
    # Add new line for cleaner output
    print()
    subprocess.call(['ps', 'auxwwf'])


def _kill_process(proc):
  print('Killing stale process...')
  proc.kill()



def call(*args, **kwargs):  # pragma: no cover
  """Interactive subprocess call."""
  kwargs['stdout'] = subprocess.PIPE
  kwargs['stderr'] = subprocess.STDOUT
  kwargs.setdefault('bufsize', BUF_SIZE)
  cwd = kwargs.get('cwd', os.getcwd())
  stdin_data = kwargs.pop('stdin_data', None)
  if stdin_data:
    kwargs['stdin'] = subprocess.PIPE
  out = BytesIO()
  new_env = kwargs.get('env', {})
  env = os.environ.copy()
  env.update(new_env)
  kwargs['env'] = env
  stale_process_duration = env.get('STALE_PROCESS_DURATION',
                                   STALE_PROCESS_DURATION)

  if new_env:
    print('===Injecting Environment Variables===')
    for k, v in sorted(new_env.items()):
      print('%s: %s' % (k, v))
  print('%s ===Running %s ===' % (datetime.now(), ' '.join(args),))
  print('In directory: %s' % cwd)
  start_time = time.time()
  try:
    proc = subprocess.Popen(args, **kwargs)
  except:
    print('\t%s failed to execute.' % ' '.join(args))
    raise
  observers = [
      RepeatingTimer(300, _print_pstree),
      RepeatingTimer(int(stale_process_duration), _kill_process, [proc])]

  for observer in observers:
    observer.start()

  try:
    # If there's an exception in this block, we need to stop all observers.
    # Otherwise, observers will be spinning and main process won't exit while
    # the main thread will be doing nothing.
    if stdin_data:
      proc.stdin.write(stdin_data)
      proc.stdin.close()
    # This is here because passing 'sys.stdout' into stdout for proc will
    # produce out of order output.
    hanging_cr = False
    while True:
      for observer in observers:
        observer.reset()
      buf = proc.stdout.read(BUF_SIZE)
      if not buf:
        break
      if hanging_cr:
        buf = b'\r' + buf
      hanging_cr = buf.endswith(b'\r')
      if hanging_cr:
        buf = buf[:-1]
      buf = buf.replace(b'\r\n', b'\n').replace(b'\r', b'\n')
      sys.stdout.buffer.write(buf)
      out.write(buf)
    if hanging_cr:
      sys.stdout.buffer.write(b'\n')
      out.write(b'\n')

    code = proc.wait()
  finally:
    for observer in observers:
      observer.shutdown()

  elapsed_time = ((time.time() - start_time) / 60.0)
  outval = out.getvalue().decode('utf-8')
  if code:
    print('%s ===Failed in %.1f mins of %s ===' %
          (datetime.now(), elapsed_time, ' '.join(args)))
    print()
    raise SubprocessFailed('%s failed with code %d in %s.' %
                           (' '.join(args), code, cwd),
                           code, outval)

  print('%s ===Succeeded in %.1f mins of %s ===' %
        (datetime.now(), elapsed_time, ' '.join(args)))
  print()
  return outval


def git(*args, **kwargs):  # pragma: no cover
  """Wrapper around call specifically for Git commands."""
  if args and args[0] == 'cache':
    # Rewrite "git cache" calls into "python git_cache.py".
    cmd = (sys.executable, '-u', GIT_CACHE_PATH) + args[1:]
  else:
    git_executable = 'git'
    # On windows, subprocess doesn't fuzzy-match 'git' to 'git.bat', so we
    # have to do it explicitly. This is better than passing shell=True.
    if sys.platform.startswith('win'):
      git_executable += '.bat'
    cmd = (git_executable,) + args
  return call(*cmd, **kwargs)


def get_gclient_spec(solutions, target_os, target_os_only, target_cpu,
                     git_cache_dir):
  return GCLIENT_TEMPLATE % {
      'solutions': pprint.pformat(solutions, indent=4),
      'cache_dir': '"%s"' % git_cache_dir,
      'target_os': ('\ntarget_os=%s' % target_os) if target_os else '',
      'target_os_only': '\ntarget_os_only=%s' % target_os_only,
      'target_cpu': ('\ntarget_cpu=%s' % target_cpu) if target_cpu else ''
  }


def solutions_printer(solutions):
  """Prints gclient solution to stdout."""
  print('Gclient Solutions')
  print('=================')
  for solution in solutions:
    name = solution.get('name')
    url = solution.get('url')
    print('%s (%s)' % (name, url))
    if solution.get('deps_file'):
      print('  Dependencies file is %s' % solution['deps_file'])
    if 'managed' in solution:
      print('  Managed mode is %s' % ('ON' if solution['managed'] else 'OFF'))
    custom_vars = solution.get('custom_vars')
    if custom_vars:
      print('  Custom Variables:')
      for var_name, var_value in sorted(custom_vars.items()):
        print('    %s = %s' % (var_name, var_value))
    custom_deps = solution.get('custom_deps')
    if 'custom_deps' in solution:
      print('  Custom Dependencies:')
      for deps_name, deps_value in sorted(custom_deps.items()):
        if deps_value:
          print('    %s -> %s' % (deps_name, deps_value))
        else:
          print('    %s: Ignore' % deps_name)
    for k, v in solution.items():
      # Print out all the keys we don't know about.
      if k in ['name', 'url', 'deps_file', 'custom_vars', 'custom_deps',
               'managed']:
        continue
      print('  %s is %s' % (k, v))
    print()


def modify_solutions(input_solutions):
  """Modifies urls in solutions to point at Git repos.

  returns: new solution dictionary
  """
  assert input_solutions
  solutions = copy.deepcopy(input_solutions)
  for solution in solutions:
    original_url = solution['url']
    parsed_url = urlparse(original_url)
    parsed_path = parsed_url.path

    solution['managed'] = False
    # We don't want gclient to be using a safesync URL. Instead it should
    # using the lkgr/lkcr branch/tags.
    if 'safesync_url' in solution:
      print('Removing safesync url %s from %s' % (solution['safesync_url'],
                                                  parsed_path))
      del solution['safesync_url']

  return solutions


def remove(target, cleanup_dir):
  """Remove a target by moving it into cleanup_dir."""
  if not path.exists(cleanup_dir):
    os.makedirs(cleanup_dir)
  dest = path.join(cleanup_dir, '%s_%s' % (
      path.basename(target), uuid.uuid4().hex))
  print('Marking for removal %s => %s' % (target, dest))
  try:
    os.rename(target, dest)
  except OSError as os_e:
    print('Error renaming %s to %s: %s' % (target, dest, str(os_e)))
    if not target.endswith(('/.', '\.')):
      raise
    # Because the solutions name is '.', we might be in a bot
    # directory that is locked to prevent renaming. Instead
    # try moving renaming all content within the directory.
    print('Trying to rename all contents %s -> %s' % (target, dest))
    if target.endswith('.'):
      allfiles = os.listdir(target)
      for f in allfiles:
        target_path = os.path.join(target, f)
        dst_path = os.path.join(dest, f)
        os.renames(target_path, dst_path)
  except Exception as e:
    print('Error renaming %s to %s: %s' % (target, dest, str(e)))
    raise


def ensure_no_checkout(dir_names, cleanup_dir):
  """Ensure that there is no undesired checkout under build/."""
  build_dir = os.getcwd()
  has_checkout = any(path.exists(path.join(build_dir, dir_name, '.git'))
                     for dir_name in dir_names)
  if has_checkout:
    for filename in os.listdir(build_dir):
      deletion_target = path.join(build_dir, filename)
      print('.git detected in checkout, deleting %s...' % deletion_target,)
      remove(deletion_target, cleanup_dir)
      print('done')


def call_gclient(*args, **kwargs):
  """Run the "gclient.py" tool with the supplied arguments.

  Args:
    args: command-line arguments to pass to gclient.
    kwargs: keyword arguments to pass to call.
  """
  cmd = [sys.executable, '-u', GCLIENT_PATH]
  cmd.extend(args)
  return call(*cmd, **kwargs)


def gclient_configure(solutions, target_os, target_os_only, target_cpu,
                      git_cache_dir):
  """Should do the same thing as gclient --spec='...'."""
  with codecs.open('.gclient', mode='w', encoding='utf-8') as f:
    f.write(get_gclient_spec(
        solutions, target_os, target_os_only, target_cpu, git_cache_dir))


@contextmanager
def git_config_if_not_set(key, value):
  """Set git config for key equal to value if key was not set.

  If key was not set, unset it once we're done."""
  should_unset = True
  try:
    git('config', '--global', key)
    should_unset = False
  except SubprocessFailed as e:
    git('config', '--global', key, value)
  try:
    yield
  finally:
    if should_unset:
      git('config', '--global', '--unset', key)


def gclient_sync(with_branch_heads,
                 with_tags,
                 revisions,
                 patch_refs,
                 gerrit_reset,
                 gerrit_rebase_patch_ref,
                 download_topics=False,
                 experiments=None):
  args = [
      'sync', '--verbose', '--reset', '--force', '--upstream', '--nohooks',
      '--noprehooks', '--delete_unversioned_trees'
  ]
  if with_branch_heads:
    args += ['--with_branch_heads']
  if with_tags:
    args += ['--with_tags']
  for name, revision in sorted(revisions.items()):
    if revision.upper() == 'HEAD':
      revision = 'refs/remotes/origin/main'
    args.extend(['--revision', '%s@%s' % (name, revision)])

  if patch_refs:
    for patch_ref in patch_refs:
      args.extend(['--patch-ref', patch_ref])
    if not gerrit_reset:
      args.append('--no-reset-patch-ref')
    if not gerrit_rebase_patch_ref:
      args.append('--no-rebase-patch-ref')
    if download_topics:
      args.append('--download-topics')

  if EXP_NO_SYNC in experiments:
    args.extend(['--experiment', EXP_NO_SYNC])

  try:
    call_gclient(*args)
  except SubprocessFailed as e:
    # If gclient sync is handling patching, parse the output for a patch error
    # message.
    if 'Failed to apply patch.' in e.output:
      raise PatchFailed(e.message, e.code, e.output)
    # Throw a GclientSyncFailed exception so we can catch this independently.
    raise GclientSyncFailed(e.message, e.code, e.output)


def normalize_git_url(url):
  """Normalize a git url to be consistent.

  This recognizes urls to the googlesoruce.com domain.  It ensures that
  the url:
  * Do not end in .git
  * Do not contain /a/ in their path.
  """
  try:
    p = urlparse(url)
  except Exception:
    # Not a url, just return it back.
    return url
  if not p.netloc.endswith('.googlesource.com'):
    # Not a googlesource.com URL, can't normalize this, just return as is.
    return url
  upath = p.path
  if upath.startswith('/a'):
    upath = upath[len('/a'):]
  if upath.endswith('.git'):
    upath = upath[:-len('.git')]
  return 'https://%s%s' % (p.netloc, upath)


def create_manifest():
  fd, fname = tempfile.mkstemp()
  os.close(fd)
  try:
    revinfo = call_gclient(
        'revinfo', '-a', '--ignore-dep-type', 'cipd', '--output-json', fname)
    with open(fname) as f:
      return {
        path: {
          'repository': info['url'],
          'revision': info['rev'],
        }
        for path, info in json.load(f).items()
        if info['rev'] is not None
      }
  except (ValueError, SubprocessFailed):
    return {}
  finally:
    os.remove(fname)

def get_commit_message_footer_map(message):
  """Returns: (dict) A dictionary of commit message footer entries.
  """
  footers = {}

  # Extract the lines in the footer block.
  lines = []
  for line in message.strip().splitlines():
    line = line.strip()
    if len(line) == 0:
      del lines[:]
      continue
    lines.append(line)

  # Parse the footer
  for line in lines:
    m = COMMIT_FOOTER_ENTRY_RE.match(line)
    if not m:
      # If any single line isn't valid, continue anyway for compatibility with
      # Gerrit (which itself uses JGit for this).
      continue
    footers[m.group(1)] = m.group(2).strip()
  return footers


def get_commit_message_footer(message, key):
  """Returns: (str/None) The footer value for 'key', or None if none was found.
  """
  return get_commit_message_footer_map(message).get(key)


# Derived from:
# http://code.activestate.com/recipes/577972-disk-usage/?in=user-4178764
def get_total_disk_space():
  cwd = os.getcwd()
  # Windows is the only platform that doesn't support os.statvfs, so
  # we need to special case this.
  if sys.platform.startswith('win'):
    _, total, free = (ctypes.c_ulonglong(), ctypes.c_ulonglong(), \
                      ctypes.c_ulonglong())
    ret = ctypes.windll.kernel32.GetDiskFreeSpaceExW(cwd, ctypes.byref(_),
                                                     ctypes.byref(total),
                                                     ctypes.byref(free))
    if ret == 0:
      # WinError() will fetch the last error code.
      raise ctypes.WinError()
    return (total.value, free.value)

  else:
    st = os.statvfs(cwd)
    free = st.f_bavail * st.f_frsize
    total = st.f_blocks * st.f_frsize
    return (total, free)


def disk_usage():
  total_disk_space, free_disk_space = get_total_disk_space()
  total_disk_space_gb = int(total_disk_space / (1024 * 1024 * 1024))
  used_disk_space_gb = int(
      (total_disk_space - free_disk_space) / (1024 * 1024 * 1024))
  percent_used = int(used_disk_space_gb * 100 / total_disk_space_gb)
  return (used_disk_space_gb, total_disk_space_gb, percent_used)


def ref_to_remote_ref(ref):
  """Maps refs to equivalent remote ref.

  This maps
    - refs/heads/BRANCH -> refs/remotes/origin/BRANCH
    - refs/branch-heads/BRANCH_HEAD -> refs/remotes/branch-heads/BRANCH_HEAD
    - origin/BRANCH -> refs/remotes/origin/BRANCH
  and leaves other refs unchanged.
  """
  if ref.startswith('refs/heads/'):
    return 'refs/remotes/origin/' + ref[len('refs/heads/'):]
  elif ref.startswith('refs/branch-heads/'):
    return 'refs/remotes/branch-heads/' + ref[len('refs/branch-heads/'):]
  elif ref.startswith('origin/'):
    return 'refs/remotes/' + ref
  else:
    return ref


def get_target_branch_and_revision(solution_name, git_url, revisions):
  solution_name = solution_name.strip('/')
  configured = revisions.get(solution_name) or revisions.get(git_url)

  if configured is None or COMMIT_HASH_RE.match(configured):
    # TODO(crbug.com/1104182): Get the default branch instead of assuming
    # 'main'.
    branch = 'refs/remotes/origin/main'
    revision = configured or 'HEAD'
    return branch, revision
  elif ':' in configured:
    branch, revision = configured.split(':', 1)
  else:
    branch = configured
    revision = 'HEAD'

  if not branch.startswith(('refs/', 'origin/')):
    branch = 'refs/remotes/origin/' + branch
  branch = ref_to_remote_ref(branch)

  return branch, revision


def _has_in_git_cache(revision_sha1, refs, git_cache_dir, url):
  """Returns whether given revision_sha1 is contained in cache of a given repo.
  """
  try:
    mirror_dir = git(
        'cache', 'exists', '--quiet', '--cache-dir', git_cache_dir, url).strip()
    if revision_sha1:
      git('cat-file', '-e', revision_sha1, cwd=mirror_dir)
    # Don't check refspecs.
    filtered_refs = [
        r for r in refs if r not in [BRANCH_HEADS_REFSPEC, TAGS_REFSPEC]
    ]
    for ref in filtered_refs:
      git('cat-file', '-e', ref, cwd=mirror_dir)
    return True
  except SubprocessFailed:
    return False


def is_broken_repo_dir(repo_dir):
  # Treat absence of 'config' as a signal of a partially deleted repo.
  return not path.exists(os.path.join(repo_dir, '.git', 'config'))


def _maybe_break_locks(checkout_path, tries=3):
  """This removes all .lock files from this repo's .git directory.

  In particular, this will cleanup index.lock files, as well as ref lock
  files.
  """
  def attempt():
    git_dir = os.path.join(checkout_path, '.git')
    for dirpath, _, filenames in os.walk(git_dir):
      for filename in filenames:
        if filename.endswith('.lock'):
          to_break = os.path.join(dirpath, filename)
          print('breaking lock: %s' % to_break)
          try:
            os.remove(to_break)
          except OSError as ex:
            print('FAILED to break lock: %s: %s' % (to_break, ex))
            raise

  for _ in range(tries):
    try:
      attempt()
      return
    except Exception:
      pass


def _set_git_config(fn):
  @functools.wraps(fn)
  def wrapper(*args, **kwargs):
    with git_config_if_not_set('user.name', 'chrome-bot'), \
         git_config_if_not_set('user.email', 'chrome-bot@chromium.org'), \
         git_config_if_not_set('fetch.uriprotocols', 'https'):
      return fn(*args, **kwargs)
  return wrapper


def git_checkouts(solutions, revisions, refs, no_fetch_tags, git_cache_dir,
                  cleanup_dir, enforce_fetch):
  build_dir = os.getcwd()
  for sln in solutions:
    sln_dir = path.join(build_dir, sln['name'])
    _git_checkout(sln, sln_dir, revisions, refs, no_fetch_tags, git_cache_dir,
                  cleanup_dir, enforce_fetch)


def _git_checkout(sln, sln_dir, revisions, refs, no_fetch_tags, git_cache_dir,
                  cleanup_dir, enforce_fetch):
  name = sln['name']
  url = sln['url']

  branch, revision = get_target_branch_and_revision(name, url, revisions)
  pin = revision if COMMIT_HASH_RE.match(revision) else None

  populate_cmd = (['cache', 'populate', '-v', '--cache-dir', git_cache_dir, url,
                   '--reset-fetch-config'])
  if no_fetch_tags:
    populate_cmd.extend(['--no-fetch-tags'])
  if pin:
    populate_cmd.extend(['--commit', pin])
  for ref in refs:
    populate_cmd.extend(['--ref', ref])

  # Step 1: populate/refresh cache, if necessary.
  if enforce_fetch or not pin:
    git(*populate_cmd)

  # If cache still doesn't have required pin/refs, try again and fetch pin/refs
  # directly.
  if not _has_in_git_cache(pin, refs, git_cache_dir, url):
    for attempt in range(3):
      git(*populate_cmd)
      if _has_in_git_cache(pin, refs, git_cache_dir, url):
        break
      print('Some required refs/commits are still not present.')
      print('Waiting 60s and trying again.')
      time.sleep(60)

  # Step 2: populate a checkout from local cache. All operations are local.
  mirror_dir = git(
      'cache', 'exists', '--quiet', '--cache-dir', git_cache_dir, url).strip()
  first_try = True
  while True:
    try:
      # If repo deletion was aborted midway, it may have left .git in broken
      # state.
      if path.exists(sln_dir) and is_broken_repo_dir(sln_dir):
        print('Git repo %s appears to be broken, removing it' % sln_dir)
        remove(sln_dir, cleanup_dir)

      # Use "tries=1", since we retry manually in this loop.
      if not path.isdir(sln_dir) or (path.isdir(sln_dir) and sln_dir.endswith(
          ('/.', '\.')) and len(os.listdir(sln_dir)) == 0):
        git('clone', '--no-checkout', '--local', '--shared', mirror_dir,
            sln_dir)
        _git_disable_gc(sln_dir)
      else:
        _git_disable_gc(sln_dir)
        git('remote', 'set-url', 'origin', mirror_dir, cwd=sln_dir)
        git('fetch', 'origin', cwd=sln_dir)
      git('remote', 'set-url', '--push', 'origin', url, cwd=sln_dir)
      if pin:
        git('fetch', 'origin', pin, cwd=sln_dir)
      for ref in refs:
        refspec = '%s:%s' % (ref, ref_to_remote_ref(ref.lstrip('+')))
        git('fetch', 'origin', refspec, cwd=sln_dir)

      # Windows sometimes has trouble deleting files.
      # This can make git commands that rely on locks fail.
      # Try a few times in case Windows has trouble again (and again).
      if sys.platform.startswith('win'):
        _maybe_break_locks(sln_dir, tries=3)

      # Note that the '--' argument is needed to ensure that git treats
      # 'pin or branch' as revision or ref, and not as file/directory which
      # happens to have the exact same name.
      git('checkout', '--force', pin or branch, '--', cwd=sln_dir)
      git('clean', '-dff', cwd=sln_dir)
      return
    except SubprocessFailed as e:
      # Exited abnormally, there's probably something wrong.
      print('Something failed: %s.' % str(e))
      if first_try:
        first_try = False
        # Lets wipe the checkout and try again.
        remove(sln_dir, cleanup_dir)
      else:
        raise


def _git_disable_gc(cwd):
  git('config', 'gc.auto', '0', cwd=cwd)
  git('config', 'gc.autodetach', '0', cwd=cwd)
  git('config', 'gc.autopacklimit', '0', cwd=cwd)


def get_commit_position(git_path, revision='HEAD'):
  """Dumps the 'git' log for a specific revision and parses out the commit
  position.

  If a commit position metadata key is found, its value will be returned.
  """
  # TODO(iannucci): Use git-footers for this.
  git_log = git('log', '--format=%B', '-n1', revision, cwd=git_path)
  footer_map = get_commit_message_footer_map(git_log)

  # Search for commit position metadata
  value = (footer_map.get(COMMIT_POSITION_FOOTER_KEY) or
           footer_map.get(COMMIT_ORIGINAL_POSITION_FOOTER_KEY))
  if value:
    return value
  return None


def parse_got_revision(manifest, got_revision_mapping):
  """Translate git gclient revision mapping to build properties."""
  properties = {}
  manifest = {
      # Make sure path always ends with a single slash.
      '%s/' % path.rstrip('/'): info
      for path, info in manifest.items()
  }
  for property_name, dir_name in got_revision_mapping.items():
    # Make sure dir_name always ends with a single slash.
    dir_name = '%s/' % dir_name.rstrip('/')
    if dir_name not in manifest:
      continue
    info = manifest[dir_name]
    revision = git('rev-parse', 'HEAD', cwd=dir_name).strip()
    commit_position = get_commit_position(dir_name)

    properties[property_name] = revision
    if commit_position:
      properties['%s_cp' % property_name] = commit_position

  return properties


def emit_json(out_file, did_run, **kwargs):
  """Write run information into a JSON file."""
  output = {}
  output.update({'did_run': did_run})
  output.update(kwargs)
  with open(out_file, 'wb') as f:
    f.write(json.dumps(output, sort_keys=True).encode('utf-8'))


@_set_git_config
def ensure_checkout(solutions, revisions, first_sln, target_os, target_os_only,
                    target_cpu, patch_root, patch_refs, gerrit_rebase_patch_ref,
                    no_fetch_tags, refs, git_cache_dir, cleanup_dir,
                    gerrit_reset, enforce_fetch, experiments,
                    download_topics=False):
  # Get a checkout of each solution, without DEPS or hooks.
  # Calling git directly because there is no way to run Gclient without
  # invoking DEPS.
  print('Fetching Git checkout')

  git_checkouts(solutions, revisions, refs, no_fetch_tags, git_cache_dir,
                cleanup_dir, enforce_fetch)

  # Ensure our build/ directory is set up with the correct .gclient file.
  gclient_configure(solutions, target_os, target_os_only, target_cpu,
                    git_cache_dir)

  # We want to pass all non-solution revisions into the gclient sync call.
  solution_dirs = {sln['name'] for sln in solutions}
  gc_revisions = {
      dirname: rev for dirname, rev in revisions.items()
      if dirname not in solution_dirs}
  # Gclient sometimes ignores "unmanaged": "False" in the gclient solution
  # if --revision <anything> is passed (for example, for subrepos).
  # This forces gclient to always treat solutions deps as unmanaged.
  for solution_name in list(solution_dirs):
    gc_revisions[solution_name] = 'unmanaged'

  # Let gclient do the DEPS syncing.
  # The branch-head refspec is a special case because it's possible Chrome
  # src, which contains the branch-head refspecs, is DEPSed in.
  gclient_sync(BRANCH_HEADS_REFSPEC in refs, TAGS_REFSPEC in refs, gc_revisions,
               patch_refs, gerrit_reset, gerrit_rebase_patch_ref,
               download_topics, experiments)


  # Now that gclient_sync has finished, we should revert any .DEPS.git so that
  # presubmit doesn't complain about it being modified.
  if git('ls-files', '.DEPS.git', cwd=first_sln).strip():
    git('checkout', 'HEAD', '--', '.DEPS.git', cwd=first_sln)

  # Reset the deps_file point in the solutions so that hooks get run properly.
  for sln in solutions:
    sln['deps_file'] = sln.get('deps_file', 'DEPS').replace('.DEPS.git', 'DEPS')
  gclient_configure(solutions, target_os, target_os_only, target_cpu,
                    git_cache_dir)


def parse_revisions(revisions, root):
  """Turn a list of revision specs into a nice dictionary.

  We will always return a dict with {root: something}.  By default if root
  is unspecified, or if revisions is [], then revision will be assigned 'HEAD'
  """
  results = {root.strip('/'): 'HEAD'}
  expanded_revisions = []
  for revision in revisions:
    # Allow rev1,rev2,rev3 format.
    # TODO(hinoka): Delete this when webkit switches to recipes.
    expanded_revisions.extend(revision.split(','))
  for revision in expanded_revisions:
    split_revision = revision.split('@', 1)
    if len(split_revision) == 1:
      # This is just a plain revision, set it as the revision for root.
      results[root] = split_revision[0]
    else:
      # This is an alt_root@revision argument.
      current_root, current_rev = split_revision

      parsed_root = urlparse(current_root)
      if parsed_root.scheme in ['http', 'https']:
        # We want to normalize git urls into .git urls.
        normalized_root = 'https://' + parsed_root.netloc + parsed_root.path
        if not normalized_root.endswith('.git'):
          normalized_root += '.git'
      elif parsed_root.scheme:
        print('WARNING: Unrecognized scheme %s, ignoring' % parsed_root.scheme)
        continue
      else:
        # This is probably a local path.
        normalized_root = current_root.strip('/')

      results[normalized_root] = current_rev

  return results


def parse_args():
  parse = optparse.OptionParser()

  parse.add_option('--experiments',
                   help='Comma separated list of experiments to enable')
  parse.add_option('--patch_root', help='Directory to patch on top of.')
  parse.add_option('--patch_ref', dest='patch_refs', action='append', default=[],
                   help='Git repository & ref to apply, as REPO@REF.')
  parse.add_option('--gerrit_no_rebase_patch_ref', action='store_true',
                   help='Bypass rebase of Gerrit patch ref after checkout.')
  parse.add_option('--gerrit_no_reset', action='store_true',
                   help='Bypass calling reset after applying a gerrit ref.')
  parse.add_option('--specs', help='Gcilent spec.')
  parse.add_option('--spec-path', help='Path to a Gcilent spec file.')
  parse.add_option('--revision_mapping_file',
                   help=('Path to a json file of the form '
                         '{"property_name": "path/to/repo/"}'))
  parse.add_option('--revision', action='append', default=[],
                   help='Revision to check out. Can be any form of git ref. '
                        'Can prepend root@<rev> to specify which repository, '
                        'where root is either a filesystem path or git https '
                        'url. To specify Tip of Tree, set rev to HEAD. ')
  parse.add_option(
      '--no_fetch_tags',
      action='store_true',
      help=('Don\'t fetch tags from the server for the git checkout. '
            'This can speed up fetch considerably when '
            'there are many tags.'))
  parse.add_option(
      '--enforce_fetch',
      action='store_true',
      help=('Enforce a new fetch to refresh the git cache, even if the '
            'solution revision passed in already exists in the current '
            'git cache.'))
  parse.add_option(
      '--download_topics',
      action='store_true')

  parse.add_option('--clobber', action='store_true',
                   help='Delete checkout first, always')
  parse.add_option('--output_json',
                   help='Output JSON information into a specified file')
  parse.add_option('--refs', action='append',
                   help='Also fetch this refspec for the main solution(s). '
                        'Eg. +refs/branch-heads/*')
  parse.add_option('--with_branch_heads', action='store_true',
                    help='Always pass --with_branch_heads to gclient.  This '
                          'does the same thing as --refs +refs/branch-heads/*')
  parse.add_option('--with_tags', action='store_true',
                    help='Always pass --with_tags to gclient.  This '
                          'does the same thing as --refs +refs/tags/*')
  parse.add_option('--git-cache-dir', help='Path to git cache directory.')
  parse.add_option('--cleanup-dir',
                   help='Path to a cleanup directory that can be used for '
                        'deferred file cleanup.')

  options, args = parse.parse_args()

  if options.spec_path:
    if options.specs:
      parse.error('At most one of --spec-path and --specs may be specified.')
    with open(options.spec_path, 'r') as fd:
      options.specs = fd.read()

  if not options.output_json:
    parse.error('--output_json is required')

  if not options.git_cache_dir:
    parse.error('--git-cache-dir is required')

  if not options.refs:
    options.refs = []

  if options.with_branch_heads:
    options.refs.append(BRANCH_HEADS_REFSPEC)
    del options.with_branch_heads

  if options.with_tags:
    options.refs.append(TAGS_REFSPEC)
    del options.with_tags

  try:
    if not options.revision_mapping_file:
      parse.error('--revision_mapping_file is required')

    with open(options.revision_mapping_file, 'r') as f:
      options.revision_mapping = json.load(f)
  except Exception as e:
    print(
        'WARNING: Caught exception while parsing revision_mapping*: %s'
        % (str(e),))

  # Because we print CACHE_DIR out into a .gclient file, and then later run
  # eval() on it, backslashes need to be escaped, otherwise "E:\b\build" gets
  # parsed as "E:[\x08][\x08]uild".
  if sys.platform.startswith('win'):
    options.git_cache_dir = options.git_cache_dir.replace('\\', '\\\\')

  return options, args


def prepare(options, git_slns, active):
  """Prepares the target folder before we checkout."""
  dir_names = [sln.get('name') for sln in git_slns if 'name' in sln]
  if options.clobber:
    ensure_no_checkout(dir_names, options.cleanup_dir)
  # Make sure we tell recipes that we didn't run if the script exits here.
  emit_json(options.output_json, did_run=active)

  step_text = '[%dGB/%dGB used (%d%%)]' % disk_usage()
  # The first solution is where the primary DEPS file resides.
  first_sln = dir_names[0]

  # Split all the revision specifications into a nice dict.
  print('Revisions: %s' % options.revision)
  revisions = parse_revisions(options.revision, first_sln)
  print('Fetching Git checkout at %s@%s' % (first_sln, revisions[first_sln]))
  return revisions, step_text


def checkout(options, git_slns, specs, revisions, step_text):
  print('Using Python version: %s' % (sys.version,))
  print('Checking git version...')
  ver = git('version').strip()
  print('Using %s' % ver)

  # Get the epoch of the git cache from a cache epoch marker file. If this file
  # does not exist, create one with the current timestamp.
  cache_epoch_marker_path = os.path.join(options.git_cache_dir, '.cache_epoch')
  if os.path.isfile(cache_epoch_marker_path):
    with open(cache_epoch_marker_path) as f:
      cache_epoch = f.readline().strip()
  else:
    # This ensures the cache path exists. This is a noop if the path already
    # exists. See crbug.com/1448769#c8.
    os.makedirs(options.git_cache_dir, exist_ok=True)
    with open(cache_epoch_marker_path, 'w') as f:
      cache_epoch = int(time.time())
      print(cache_epoch, file=f)

  print('git_cache epoch: {}'.format(datetime.fromtimestamp(int(cache_epoch))))

  try:
    protocol = git('config', '--get', 'protocol.version')
    print('Using git protocol version %s' % protocol)
  except SubprocessFailed as e:
    print('git protocol version is not specified.')

  first_sln = git_slns[0]['name']
  dir_names = [sln.get('name') for sln in git_slns if 'name' in sln]
  dirty_path = '.dirty_bot_checkout'
  if os.path.exists(dirty_path):
    ensure_no_checkout(dir_names, options.cleanup_dir)

  should_create_dirty_file = True
  experiments = []
  if options.experiments:
    experiments = options.experiments.split(',')

  try:
    # Outer try is for catching patch failures and exiting gracefully.
    # Inner try is for catching gclient failures and retrying gracefully.
    try:
      checkout_parameters = dict(
          # First, pass in the base of what we want to check out.
          solutions=git_slns,
          revisions=revisions,
          first_sln=first_sln,

          # Also, target os variables for gclient.
          target_os=specs.get('target_os', []),
          target_os_only=specs.get('target_os_only', False),

          # Also, target cpu variables for gclient.
          target_cpu=specs.get('target_cpu', []),

          # Then, pass in information about how to patch.
          patch_root=options.patch_root,
          patch_refs=options.patch_refs,
          gerrit_rebase_patch_ref=not options.gerrit_no_rebase_patch_ref,
          download_topics=options.download_topics,

          # Control how the fetch step will occur.
          no_fetch_tags=options.no_fetch_tags,
          enforce_fetch=options.enforce_fetch,

          # Finally, extra configurations cleanup dir location.
          refs=options.refs,
          git_cache_dir=options.git_cache_dir,
          cleanup_dir=options.cleanup_dir,
          gerrit_reset=not options.gerrit_no_reset,

          experiments=experiments)
      ensure_checkout(**checkout_parameters)
      should_create_dirty_file = False
    except GclientSyncFailed:
      print('We failed gclient sync, lets delete the checkout and retry.')
      ensure_no_checkout(dir_names, options.cleanup_dir)
      ensure_checkout(**checkout_parameters)
      should_create_dirty_file = False
  except PatchFailed as e:
    # Tell recipes information such as root, got_revision, etc.
    emit_json(options.output_json,
              did_run=True,
              root=first_sln,
              patch_apply_return_code=e.code,
              patch_root=options.patch_root,
              patch_failure=True,
              failed_patch_body=e.output,
              step_text='%s PATCH FAILED' % step_text,
              fixed_revisions=revisions)
    should_create_dirty_file = False
    raise
  finally:
    if should_create_dirty_file:
      with open(dirty_path, 'w') as f:
        # create file, no content
        pass

  # Take care of got_revisions outputs.
  revision_mapping = GOT_REVISION_MAPPINGS.get(git_slns[0]['url'], {})
  if options.revision_mapping:
    revision_mapping.update(options.revision_mapping)

  # If the repo is not in the default GOT_REVISION_MAPPINGS and no
  # revision_mapping were specified on the command line then
  # default to setting 'got_revision' based on the first solution.
  if not revision_mapping:
    revision_mapping['got_revision'] = first_sln

  manifest = create_manifest()
  properties = parse_got_revision(manifest, revision_mapping)

  if not properties:
    # TODO(hinoka): We should probably bail out here, but in the interest
    # of giving mis-configured bots some time to get fixed use a dummy
    # revision here.
    properties = {'got_revision': 'BOT_UPDATE_NO_REV_FOUND'}
    #raise Exception('No got_revision(s) found in gclient output')

  # Add git cache age to the output.properties
  properties['git_cache_epoch'] = cache_epoch

  usage = disk_usage()
  # successfully checked out. remove cleanup_dir to get free disk space.
  if os.path.exists(options.cleanup_dir):
    prev_usage = usage
    print('Removing cleanup_dir %s' % options.cleanup_dir)
    shutil.rmtree(options.cleanup_dir, ignore_errors=True)
    usage = disk_usage()
    print('Release %dGB (%d%%)' %
          (prev_usage[0] - usage[0], prev_usage[2] - usage[2]))

  step_text = step_text + (' -> [%dGB/%dGB used (%d%%)]' % usage)

  # Tell recipes information such as root, got_revision, etc.
  emit_json(options.output_json,
            did_run=True,
            root=first_sln,
            patch_root=options.patch_root,
            step_text=step_text,
            fixed_revisions=revisions,
            properties=properties,
            manifest=manifest)


def print_debug_info():
  print("Debugging info:")
  debug_params = {
    'CURRENT_DIR': path.abspath(os.getcwd()),
    'THIS_DIR': THIS_DIR,
    'DEPOT_TOOLS_DIR': DEPOT_TOOLS_DIR,
  }
  for k, v in sorted(debug_params.items()):
    print("%s: %r" % (k, v))


def main():
  # Get inputs.
  options, _ = parse_args()

  # Check if this script should activate or not.
  active = True

  # Print a helpful message to tell developers what's going on with this step.
  print_debug_info()

  # Parse, manipulate, and print the gclient solutions.
  specs = {}
  exec(options.specs, specs)
  orig_solutions = specs.get('solutions', [])
  git_slns = modify_solutions(orig_solutions)

  solutions_printer(git_slns)

  try:
    # Dun dun dun, the main part of bot_update.
    # gn creates hardlinks during the build. By default, this makes
    # `git reset` overwrite the sources of the hardlinks, which causes
    # unnecessary rebuilds. (See crbug.com/330461#c13 for an explanation.)
    with git_config_if_not_set('core.trustctime', 'false'):
      revisions, step_text = prepare(options, git_slns, active)
      checkout(options, git_slns, specs, revisions, step_text)

  except PatchFailed as e:
    # Return a specific non-zero exit code for patch failure (because it is
    # a failure), but make it different than other failures to distinguish
    # between infra failures (independent from patch author), and patch
    # failures (that patch author can fix). However, PatchFailure due to
    # download patch failure is still an infra problem.
    if e.code == 3:
      # Patch download problem.
      return 87
    # Genuine patch problem.
    return 88


if __name__ == '__main__':
  sys.exit(main())
