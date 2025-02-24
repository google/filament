#!/usr/bin/env python
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from . import symbolize_trace

class StackFrameMapTest(unittest.TestCase):

  def assertStackFrame(self, stack_frame_map, frame_id, name, parent_id=None):
    self.assertTrue(frame_id in stack_frame_map.frame_by_id)
    frame = stack_frame_map.frame_by_id[frame_id]
    self.assertEqual(name, frame.name)
    self.assertEqual(parent_id, frame.parent_id)

  def testParseNext(self):
    string_map = symbolize_trace.StringMap()
    stack_frame_map = symbolize_trace.StackFrameMap()

    # Check that ParseNext() actually parses anything.
    stack_frame_map.ParseNext(
        symbolize_trace.Trace.HEAP_DUMP_VERSION_1,
        [
            {'id': 1, 'name_sid': string_map.AddString('main')},
            {'id': 45, 'name_sid': string_map.AddString('foo'), 'parent': 1},
        ],
        string_map)
    self.assertStackFrame(stack_frame_map, 1, 'main')
    self.assertStackFrame(stack_frame_map, 45, 'foo', parent_id=1)

    # Check that ParseNext() retains all previously parsed frames.
    stack_frame_map.ParseNext(
        symbolize_trace.Trace.HEAP_DUMP_VERSION_1,
        [
            {'id': 33, 'name_sid': string_map.AddString('bar')},
        ],
        string_map)
    self.assertStackFrame(stack_frame_map, 1, 'main')
    self.assertStackFrame(stack_frame_map, 45, 'foo', parent_id=1)
    self.assertStackFrame(stack_frame_map, 33, 'bar')

  def testParseNextLegacy(self):
    stack_frame_map = symbolize_trace.StackFrameMap()

    stack_frame_map.ParseNext(
        symbolize_trace.Trace.HEAP_DUMP_VERSION_LEGACY,
        {
            1: {'name': 'main'},
            45: {'name': 'foo', 'parent': 1},
        },
        string_map=None)
    self.assertStackFrame(stack_frame_map, 1, 'main')
    self.assertStackFrame(stack_frame_map, 45, 'foo', parent_id=1)

    # When parsing legacy format, ParseNext() is expected to be called once.
    self.assertRaises(
        Exception,
        stack_frame_map.ParseNext,
        symbolize_trace.Trace.HEAP_DUMP_VERSION_LEGACY,
        {
            33: {'name': 'bar'},
        },
        string_map=None)


if __name__ == '__main__':
  unittest.main()
