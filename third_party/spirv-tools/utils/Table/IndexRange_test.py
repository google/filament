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
from . IndexRange import IndexRange

class TestIndexRange(unittest.TestCase):
  def test_creation(self) -> None:
    x: IndexRange = IndexRange(4,5);
    self.assertEqual(x.first, 4)
    self.assertEqual(x.count, 5)

  def test_creation_bad_first(self) -> None:
    self.assertRaises(Exception, IndexRange, -1, 5)

  def test_creation_bad_count(self) -> None:
    self.assertRaises(Exception, IndexRange, 1, -5)

  def test_distinct(self) -> None:
    x = IndexRange(4, 5);
    y = IndexRange(6, 7);
    self.assertNotEqual(x.first, y.first)
    self.assertNotEqual(x.count, y.count)

  def test_equality(self) -> None:
    self.assertEqual(IndexRange(4,5), IndexRange(4,5))
    self.assertNotEqual(IndexRange(4,5), IndexRange(4,7))
    self.assertNotEqual(IndexRange(7,5), IndexRange(4,5))

  def test_hash_heuristic(self) -> None:
    x = hash(IndexRange(4,5))
    y = hash(IndexRange(4,5))
    z = hash(IndexRange(6,7))
    self.assertEqual(x, y)
    self.assertNotEqual(x, z)

if __name__ == "__main__":
    unittest.main()
