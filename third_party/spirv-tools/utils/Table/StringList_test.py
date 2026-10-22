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

import unittest
from . StringList import *

class TestStringList(unittest.TestCase):
  def test_creation_empty(self) -> None:
    x = StringList([])
    self.assertEqual(len(x), 0)
    self.assertEqual(x, [])

  def test_creation_nonempty(self) -> None:
    x = StringList(["abc", "def"])
    self.assertEqual(len(x), 2)
    self.assertEqual(x, ["abc", "def"])

  def test_creation_does_not_sort(self) -> None:
    x = StringList(["abc", "def"])
    self.assertEqual(x, ["abc", "def"])
    self.assertNotEqual(x, ["def", "abc"])

  def test_equality(self) -> None:
    x = StringList(["abc", "def"])
    y = StringList(["abc", "def"])
    z = StringList(["abc", "ef"])
    self.assertEqual(x, x)
    self.assertEqual(x, y)
    self.assertNotEqual(x, z)

  def test_hash_heuristic(self) -> None:
    x = StringList(["abc", "def"])
    y = StringList(["abc", "def"])
    z = StringList(["abc", "df"])
    self.assertEqual(hash(x), hash(x))
    self.assertEqual(hash(x), hash(y))
    self.assertNotEqual(hash(x), hash(z))

if __name__ == "__main__":
    unittest.main()
