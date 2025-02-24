# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import unittest

from telemetry import story


# pylint: disable=abstract-method
class SharedStateFoo(story.SharedState):
  pass


class SharedStateBar(story.SharedState):
  pass


class StoryFoo(story.Story):
  def __init__(self, name='', labels=None, grouping_keys=None,
               shared_state_class=SharedStateFoo):
    super().__init__(
        shared_state_class, name, labels, grouping_keys=grouping_keys)


class StorySetFoo(story.StorySet):
  """ StorySetFoo is a story set created for testing purpose. """


class StorySetTest(unittest.TestCase):

  def testStorySetTestName(self):
    self.assertEqual('story_set_unittest', StorySetFoo.Name())

  def testStorySetTestDescription(self):
    self.assertEqual(
        ' StorySetFoo is a story set created for testing purpose. ',
        StorySetFoo.Description())

  def testBaseDir(self):
    story_set = StorySetFoo()
    base_dir = story_set.base_dir
    self.assertTrue(os.path.isdir(base_dir))
    self.assertEqual(base_dir, os.path.dirname(__file__))

  def testFilePath(self):
    story_set = StorySetFoo()
    self.assertEqual(os.path.abspath(__file__).replace('.pyc', '.py'),
                     story_set.file_path)

  def testCloudBucket(self):
    blank_story_set = story.StorySet()
    self.assertEqual(blank_story_set.bucket, None)

    public_story_set = story.StorySet(
        cloud_storage_bucket=story.PUBLIC_BUCKET)
    self.assertEqual(public_story_set.bucket, story.PUBLIC_BUCKET)

    partner_story_set = story.StorySet(
        cloud_storage_bucket=story.PARTNER_BUCKET)
    self.assertEqual(partner_story_set.bucket, story.PARTNER_BUCKET)

    internal_story_set = story.StorySet(
        cloud_storage_bucket=story.INTERNAL_BUCKET)
    self.assertEqual(internal_story_set.bucket, story.INTERNAL_BUCKET)

    with self.assertRaises(ValueError):
      story.StorySet(cloud_storage_bucket='garbage_bucket')

  def testRemoveWithEmptySetRaises(self):
    story_set = story.StorySet()
    foo_story = StoryFoo(name='foo')
    with self.assertRaises(ValueError):
      story_set.RemoveStory(foo_story)

  def testBasicAddRemove(self):
    story_set = story.StorySet()
    foo_story = StoryFoo(name='foo')
    story_set.AddStory(foo_story)
    self.assertEqual([foo_story], story_set.stories)

    story_set.RemoveStory(foo_story)
    self.assertEqual([], story_set.stories)

  def testSharedStateProperty(self):
    story_set = story.StorySet()
    foo_story = StoryFoo(name='foo')

    self.assertIsNone(story_set.shared_state_class)
    story_set.AddStory(foo_story)
    self.assertEqual(SharedStateFoo, story_set.shared_state_class)

  def testAddMixedSharedStatesRaises(self):
    foo_story = StoryFoo(name='foo')
    bar_story = StoryFoo(name='bar', shared_state_class=SharedStateBar)

    story_set = story.StorySet()
    story_set.AddStory(foo_story)
    with self.assertRaises(AssertionError):
      story_set.AddStory(bar_story)

  def testAddDuplicateDisplayNameWithoutGroupingKeysRaises(self):
    story_set = story.StorySet()
    foo_story = StoryFoo(name='foo')

    story_set.AddStory(foo_story)

    with self.assertRaises(AssertionError):
      story_set.AddStory(foo_story)

  def testAddDuplicateDisplayNameWithDifferentGroupingKeys(self):
    story_set = story.StorySet()
    foo_story0 = StoryFoo(name='foo', grouping_keys={
        'bar': 3, 'baz': 4})
    foo_story1 = StoryFoo(name='foo', grouping_keys={
        'bar': 7, 'baz': 8})

    story_set.AddStory(foo_story0)
    with self.assertRaises(AssertionError):
      story_set.AddStory(foo_story1)

  def testAddDuplicateDisplayNameWithSameGroupingKeysRaises(self):
    story_set = story.StorySet()
    foo_story0 = StoryFoo(name='foo', grouping_keys={
        'bar': 3, 'baz': 4})
    foo_story1 = StoryFoo(name='foo', grouping_keys={
        'bar': 3, 'baz': 4})

    story_set.AddStory(foo_story0)

    with self.assertRaises(AssertionError):
      story_set.AddStory(foo_story1)

  def testAddRemoveAddStoryIsStillUnique(self):
    story_set = story.StorySet()
    foo_story = StoryFoo(name='foo', grouping_keys={
        'bar': 3, 'baz': 4})

    story_set.AddStory(foo_story)
    story_set.RemoveStory(foo_story)
    story_set.AddStory(foo_story)
