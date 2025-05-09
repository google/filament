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

import sys
import os
import glob
import time

from golden_manager import GoldenManager, ACCESS_TYPE_SSH, ACCESS_TYPE_TOKEN
from image_diff import same_image

from utils import execute, ArgParseImpl
from utils import prompt_helper, PROMPT_YES, PROMPT_NO

def line_prompt(prompt, validator=lambda a:True):
  while True:
    res = input(f'{prompt} => ').strip()
    if validator(res):
      return res
  return None

CONFIG_NEW_SRC_DIR = 'goldens_dir'
CONFIG_GOLDENS_BRANCH = 'goldens_branch'
CONFIG_GOLDENS_UPDATES = 'goldens_updates'
CONFIG_GOLDENS_DELETES = 'goldens_deletes'
CONFIG_AUTO_COMMIT = 'auto-commit'
CONFIG_COMMIT_MSG = 'commit_msg'

def _get_current_branch():
  code, res = execute('git branch --show-current')
  return res.strip()

def _file_as_str(fpath):
  with open(fpath, 'r') as f:
    return f.read()

def _do_update(golden_manager, config):
  deletes = config[CONFIG_GOLDENS_DELETES]
  updates = config[CONFIG_GOLDENS_UPDATES]
  if len(deletes) == 0 and len(updates) == 0:
    print('Nothing to update. Exiting...')
    exit(0)

  branch = config[CONFIG_GOLDENS_BRANCH]
  src_dir = config[CONFIG_NEW_SRC_DIR]
  auto_commit = config[CONFIG_AUTO_COMMIT]
  commit_msg = config[CONFIG_COMMIT_MSG]
  golden_manager.source_from(src_dir, commit_msg, branch,
                             updates=updates,
                             deletes=deletes,
                             push_to_remote=auto_commit)

def _get_deletes_updates(update_dir, golden_dir):
  ret_delete = []
  ret_update = []
  for ext in ['tif', 'json']:
    base = set(glob.glob(f'./**/*.{ext}', root_dir=golden_dir, recursive=True))
    new = set(glob.glob(f'./**/*.{ext}', root_dir=update_dir, recursive=True))

    delete = list(base - new)
    update = list(new - base)

    for fpath in base.intersection(new):
      base_fpath = os.path.join(golden_dir, fpath)
      new_fpath = os.path.join(update_dir, fpath)
      if (ext == 'tif' and not same_image(new_fpath, base_fpath)) or \
         (ext == 'json' and _file_as_str(new_fpath) != _file_as_str(base_fpath)):
        update.append(fpath)

    ret_update += update
    ret_delete += delete

  return ret_delete, ret_update

# Ask a bunch of questions to gather the configuration for the update
def _interactive_mode(base_golden_dir):
  config = {}
  cur_branch = _get_current_branch()
  if prompt_helper(
          f'Generate the new goldens from your local ' \
          f'Filament branch? (branch={cur_branch})') == PROMPT_YES:
    code, res = execute('bash ./test/renderdiff/test.sh generate',
            capture_output=False)
    if code != 0:
      print('Failed to generate new goldens')
      exit(1)
    config[CONFIG_NEW_SRC_DIR] = os.path.join(os.getcwd(), './out/renderdiff_tests/')
  else:
    def validator(src_dir):
      if not os.path.exists(src_dir):
        print(f'Cannot find directory {src_dir}. Please try again.')
        return False
      return True

    config[CONFIG_NEW_SRC_DIR] = line_prompt(
        'Please provide path of directory containing new goldens',
        validator)

  if prompt_helper(f'Update new goldens to branch={cur_branch}? '
                   '(Note that this refers to a branch in the goldens repo, not the Filament repo.)'
                   ) == PROMPT_YES:
    config[CONFIG_GOLDENS_BRANCH] = cur_branch
  else:
    config[CONFIG_GOLDENS_BRANCH] = line_prompt('Please provide new branch name for update')

  if prompt_helper(f'Provide a commit message?') == PROMPT_YES:
    config[CONFIG_COMMIT_MSG] = line_prompt('Message:')
  else:
    config[CONFIG_COMMIT_MSG] = f'Update {time.time()} from filament ({cur_branch})'

  new_golden_dir = config[CONFIG_NEW_SRC_DIR]
  deletes, updates = _get_deletes_updates(new_golden_dir, base_golden_dir)
  if len(deletes) + len(updates) != 0:
    prompt = 'The following files will be changed:\n' + \
        '\n'.join([f'  {fname} [delete]' for fname in deletes]) + \
        '\n'.join([f'  {fname} [update]' for fname in updates]) + \
        '\nIs that ok?'
    if prompt_helper(prompt) == PROMPT_YES:
      config[CONFIG_GOLDENS_DELETES] = deletes
      config[CONFIG_GOLDENS_UPDATES] = updates
    else:
      # We cannot proceed if user answered no.
      exit(1)
  else:
    config[CONFIG_GOLDENS_DELETES] = []
    config[CONFIG_GOLDENS_UPDATES] = []

  config[CONFIG_AUTO_COMMIT] = \
      prompt_helper(f'Commit golden repo changes to remote?') == PROMPT_YES
  return config

if __name__ == "__main__":
  parser = ArgParseImpl()
  parser.add_argument('--branch', help='Branch of the golden repo to write to')
  parser.add_argument('--source', help='Directory containing the new goldens')
  parser.add_argument('--commit-msg', help='Message for the commit to the golden repo')
  parser.add_argument('--golden-repo-token', help='Access token for the golden repo')

  args, _ = parser.parse_known_args(sys.argv[1:])
  config = {}
  golden_manager = GoldenManager(
      os.getcwd(),
      access_type=ACCESS_TYPE_SSH if not args.golden_repo_token else ACCESS_TYPE_TOKEN,
      access_token=args.golden_repo_token
  )
  base_golden_dir = golden_manager.directory()
  if args.branch and args.source and args.commit_msg:
    assert os.path.exists(args.source), f'{args.source} (--source) directory not found'
    deletes, updates = _get_deletes_updates(args.source, base_golden_dir)
    config = {
        CONFIG_AUTO_COMMIT: True,
        CONFIG_GOLDENS_BRANCH: args.branch,
        CONFIG_NEW_SRC_DIR: args.source,
        CONFIG_GOLDENS_UPDATES: updates,
        CONFIG_GOLDENS_DELETES: deletes,
        CONFIG_COMMIT_MSG: args.commit_msg,
    }
  else:
    config = _interactive_mode(base_golden_dir)
  _do_update(golden_manager, config)
