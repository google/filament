# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest
import re

from telemetry.story import story_filter as story_filter_module
from telemetry.testing import fakes


class StoryFilterInitUnittest(unittest.TestCase):

  def testBadStoryFilterRegexRaises(self):
    with self.assertRaises(re.error):
      story_filter_module.StoryFilter(story_filter='+')

  def testBadStoryFilterExcludeRegexRaises(self):
    with self.assertRaises(re.error):
      story_filter_module.StoryFilter(story_filter_exclude='+')

  def testBadStoryShardArgEnd(self):
    with self.assertRaises(ValueError):
      story_filter_module.StoryFilter(shard_end_index=-1)

  def testMismatchedStoryShardArgEndAndBegin(self):
    with self.assertRaises(ValueError):
      story_filter_module.StoryFilter(
          shard_end_index=2,
          shard_begin_index=3)


class ProcessCommandLineUnittest(unittest.TestCase):

  def testStoryFlagExclusivity(self):
    args = fakes.FakeParsedArgsForStoryFilter(
        story_filter='blah', stories=['aa', 'bb'])
    with self.assertRaises(AssertionError):
      story_filter_module.StoryFilterFactory.ProcessCommandLineArgs(
          parser=None, args=args)

  def testStoryFlagSmoke(self):
    args = fakes.FakeParsedArgsForStoryFilter(stories=['aa', 'bb'])
    story_filter_module.StoryFilterFactory.ProcessCommandLineArgs(
        parser=None, args=args)


class FakeStory():
  def __init__(self, name='fake_story_name', tags=None):
    self.name = name
    self.tags = tags or set()

  def __repr__(self):
    return '<FakeStory: name=%s, tags=%s>' % (self.name, self.tags)


class FilterStoriesUnittest(unittest.TestCase):

  def testStoryFlag(self):
    a = FakeStory('a')
    b = FakeStory('b')
    c = FakeStory('c')
    d = FakeStory('d')
    stories = (a, b, c, d)
    story_filter = story_filter_module.StoryFilter(stories=['a', 'c'])
    output = story_filter.FilterStories(stories)
    self.assertEqual([a, c], output)

  def testStoryFlag_InvalidStory(self):
    a = FakeStory('a')
    stories = (a,)
    story_filter = story_filter_module.StoryFilter(stories=['a', 'c'])
    with self.assertRaises(ValueError):
      story_filter.FilterStories(stories)

  def testNoFilter(self):
    a = FakeStory('a')
    b = FakeStory('b')
    stories = (a, b)
    story_filter = story_filter_module.StoryFilter()
    output = story_filter.FilterStories(stories)
    self.assertEqual(list(stories), output)

  def testSimple(self):
    a = FakeStory('a')
    foo = FakeStory('foo')  # pylint: disable=blacklisted-name
    stories = (a, foo)
    story_filter = story_filter_module.StoryFilter(
        story_filter='foo')
    output = story_filter.FilterStories(stories)
    self.assertEqual([foo], output)

  def testMultimatch(self):
    a = FakeStory('a')
    foo = FakeStory('foo')  # pylint: disable=blacklisted-name
    foobar = FakeStory('foobar')
    stories = (a, foo, foobar)
    story_filter = story_filter_module.StoryFilter(
        story_filter='foo')
    output = story_filter.FilterStories(stories)
    self.assertEqual([foo, foobar], output)

  def testNoMatch(self):
    a = FakeStory('a')
    foo = FakeStory('foo')  # pylint: disable=blacklisted-name
    foobar = FakeStory('foobar')
    stories = (a, foo, foobar)
    story_filter = story_filter_module.StoryFilter(
        story_filter='1234')
    output = story_filter.FilterStories(stories)
    self.assertEqual([], output)

  def testExclude(self):
    a = FakeStory('a')
    foo = FakeStory('foo')  # pylint: disable=blacklisted-name
    foobar = FakeStory('foobar')
    stories = (a, foo, foobar)
    story_filter = story_filter_module.StoryFilter(
        story_filter_exclude='a')
    output = story_filter.FilterStories(stories)
    self.assertEqual([foo], output)

  def testExcludeTakesPriority(self):
    a = FakeStory('a')
    foo = FakeStory('foo')  # pylint: disable=blacklisted-name
    foobar = FakeStory('foobar')
    stories = (a, foo, foobar)
    story_filter = story_filter_module.StoryFilter(
        story_filter='foo',
        story_filter_exclude='bar')
    output = story_filter.FilterStories(stories)
    self.assertEqual([foo], output)

  def testNoTagMatch(self):
    a = FakeStory('a')
    foo = FakeStory('foo')  # pylint: disable=blacklisted-name
    stories = (a, foo)
    story_filter = story_filter_module.StoryFilter(
        story_tag_filter=' x ')
    output = story_filter.FilterStories(stories)
    self.assertEqual([], output)

  def testTagsAllMatch(self):
    a = FakeStory('a', {'1', '2'})
    b = FakeStory('b', {'1', '2'})
    stories = (a, b)
    story_filter = story_filter_module.StoryFilter(
        story_tag_filter='1, 2')
    output = story_filter.FilterStories(stories)
    self.assertEqual(list(stories), output)

  def testExcludetagTakesPriority(self):
    x = FakeStory('x', {'1'})
    y = FakeStory('y', {'1', '2'})
    stories = (x, y)
    story_filter = story_filter_module.StoryFilter(
        story_tag_filter='1',
        story_tag_filter_exclude='2 ')
    output = story_filter.FilterStories(stories)
    self.assertEqual([x], output)

  def testAbridgedStorySetTag(self):
    x = FakeStory('x', {'1'})
    y = FakeStory('y', {'1', '2'})
    stories = (x, y)
    story_filter = story_filter_module.StoryFilter(
        abridged_story_set_tag='2')
    output = story_filter.FilterStories(stories)
    self.assertEqual([y], output)

  def testAbridgeBeforeShardIndexing(self):
    """Test that the abridged story set tag gets applied before indexing.

    Shard maps on the chromium side allow us to distribute runtime evenly across
    shards so that we minimize waterfall cycle time. If we abridge after we
    select indexes then we cannot control how many stories is on each shard.
    """
    x = FakeStory('x', {'t'})
    y = FakeStory('y')
    z = FakeStory('z', {'t'})
    stories = (x, y, z)
    story_filter = story_filter_module.StoryFilter(
        abridged_story_set_tag='t',
        shard_end_index=2)
    output = story_filter.FilterStories(stories)
    self.assertEqual([x, z], output)

  def testStoryTagTakesPriorityOverShard(self):
    x = FakeStory('x')
    y = FakeStory('y')
    z = FakeStory('z', {'t'})
    stories = (x, y, z)
    story_filter = story_filter_module.StoryFilter(
        story_tag_filter='t',
        shard_end_index=1)
    output = story_filter.FilterStories(stories)
    self.assertEqual([z], output)


class FilterStoriesShardIndexUnittest(unittest.TestCase):
  def setUp(self):
    self.s1 = FakeStory('1')
    self.s2 = FakeStory('2')
    self.s3 = FakeStory('3')
    self.s4 = FakeStory('4')
    self.s5 = FakeStory('5')
    self.stories = (self.s1, self.s2, self.s3, self.s4, self.s5)

  def testStoryShardBegin(self):
    story_filter = story_filter_module.StoryFilter(
        shard_begin_index=1)
    output = story_filter.FilterStories(self.stories)
    self.assertEqual([self.s2, self.s3, self.s4, self.s5], output)

  def testStoryShardEnd(self):
    story_filter = story_filter_module.StoryFilter(
        shard_end_index=2)
    output = story_filter.FilterStories(self.stories)
    self.assertEqual([self.s1, self.s2], output)

  def testStoryShardBoth(self):
    story_filter = story_filter_module.StoryFilter(
        shard_begin_index=1,
        shard_end_index=2)
    output = story_filter.FilterStories(self.stories)
    self.assertEqual([self.s2], output)

  def testStoryShardBeginWraps(self):
    story_filter = story_filter_module.StoryFilter(
        shard_begin_index=-1)
    output = story_filter.FilterStories(self.stories)
    self.assertEqual(list(self.stories), output)

  def testStoryIndexRange(self):
    story_filter = story_filter_module.StoryFilter(
        shard_indexes='1-3')
    output = story_filter.FilterStories(self.stories)
    self.assertEqual([self.s2, self.s3], output)

  def testStoryIndexRangeOpenBegin(self):
    story_filter = story_filter_module.StoryFilter(
        shard_indexes='-3')
    output = story_filter.FilterStories(self.stories)
    self.assertEqual([self.s1, self.s2, self.s3], output)

  def testStoryIndexRangeOpenEnd(self):
    story_filter = story_filter_module.StoryFilter(
        shard_indexes='1-')
    output = story_filter.FilterStories(self.stories)
    self.assertEqual([self.s2, self.s3, self.s4, self.s5], output)

  def testStoryIndexRangeSingles(self):
    story_filter = story_filter_module.StoryFilter(
        shard_indexes='1,3')
    output = story_filter.FilterStories(self.stories)
    self.assertEqual([self.s2, self.s4], output)

  def testStoryIndexRangeCombinations(self):
    story_filter = story_filter_module.StoryFilter(
        shard_indexes='0,2-')
    output = story_filter.FilterStories(self.stories)
    self.assertEqual([self.s1, self.s3, self.s4, self.s5], output)

  def testStoryIndexRangeInvalidRange(self):
    with self.assertRaises(ValueError):
      story_filter = story_filter_module.StoryFilter(
          shard_indexes='3-1')
      story_filter.FilterStories(self.stories)

  def testStoryIndexRangeRangeOverLap(self):
    with self.assertRaises(ValueError):
      story_filter = story_filter_module.StoryFilter(
          shard_indexes='0-2,1-3')
      story_filter.FilterStories(self.stories)

  def testStoryIndexRangeOutOfOrder(self):
    with self.assertRaises(ValueError):
      story_filter = story_filter_module.StoryFilter(
          shard_indexes='2,0-2')
      story_filter.FilterStories(self.stories)

  def testStoryShardEndWraps(self):
    """This is needed since benchmarks may change size.

    When they change size, we will not immediately write new
    shard maps for them.
    """
    story_filter = story_filter_module.StoryFilter(
        shard_end_index=5)
    output = story_filter.FilterStories(self.stories)
    self.assertEqual(list(self.stories), output)


class FakeExpectations():
  def __init__(self, stories_to_disable=None):
    self._stories_to_disable = stories_to_disable or []

  def IsStoryDisabled(self, story):
    if story.name in self._stories_to_disable:
      return 'fake reason'
    return ''


class ShouldSkipUnittest(unittest.TestCase):

  def testRunDisabledStories_DisabledStory(self):
    story = FakeStory()
    expectations = FakeExpectations(stories_to_disable=[story.name])
    story_filter = story_filter_module.StoryFilter(
        expectations=expectations,
        run_disabled_stories=True)
    self.assertFalse(story_filter.ShouldSkip(story))

  def testRunDisabledStories_EnabledStory(self):
    story = FakeStory()
    expectations = FakeExpectations(stories_to_disable=[])
    story_filter = story_filter_module.StoryFilter(
        expectations=expectations,
        run_disabled_stories=True)
    self.assertFalse(story_filter.ShouldSkip(story))

  def testEnabledStory(self):
    story = FakeStory()
    expectations = FakeExpectations(stories_to_disable=[])
    story_filter = story_filter_module.StoryFilter(
        expectations=expectations,
        run_disabled_stories=False)
    self.assertFalse(story_filter.ShouldSkip(story))

  def testDisabledStory(self):
    story = FakeStory()
    expectations = FakeExpectations(stories_to_disable=[story.name])
    story_filter = story_filter_module.StoryFilter(
        expectations=expectations,
        run_disabled_stories=False)
    self.assertEqual(story_filter.ShouldSkip(story), 'fake reason')

  def testDisabledStory_StoryFlag(self):
    story = FakeStory('a_name')
    expectations = FakeExpectations(stories_to_disable=[story.name])
    story_filter = story_filter_module.StoryFilter(
        expectations=expectations,
        run_disabled_stories=False,
        stories=['a_name'])
    self.assertFalse(story_filter.ShouldSkip(story))
