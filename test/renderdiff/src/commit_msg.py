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
import re

from utils import execute, ArgParseImpl

RDIFF_UPDATE_GOLDEN_STR = 'RDIFF_BRANCH'

def _parse_commit(commit_str):
  lines = commit_str.split('\n')
  if len(lines) >= 4:
    commit, author, date, _, title, *desc = lines
  else:
    print(commit_str, file=sys.stderr)
    return (
      lines[0],
      lines[1],
      '',
    )
  commit = commit.split(' ')[1]
  title = title.strip()

  desc = [l.strip() for l in desc[1:]]
  while len(desc) > 0 and len(desc[0]) == 0:
    desc = desc[1:]

  return (
    commit,
    title,
    desc
  )

if __name__ == "__main__":
  RE_STR = rf"{RDIFF_UPDATE_GOLDEN_STR}(?:S)?=[\[]?([a-zA-Z0-9,\s\-\/]+)[\]]?"

  parser = ArgParseImpl()
  parser.add_argument('--file', help='A file containing the commit message')
  args, _ = parser.parse_known_args(sys.argv[1:])

  if not args.file:
    msg = sys.stdin.read()
  else:
    with open(args.file, 'r') as f:
      msg = f.read()

  to_update = []
  commit, title, description = _parse_commit(msg)
  for line in description:
    m = re.match(RE_STR, line)
    if not m:
      continue
    print(m.group(1))
    exit(0)

  # Always default to the main branch
  print('main')
