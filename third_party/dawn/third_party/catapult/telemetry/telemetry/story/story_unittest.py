# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest

from telemetry import story
from telemetry.story import shared_state


# pylint: disable=abstract-method
class SharedStateBar(shared_state.SharedState):
  pass


class StoryFoo(story.Story):
  def __init__(self, name='', tags=None):
    super().__init__(
        SharedStateBar, name, tags)


class StoryTest(unittest.TestCase):
  def testStoriesHaveDifferentIds(self):
    s0 = story.Story(SharedStateBar, 'foo')
    s1 = story.Story(SharedStateBar, 'bar')
    self.assertNotEqual(s0.id, s1.id)

  def testStoryName(self):
    s = StoryFoo('Bar')
    self.assertEqual('Bar', s.name)

  def testStoryFileSafeName(self):
    s = StoryFoo('Foo Bar:Baz~0')
    self.assertEqual('Foo_Bar_Baz_0', s.file_safe_name)

  def testStoryAsDict(self):
    s = story.Story(SharedStateBar, 'Foo')
    s_dict = s.AsDict()
    self.assertEqual(s_dict['id'], s.id)
    self.assertEqual('Foo', s_dict['name'])

  def testMakeJavaScriptDeterministic(self):
    s = story.Story(SharedStateBar, name='story')
    self.assertTrue(s.make_javascript_deterministic)

    s = story.Story(SharedStateBar, make_javascript_deterministic=False,
                    name='story')
    self.assertFalse(s.make_javascript_deterministic)

    s = story.Story(SharedStateBar, make_javascript_deterministic=True,
                    name='story')
    self.assertTrue(s.make_javascript_deterministic)

  def testInvalidTags(self):
    with self.assertRaises(ValueError):
      story.Story(SharedStateBar, tags=['a@'], name='story')
    with self.assertRaises(ValueError):
      story.Story(SharedStateBar, tags=['a:b'], name='story')
    with self.assertRaises(ValueError):
      story.Story(SharedStateBar, tags=['tags with space'], name='story')
    with self.assertRaises(ValueError):
      story.Story(SharedStateBar, tags=['a:b'], name='story')
    with self.assertRaises(ValueError):
      story.Story(SharedStateBar, tags=[''], name='story')
    with self.assertRaises(ValueError):
      story.Story(SharedStateBar, tags=['aaaaaa~1'], name='story')
    with self.assertRaises(ValueError):
      story.Story(SharedStateBar, tags=['tag-with-dash'], name='story')
    with self.assertRaises(ValueError):
      story.Story(SharedStateBar, tags=['a'*51], name='story')


  def testValidTags(self):
    story.Story(SharedStateBar, tags=['abc'], name='story')
    story.Story(SharedStateBar, tags=['a'*50, 'b'*25 + 'a'*25], name='story')
    story.Story(SharedStateBar, tags=['a_b'], name='story')
    story.Story(SharedStateBar, tags=['_1'], name='story')
    story.Story(SharedStateBar, tags=['1honda_2tesla_3airplanes'], name='story')

  def testHasName(self):
    with self.assertRaises(AssertionError):
      story.Story(SharedStateBar)
