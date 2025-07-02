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

# This script contains several checks on the current commit, each corresponds to `possible_conditions`
# in the main function below
#   - Direct (non-filamentbot) edits of /docs
#   - Edits to sources of READMEs that are duplicated to /docs
#       or edits to /docs_src sources
#   - Commit message has DOCS_BYPASS
#   - Commit message has DOCS_FORCE
#   - Commit message has DOCS_ALLOW_DIRECT_EDITS

import json
import sys
import os

from utils import execute, ArgParseImpl

CUR_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.join(CUR_DIR, '../../')

def get_edited_files(commit_hash):
  INSERT = '#####?????'
  res, ret = execute(f'git show --name-only --pretty=%b{INSERT} {commit_hash}')
  assert res == 0, f'Failed to get edited filed {res}: ' + ret
  files = []
  _, after = ret.split(INSERT)
  for r in filter(lambda a: len(a) > 0, after.split('\n')):
    if r.startswith('commit'):
      break
    files.append(r)
  return files

# Returns True if there were no direct edits to '/docs'
def check_no_direct_edits(commit_hash, printing=True):
  bad = [f'\t{f}' for f in get_edited_files(commit_hash) if f.startswith('docs/')]
  if printing and len(bad) > 0:
    print(f'Found edits to /docs:\n' + '\n'.join(bad))
  return len(bad) == 0

# Returns True if docs sources have been modified
def check_has_source_edits(commit_hash, printing=True):
  config = {}
  with open(f'{CUR_DIR}/duplicates.json') as config_txt:
    config = json.loads(config_txt.read())
  source_files = set(config.keys())
  edited_files = set(get_edited_files(commit_hash))
  overlap = [f'\t{f}' for f in list(source_files & edited_files)]
  if printing and len(overlap) > 0:
    print(f'Found edited source files:\n' + '\n'.join(overlap))
  return len(overlap) > 0

# Returns true in a given TAG is found in the commit msg
def commit_msg_has_tag(commit_hash, tag, printing=True):
  res, ret = execute(f'git log --pretty=%B {commit_hash}', cwd=ROOT_DIR)
  for l in ret.split('\n'):
    if tag == l.strip():
      if printing:
        print(f'Found tag={tag} in commit message')
      return True
  return False

if __name__ == "__main__":
  parser = ArgParseImpl()

  TAG_DOCS_BYPASS = 'DOCS_BYPASS'
  TAG_DOCS_FORCE = 'DOCS_FORCE'
  TAG_DOCS_ALLOW_DIRECT_EDITS = 'DOCS_ALLOW_DIRECT_EDITS'

  possible_conditions = {
      'no_direct_edits':
          check_no_direct_edits,
      'source_edits':
          check_has_source_edits,
      'commit_docs_bypass':
          lambda h: commit_msg_has_tag(h, TAG_DOCS_BYPASS),
      'commit_docs_force':
          lambda h: commit_msg_has_tag(h, TAG_DOCS_FORCE),
      'commit_docs_allow_direct_edits':
          lambda h: commit_msg_has_tag(h, TAG_DOCS_ALLOW_DIRECT_EDITS),
  }

  possible_str = ', '.join(list(possible_conditions.keys()))
  parser.add_argument(
      '--do-and',
      type=str,
      help=(
          f'A conjunction of boolean conditions. Possible values are={possible_str}. '
          'Negation is done with `-`'
      )
  )
  parser.add_argument(
      '--do-or',
      type=str,
      help=(
          f'A disjunction of boolean conditions. Possible values are={possible_str}. '
          'Negation is done with `-`'
      )
  )

  args, _ = parser.parse_known_args(sys.argv[1:len(sys.argv) - 1])
  commit_hash = sys.argv[-1]

  assert not (args.do_and and args.do_or), "Must not supply both '--do-and' and '--do-or'"
  assert args.do_and or args.do_or, "Must supply argument '--do-and' or '--do-or'"
  conds = args.do_and.split(',') if args.do_and else args.do_or.split(',')
  conds = [c.strip() for c in conds]
  assert all(
    [c in possible_conditions.keys() for c in
     [(c.replace('-', '') if c.startswith('-') else c) for c in conds]]),\
     f"Not all conditions are valid names. Only {possible_str} are accepted"

  res = []
  for cond in conds:
    if cond.startswith('-'):
      f = lambda: not possible_conditions[cond.replace('-', '')]()
    else:
      f = possible_conditions[cond]
    res.append(f(commit_hash))

  if args.do_and:
    exit(0 if all(res) else 1)
  else:
    exit(0 if any(res) else 1)
