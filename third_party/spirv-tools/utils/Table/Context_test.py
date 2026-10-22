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
from . Context import Context
from . IndexRange import IndexRange
from . StringList import StringList

class TestCreate(unittest.TestCase):
  def test_creation(self) -> None:
    x = Context()
    self.assertIsInstance(x.string_total_len, int)
    self.assertIsInstance(x.string_buffer, list)
    self.assertIsInstance(x.strings, dict)
    self.assertEqual(x.string_total_len, 0)
    self.assertEqual(x.string_buffer, [])
    self.assertEqual(x.strings, {})

class TestString(unittest.TestCase):
  def test_AddString_new(self) -> None:
    x = Context()
    abc_ir = x.AddString("abc")
    self.assertEqual(abc_ir, IndexRange(0,4))
    self.assertEqual(x.string_total_len, 4)
    self.assertEqual(x.string_buffer, ["abc"])
    self.assertEqual(x.strings, {"abc": IndexRange(0,4)})

    qz_ir = x.AddString("qz")
    self.assertEqual(qz_ir, IndexRange(4,3))
    self.assertEqual(x.string_total_len, 7)
    self.assertEqual(x.string_buffer, ["abc", "qz"])
    self.assertEqual(x.strings, {"abc": IndexRange(0,4), "qz": IndexRange(4,3)})

    empty_ir = x.AddString("")
    self.assertEqual(empty_ir, IndexRange(7,1))
    self.assertEqual(x.string_total_len, 8)
    self.assertEqual(x.string_buffer, ["abc", "qz", ""])
    self.assertEqual(x.strings, {"abc": IndexRange(0,4), "qz": IndexRange(4,3), "": IndexRange(7,1)})

  def test_AddString_idempotent(self) -> None:
    x = Context()
    abc_ir = x.AddString("abc")
    self.assertEqual(abc_ir, IndexRange(0,4))
    self.assertEqual(x.string_total_len, 4)
    self.assertEqual(x.string_buffer, ["abc"])
    self.assertEqual(x.strings, {"abc": IndexRange(0,4)})

    abc_ir = x.AddString("abc")
    self.assertEqual(abc_ir, IndexRange(0,4))
    self.assertEqual(x.string_total_len, 4)
    self.assertEqual(x.string_buffer, ["abc"])
    self.assertEqual(x.strings, {"abc": IndexRange(0,4)})

class TestStringList(unittest.TestCase):
  def test_AddStringList_empty(self) -> None:
    x = Context()
    x_ir = x.AddStringList('x', [])
    self.assertEqual(x_ir, IndexRange(0,0))
    self.assertEqual(x.string_buffer, [])
    self.assertEqual(x.range_buffer, { 'x': [] })
    self.assertEqual(x.ranges, {'x': {StringList([]): IndexRange(0,0)}})

  def test_AddgStringList_nonempty(self) -> None:
    x = Context()
    x_ir = x.AddStringList('x', ["abc", "def"])

    self.assertEqual(x_ir, IndexRange(0,2))
    self.assertEqual(x.range_buffer, {'x': [IndexRange(0,4), IndexRange(4,4)]})
    self.assertEqual(x.ranges, {'x': {StringList(['abc','def']): IndexRange(0,2)}})

  def test_AddgStringList_nonempty_idempotent(self) -> None:
    x = Context()
    x_ir = x.AddStringList('x', ["abc", "def"])
    y_ir = x.AddStringList('x', ["abc", "def"])

    self.assertEqual(x_ir, IndexRange(0,2))
    self.assertEqual(y_ir, IndexRange(0,2))
    self.assertEqual(x.range_buffer, {'x': [IndexRange(0,4), IndexRange(4,4)]})
    self.assertEqual(x.ranges, {'x': {StringList(['abc','def']): IndexRange(0,2)}})

  def test_AddgStringList_nonempty_does_not_sort(self) -> None:
    x = Context()
    x_ir = x.AddStringList('x', ["abc", "def"])
    y_ir = x.AddStringList('x', ["def", "abc"])

    self.assertEqual(x_ir, IndexRange(0,2))
    self.assertEqual(y_ir, IndexRange(2,2))
    self.assertEqual(x.range_buffer, {'x': [IndexRange(0,4),
                                            IndexRange(4,4),
                                            IndexRange(4,4),
                                            IndexRange(0,4)]})
    self.assertEqual(x.ranges, {'x':{StringList(['abc','def']): IndexRange(0,2),
                                     StringList(['def','abc']): IndexRange(2,2)}})

  def test_AddgStringList_separate_by_kind(self) -> None:
    x = Context()
    x_ir = x.AddStringList('x', ["abc", "def"])
    y_ir = x.AddStringList('y', ["ghi", "abc"])

    self.assertEqual(x_ir, IndexRange(0,2))
    self.assertEqual(y_ir, IndexRange(0,2))
    self.assertEqual(x.range_buffer,
                     {'x': [IndexRange(0,4), IndexRange(4,4)],
                      'y': [IndexRange(8,4), IndexRange(0,4)]})
    self.assertEqual(x.ranges, {'x': {StringList(['abc','def']): IndexRange(0,2)},
                                'y': {StringList(['ghi','abc']): IndexRange(0,2)}})

if __name__ == "__main__":
    unittest.main()
