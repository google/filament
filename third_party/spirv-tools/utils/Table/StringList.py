#!/usr/bin/env python3
# Copyright 2025 Google LLC

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import functools
from typing import List

class StringList(list):
    """
    A hashable ordered list of strings.
    This can be used as the key for a dictionary.
    """
    def __init__(self, strs: List[str]) -> None:
        super().__init__(strs)

    def __hash__(self) -> int: # type: ignore[override]
        return functools.reduce(lambda h, ir: hash((h, hash(ir))), self, 0)
