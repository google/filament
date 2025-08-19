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

from typing import Any

class IndexRange():
  def __init__(self, first: int, count: int) -> None:
    self.first = first
    self.count = count
    if first < 0:
      raise Exception("invalid arg: first {} must be non-negative".format(first))
    if count < 0:
      raise Exception("invalid arg: count {} must be non-negative".format(count))

  def __eq__(self, other: Any) -> bool:
    return isinstance(other, IndexRange) and self.first == other.first and self.count == other.count

  def __hash__(self) -> int:
    return hash("{} {}".format(self.first, self.count))

  def __str__(self) -> str:
    return "IR({}, {})".format(self.first, self.count)
