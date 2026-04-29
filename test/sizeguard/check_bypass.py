# Copyright (C) 2026 The Android Open Source Project
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

#!/usr/bin/env python3

import subprocess
import sys

def commit_msg_has_tag(commit_hash, tag):
    try:
        result = subprocess.run(
            ['git', 'log', '-n1', '--pretty=%B', commit_hash],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=True,
            text=True
        )
        for line in result.stdout.split('\n'):
            if tag == line.strip():
                return True
        return False
    except subprocess.CalledProcessError as e:
        print(f"Error reading commit message: {e}", file=sys.stderr)
        return False

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: check_bypass.py <commit_hash>", file=sys.stderr)
        sys.exit(1)

    commit_hash = sys.argv[1]
    
    if commit_msg_has_tag(commit_hash, "SIZEGUARD_BYPASS"):
        sys.exit(0)
    else:
        sys.exit(1)
