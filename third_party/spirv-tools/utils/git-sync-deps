#!/usr/bin/env python3
# Copyright 2014 Google Inc.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#    * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#    * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""Parse a DEPS file and git checkout all of the dependencies.

Args:
  An optional list of deps_os values.

Environment Variables:
  GIT_EXECUTABLE: path to "git" binary; if unset, will look for one of
  ['git', 'git.exe', 'git.bat'] in your default path.

  GIT_SYNC_DEPS_PATH: file to get the dependency list from; if unset,
  will use the file ../DEPS relative to this script's directory.

  GIT_SYNC_DEPS_QUIET: if set to non-empty string, suppress messages.

Git Config:
  To disable syncing of a single repository:
      cd path/to/repository
      git config sync-deps.disable true

  To re-enable sync:
      cd path/to/repository
      git config --unset sync-deps.disable
"""


import os
import re
import subprocess
import sys
import threading
from builtins import bytes


def git_executable():
  """Find the git executable.

  Returns:
      A string suitable for passing to subprocess functions, or None.
  """
  envgit = os.environ.get('GIT_EXECUTABLE')
  searchlist = ['git', 'git.exe', 'git.bat']
  if envgit:
    searchlist.insert(0, envgit)
  with open(os.devnull, 'w') as devnull:
    for git in searchlist:
      try:
        subprocess.call([git, '--version'], stdout=devnull)
      except (OSError,):
        continue
      return git
  return None


DEFAULT_DEPS_PATH = os.path.normpath(
  os.path.join(os.path.dirname(__file__), os.pardir, 'DEPS'))


def usage(deps_file_path = None):
  sys.stderr.write(
    'Usage: run to grab dependencies, with optional platform support:\n')
  sys.stderr.write('  %s %s' % (sys.executable, __file__))
  if deps_file_path:
    parsed_deps = parse_file_to_dict(deps_file_path)
    if 'deps_os' in parsed_deps:
      for deps_os in parsed_deps['deps_os']:
        sys.stderr.write(' [%s]' % deps_os)
  sys.stderr.write('\n\n')
  sys.stderr.write(__doc__)


def git_repository_sync_is_disabled(git, directory):
  try:
    disable = subprocess.check_output(
      [git, 'config', 'sync-deps.disable'], cwd=directory)
    return disable.lower().strip() in ['true', '1', 'yes', 'on']
  except subprocess.CalledProcessError:
    return False


def is_git_toplevel(git, directory):
  """Return true iff the directory is the top level of a Git repository.

  Args:
    git (string) the git executable

    directory (string) the path into which the repository
              is expected to be checked out.
  """
  try:
    toplevel = subprocess.check_output(
      [git, 'rev-parse', '--show-toplevel'], cwd=directory).strip()
    return os.path.realpath(bytes(directory, 'utf8')) == os.path.realpath(toplevel)
  except subprocess.CalledProcessError:
    return False


def status(directory, checkoutable):
  def truncate(s, length):
    return s if len(s) <= length else s[:(length - 3)] + '...'
  dlen = 36
  directory = truncate(directory, dlen)
  checkoutable = truncate(checkoutable, 40)
  sys.stdout.write('%-*s @ %s\n' % (dlen, directory, checkoutable))


def git_checkout_to_directory(git, repo, checkoutable, directory, verbose):
  """Checkout (and clone if needed) a Git repository.

  Args:
    git (string) the git executable

    repo (string) the location of the repository, suitable
         for passing to `git clone`.

    checkoutable (string) a tag, branch, or commit, suitable for
                 passing to `git checkout`

    directory (string) the path into which the repository
              should be checked out.

    verbose (boolean)

  Raises an exception if any calls to git fail.
  """
  if not os.path.isdir(directory):
    subprocess.check_call(
      [git, 'clone', '--quiet', repo, directory])

  if not is_git_toplevel(git, directory):
    # if the directory exists, but isn't a git repo, you will modify
    # the parent repostory, which isn't what you want.
    sys.stdout.write('%s\n  IS NOT TOP-LEVEL GIT DIRECTORY.\n' % directory)
    return

  # Check to see if this repo is disabled.  Quick return.
  if git_repository_sync_is_disabled(git, directory):
    sys.stdout.write('%s\n  SYNC IS DISABLED.\n' % directory)
    return

  with open(os.devnull, 'w') as devnull:
    # If this fails, we will fetch before trying again.  Don't spam user
    # with error infomation.
    if 0 == subprocess.call([git, 'checkout', '--quiet', checkoutable],
                            cwd=directory, stderr=devnull):
      # if this succeeds, skip slow `git fetch`.
      if verbose:
        status(directory, checkoutable)  # Success.
      return

  # If the repo has changed, always force use of the correct repo.
  # If origin already points to repo, this is a quick no-op.
  subprocess.check_call(
      [git, 'remote', 'set-url', 'origin', repo], cwd=directory)

  subprocess.check_call([git, 'fetch', '--quiet'], cwd=directory)

  subprocess.check_call([git, 'checkout', '--quiet', checkoutable], cwd=directory)

  if verbose:
    status(directory, checkoutable)  # Success.


def parse_file_to_dict(path):
  dictionary = {}
  contents = open(path).read()
  # Need to convert Var() to vars[], so that the DEPS is actually Python. Var()
  # comes from Autoroller using gclient which has a slightly different DEPS
  # format.
  contents = re.sub(r"Var\((.*?)\)", r"vars[\1]", contents)
  exec(contents, dictionary)
  return dictionary


def git_sync_deps(deps_file_path, command_line_os_requests, verbose):
  """Grab dependencies, with optional platform support.

  Args:
    deps_file_path (string) Path to the DEPS file.

    command_line_os_requests (list of strings) Can be empty list.
        List of strings that should each be a key in the deps_os
        dictionary in the DEPS file.

  Raises git Exceptions.
  """
  git = git_executable()
  assert git

  deps_file_directory = os.path.dirname(deps_file_path)
  deps_file = parse_file_to_dict(deps_file_path)
  dependencies = deps_file['deps'].copy()
  os_specific_dependencies = deps_file.get('deps_os', dict())
  if 'all' in command_line_os_requests:
    for value in list(os_specific_dependencies.values()):
      dependencies.update(value)
  else:
    for os_name in command_line_os_requests:
      # Add OS-specific dependencies
      if os_name in os_specific_dependencies:
        dependencies.update(os_specific_dependencies[os_name])
  for directory in dependencies:
    for other_dir in dependencies:
      if directory.startswith(other_dir + '/'):
        raise Exception('%r is parent of %r' % (other_dir, directory))
  list_of_arg_lists = []
  for directory in sorted(dependencies):
    if '@' in dependencies[directory]:
      repo, checkoutable = dependencies[directory].split('@', 1)
    else:
      raise Exception("please specify commit or tag")

    relative_directory = os.path.join(deps_file_directory, directory)

    list_of_arg_lists.append(
      (git, repo, checkoutable, relative_directory, verbose))

  multithread(git_checkout_to_directory, list_of_arg_lists)

  for directory in deps_file.get('recursedeps', []):
    recursive_path = os.path.join(deps_file_directory, directory, 'DEPS')
    git_sync_deps(recursive_path, command_line_os_requests, verbose)


def multithread(function, list_of_arg_lists):
  # for args in list_of_arg_lists:
  #   function(*args)
  # return
  threads = []
  for args in list_of_arg_lists:
    thread = threading.Thread(None, function, None, args)
    thread.start()
    threads.append(thread)
  for thread in threads:
    thread.join()


def main(argv):
  deps_file_path = os.environ.get('GIT_SYNC_DEPS_PATH', DEFAULT_DEPS_PATH)
  verbose = not bool(os.environ.get('GIT_SYNC_DEPS_QUIET', False))

  if '--help' in argv or '-h' in argv:
    usage(deps_file_path)
    return 1

  git_sync_deps(deps_file_path, argv, verbose)
  # subprocess.check_call(
  #     [sys.executable,
  #      os.path.join(os.path.dirname(deps_file_path), 'bin', 'fetch-gn')])
  return 0


if __name__ == '__main__':
  exit(main(sys.argv[1:]))
