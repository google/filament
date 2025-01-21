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

from utils import execute

def get_last_commit():
  res, o = execute('git log -1')
  commit, author, date, _, title, *desc = o.split('\n')

  desc = [l.strip() for l in desc[1:]]
  if len(desc) > 0 and len(desc[0]) == 0:
    desc = desc[1:]

  return (
    commit.split(' ')[1],
    title.strip(),
    desc)

def sanitized_split(line, split_atom='\n'):
  return list(filter(lambda x: len(x) > 0, map(lambda x: x.strip(), line.split(split_atom))))

RDIFF_UPDATE_GOLDEN_STR = 'RDIFF_UPDATE_GOLDEN'

if __name__ == "__main__":
  RE_STR = f'{RDIFF_UPDATE_GOLDEN_STR}(?:S)?=[\[]?([a-zA-Z0-9,\s]+)[\]]?'

  to_update = []
  commit, title, description = get_last_commit()
  for line in description:
    m = re.match(RE_STR, line)
    if not m:
      continue

    to_update += sanitize_split(m.group(1).replace(',', ' '), ' ')
  print(','.join(to_update))
