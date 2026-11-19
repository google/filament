# Copyright (C) 2025 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import shlex
import shutil
import re
import sys
import tempfile

from utils import execute, ArgParseImpl, mkdir_p, renderer_platform

GOLDENS_DIR = 'renderdiff'

ACCESS_TYPE_TOKEN = 'token'
ACCESS_TYPE_SSH = 'ssh'
ACCESS_TYPE_READ_ONLY = 'read-only'

def _read_git_config(curdir):
  with open(os.path.join(curdir, './.git/config'), 'r') as f:
    return f.read()

def _write_git_config(curdir, config_str):
  with open(os.path.join(curdir, './.git/config'), 'w') as f:
    return f.write(config_str)

class GoldenManager:
  def __init__(self, working_dir, access_type=ACCESS_TYPE_READ_ONLY, access_token=None,
               branch=None):
    self.working_dir_ = working_dir
    self.access_token_ = access_token
    self.access_type_ = access_type
    self.branch_ = branch
    assert os.path.isdir(self.working_dir_),\
        f"working directory {self.working_dir_} does not exist"
    self._prepare()

  def _assets_dir(self):
    return os.path.join(self.working_dir_, "filament-assets")

  # Returns the directory containing the goldens
  def directory(self):
    return os.path.join(self._assets_dir(), GOLDENS_DIR)

  def _get_repo_url(self):
    protocol = ''
    protocol_separator = ''
    if self.access_type_ == ACCESS_TYPE_SSH:
      protocol = 'git@'
      protocol_separator = ':'
    else:
      protocol = 'https://' + \
        (f'x-access-token:{self.access_token_}@' if self.access_token_ else '')
      protocol_separator = '/'
    return f'{protocol}github.com{protocol_separator}google/filament-assets.git'

  def _prepare(self):
    assets_dir = self._assets_dir()
    if not os.path.exists(assets_dir):
      if self.access_type_ == ACCESS_TYPE_READ_ONLY and self.branch_:
        # Read-only comparisons only need the target branch's renderdiff/ tree, not years of
        # historical binary .tif commits or the sizeguard/ directory in filament-assets.
        execute(
            f'git clone --depth=1 --filter=blob:none --sparse --single-branch '
            f'--branch {shlex.quote(self.branch_)} {self._get_repo_url()}',
            cwd=self.working_dir_,
            capture_output=False,
            raise_errors=True
        )
        self._git_exec(f'sparse-checkout set {GOLDENS_DIR}', raise_errors=True)
      else:
        execute(
            f'git clone {self._get_repo_url()}',
            cwd=self.working_dir_,
            capture_output=False
        )
    else:
      if self.access_type_ == ACCESS_TYPE_SSH:
        config = _read_git_config(self._assets_dir())
        https_url = r'https://github\.com\/google\/filament\.git'
        config = re.sub(https_url, self._get_repo_url(), config)
        _write_git_config(self._assets_dir(), config)
      self.update()

  def update(self):
    self._git_exec('fetch')
    self._git_exec('checkout main')
    self._git_exec('rebase')

  def _git_exec(self, cmd, capture_output=False, raise_errors=False):
    return execute(f'git {cmd}', cwd=self._assets_dir(),
                   capture_output=capture_output, raise_errors=raise_errors)

  # Merge a local branch to the main branch.
  #  - `tag` is a hash in the filament repo that this merge is associated with
  #  - `push_to_remote` indicates that the local changes should be pushed to the
  #    remote branch.
  def merge_to_main(self, branch, tag, push_to_remote=False):
    self.update()
    assets_dir = self._assets_dir()

    # Update commit message
    self._git_exec(f'checkout {branch}')
    code, old_commit = execute(f'git log --format=%B -n 1', cwd=assets_dir)
    if tag and len(tag) > 0:
      old_commit += f'\nFILAMENT={tag}'
      with tempfile.NamedTemporaryFile('w', delete=False) as f:
        f.write(old_commit)
        commit_file = f.name
      self._git_exec(f'commit --amend -F {commit_file}')
      os.remove(commit_file)

    # Do the actual merge
    self._git_exec(f'checkout main')
    self._git_exec(f'merge --no-ff --no-edit {branch}')
    if push_to_remote and \
       (self.access_token_ or self.access_type_ == ACCESS_TYPE_SSH):
      self._git_exec(f'push origin main')
      self.update()

  # Create a branch on the local repo by copying the content from a source directory.
  # The change/diff is indicated by the 'updates' and the 'deletes' list,
  #  - Files in 'updates' need to be present in both src_dir and in the repo.
  #  - Files in 'deletes' only need to be present in the repo.
  # If caller wants the changes to be pushed to remote, set the `push_to_remote`
  # value to True.
  def source_from(self, src_dir, commit_msg, branch,
                  updates=[], deletes=[], push_to_remote=False):
    assets_dir = self._assets_dir()
    self._git_exec(f'checkout main')
    # Force create the branch (note will overwrite the old branch)
    self._git_exec(f'switch -C {branch}')
    rdiff_dir = os.path.join(assets_dir, GOLDENS_DIR)
    if len(updates) == 0 and len(deletes) == 0:
      shutil.rmtree(rdiff_dir, ignore_errors=True)
      mkdir_p(rdiff_dir)
      shutil.copytree(src_dir, rdiff_dir, dirs_exist_ok=True)
      self._git_exec(f'add {GOLDENS_DIR}')
    else:
      for f in deletes:
        self._git_exec(f'rm {os.path.join(GOLDENS_DIR, f)}')
      for f in updates:
        shutil.copy2(
          os.path.join(src_dir, f),
          os.path.join(rdiff_dir, f))
        self._git_exec(f'add {os.path.join(GOLDENS_DIR, f)}')

    with tempfile.NamedTemporaryFile('w', delete=False) as f:
      f.write(commit_msg)
      tmp_golden_commit_file = f.name
    self._git_exec(f'commit -a -F {tmp_golden_commit_file}')
    os.remove(tmp_golden_commit_file)
    if push_to_remote and \
       (self.access_token_ or self.access_type_ == ACCESS_TYPE_SSH):
      self._git_exec(f'push -f origin {branch}')
      self.update()

  # Download the content of a branch to a directory.
  def download_to(self, dest_dir, branch='main', platform=None):
    if not (self.access_type_ == ACCESS_TYPE_READ_ONLY and self.branch_ == branch):
      self._git_exec(f'checkout {branch}')
    assets_dir = self._assets_dir()
    mkdir_p(dest_dir)
    rdiff_dir = os.path.join(assets_dir, GOLDENS_DIR)
    if not platform:
      shutil.copytree(rdiff_dir, dest_dir, dirs_exist_ok=True)
    else:
      for dirpath, _, filenames in os.walk(rdiff_dir):
        rel_dir = os.path.relpath(dirpath, rdiff_dir)
        target_dir = dest_dir if rel_dir == '.' else os.path.join(dest_dir, rel_dir)
        for fname in filenames:
          fplat = renderer_platform(fname)
          if fplat is not None and fplat != platform:
            continue
          mkdir_p(target_dir)
          shutil.copy2(os.path.join(dirpath, fname), os.path.join(target_dir, fname))
    code, o = self._git_exec(f'rev-parse HEAD', capture_output=True)
    with open(os.path.join(dest_dir, 'GIT_COMMIT_HASH'), 'w') as f:
      f.write(o)

# The main entry point will enable download content of a branch to a directory
if __name__ == "__main__":
  parser = ArgParseImpl()
  parser.add_argument('--branch', type=str, help='Branch of the golden repo', default='main')
  parser.add_argument('--platform', type=str,
                      help='Optional platform filter (e.g. desktop or web) for downloaded goldens')
  parser.add_argument('--output', type=str, help='Directory to download to', required=True)

  args, _ = parser.parse_known_args(sys.argv[1:])

  # prepare goldens working directory
  golden_dir = args.output
  assert os.path.isdir(golden_dir),\
    f"Output directory {golden_dir} does not exist"

  # Download the golden repo into the current working directory
  golden_manager = GoldenManager(os.getcwd(), branch=args.branch)
  golden_manager.download_to(golden_dir, branch=args.branch, platform=args.platform)
