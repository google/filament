# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import logging
import re

from telemetry.story import typ_expectations


class _StoryMatcher():
  def __init__(self, pattern):
    self._regex = None
    if pattern:
      try:
        self._regex = re.compile(pattern)
      except:
        # Provide context since the error that re module provides
        # is not user friendly.
        logging.error('We failed to compile the regex "%s"', pattern)
        raise

  def __nonzero__(self):
    return self._regex is not None

  __bool__ = __nonzero__

  def HasMatch(self, story):
    return self and bool(self._regex.search(story.name))


class _StoryTagMatcher():
  def __init__(self, tags_str):
    self._tags = [tag.strip() for tag in tags_str.split(',')
                 ] if tags_str else None

  def __nonzero__(self):
    return self._tags is not None

  __bool__ = __nonzero__

  def HasLabelIn(self, story):
    return self and bool(story.tags.intersection(self._tags))


class StoryFilterFactory():
  """This factory reads static global configuration for a StoryFilter.

  Static global configuration includes commandline flags and ProjectConfig.

  It then provides a way to create a StoryFilter by only providing
  the runtime configuration.
  """

  @classmethod
  def BuildStoryFilter(cls, benchmark_name, platform_tags,
                       abridged_story_set_tag):
    expectations = typ_expectations.StoryExpectations(benchmark_name)
    expectations.SetTags(platform_tags or [])
    if cls._expectations_file and os.path.exists(cls._expectations_file):
      with open(cls._expectations_file) as fh:
        expectations.GetBenchmarkExpectationsFromParser(fh.read())
    if not cls._run_abridged_story_set:
      abridged_story_set_tag = None
    return StoryFilter(
        expectations, abridged_story_set_tag, cls._story_filter,
        cls._story_filter_exclude,
        cls._story_tag_filter, cls._story_tag_filter_exclude,
        cls._shard_begin_index, cls._shard_end_index, cls._run_disabled_stories,
        stories=cls._stories, shard_indexes=cls._shard_indexes)

  @classmethod
  def AddCommandLineArgs(cls, parser):
    group = parser.add_argument_group('User story filtering options')
    group.add_argument(
        '--story-filter',
        help='Use only stories whose names match the given filter regexp.')
    group.add_argument(
        '--story-filter-exclude',
        help='Exclude stories whose names match the given filter regexp.')
    group.add_argument('--story-tag-filter',
                       help='Use only stories that have any of these tags')
    group.add_argument('--story-tag-filter-exclude',
                       help='Exclude stories that have any of these tags')
    common_story_shard_help = (
        'Indices start at 0, and have the same rules as python slices,'
        ' e.g.  [4, 5, 6, 7, 8][0:3] -> [4, 5, 6])')
    group.add_argument(
        '--story-shard-begin-index',
        type=int,
        help=(
            'Beginning index of set of stories to run. If this is ommited, '
            'the starting index will be from the first story in the benchmark' +
            common_story_shard_help))
    group.add_argument(
        '--story-shard-end-index',
        type=int,
        help=('End index of set of stories to run. Value will be '
              'rounded down to the number of stories. Negative values not'
              'allowed. If this is omited, the end index is the final story'
              'of the benchmark. ' + common_story_shard_help))
    group.add_argument(
        '--story-shard-indexes',
        help=('Index ranges of sets of stories to run. (Negative values not '
              'allowed.) Each range can be a single index or a range in '
              '"begin-end" format. E.g., 2,4-6 means stories on index 2, 4, '
              '5 and 6. Ranges should be ordered. ' + common_story_shard_help))
    # This should be renamed to --also-run-disabled-stories.
    group.add_argument('-d',
                       '--also-run-disabled-tests',
                       dest='run_disabled_stories',
                       action='store_true',
                       default=False,
                       help='Ignore expectations.config disabling.')
    group.add_argument(
        '--run-abridged-story-set',
        action='store_true',
        default=None,
        help=('Whether to run the abridged set of stories from the benchmark '
              'instead of the whole set of stories. Note that many benchmarks '
              'do not have an abridged version: for those benchmarks this flag '
              'will have no effect.'))
    group.add_argument(
        '--story',
        action='append',
        dest='stories',
        help=('An exact name of a story to run. These strings should be '
              'the exact values as stored in the name attribute of a story '
              'object. Passing in a story name this way will cause the story '
              'to run even if it is marked as "Skip" in the expectations '
              'config. This name does not include the benchmark name. This '
              'flag can be provided multiple times to chose to run multiple '
              'stories. The story flag is exclusive with other story selection '
              'flags.'))

  @classmethod
  def ProcessCommandLineArgs(cls, parser, args, environment=None):
    del parser
    cls._story_filter = args.story_filter
    cls._story_filter_exclude = args.story_filter_exclude
    cls._story_tag_filter = args.story_tag_filter
    cls._story_tag_filter_exclude = args.story_tag_filter_exclude
    cls._stories = args.stories
    if cls._stories:
      assert args.story_shard_begin_index is None, (
          '--story and --story-shard-begin-index are mutually exclusive.')
      assert args.story_shard_end_index is None, (
          '--story and --story-shard-end-index are mutually exclusive.')
      assert args.story_shard_indexes is None, (
          '--story and --story-shard-indexes are mutually exclusive.')
      assert args.story_filter is None, (
          '--story and --story-filter are mutually exclusive.')
      assert args.story_filter_exclude is None, (
          '--story and --story-filter-exclude are mutually exclusive.')
      assert args.story_tag_filter is None, (
          '--story and --story-tag-filter are mutually exclusive.')
      assert args.story_tag_filter_exclude is None, (
          '--story and --story-tag-filter-exclude are mutually exclusive.')
      assert args.run_abridged_story_set is None, (
          '--story and --run-abridged-story-set are mutually exclusive.')
    if args.story_shard_indexes:
      assert args.story_shard_begin_index is None, (
          '--story-shard-indexes and --story-shard-begin-index are mutually '
          'exclusive.')
      assert args.story_shard_end_index is None, (
          '--story-shard-indexes and --story-shard-end-index are mutually '
          'exclusive.')
    cls._shard_indexes = args.story_shard_indexes
    cls._shard_begin_index = args.story_shard_begin_index or 0
    cls._shard_end_index = args.story_shard_end_index
    if environment and environment.expectations_files:
      assert len(environment.expectations_files) == 1
      cls._expectations_file = environment.expectations_files[0]
    else:
      cls._expectations_file = None
    cls._run_disabled_stories = args.run_disabled_stories
    cls._run_abridged_story_set = args.run_abridged_story_set


class StoryFilter():
  """Logic to decide whether to run, skip, or ignore stories."""

  def __init__(
      self, expectations=None, abridged_story_set_tag=None, story_filter=None,
      story_filter_exclude=None,
      story_tag_filter=None, story_tag_filter_exclude=None,
      shard_begin_index=0, shard_end_index=None, run_disabled_stories=False,
      stories=None, shard_indexes=None):
    self._expectations = expectations
    self._include_regex = _StoryMatcher(story_filter)
    self._exclude_regex = _StoryMatcher(story_filter_exclude)
    self._include_tags = _StoryTagMatcher(story_tag_filter)
    self._exclude_tags = _StoryTagMatcher(story_tag_filter_exclude)
    self._shard_begin_index = shard_begin_index
    self._shard_end_index = shard_end_index
    self._shard_indexes = shard_indexes
    if self._shard_end_index is not None:
      if self._shard_end_index < 0:
        raise ValueError(
            'shard end index cannot be less than 0, since stories are indexed '
            'with positive numbers')
      if (self._shard_begin_index is not None and
          self._shard_end_index <= self._shard_begin_index):
        raise ValueError(
            'shard end index cannot be less than or equal to shard begin index')
    self._run_disabled_stories = run_disabled_stories
    self._abridged_story_set_tag = abridged_story_set_tag
    if stories:
      assert isinstance(stories, list)
    self._stories = stories

  def _ApplyShards(self, stories):
    if self._shard_indexes:
      return [stories[i] for i in self._GetSelectedIndexes(len(stories))]

    if self._shard_begin_index < 0:
      self._shard_begin_index = 0
    if self._shard_end_index is None:
      self._shard_end_index = len(stories)
    return stories[self._shard_begin_index:self._shard_end_index]

  def FilterStories(self, stories):
    """Filters the given stories, using filters provided in the command line.

    This filter causes stories to become completely ignored, and therefore
    they will not show up in test results output.

    Story sharding is done after exclusion and inclusion.

    Args:
      stories: A list of stories.

    Returns:
      A list of remaining stories.
    """
    if self._stories:
      output_stories = []
      output_stories_names = []
      for story in stories:
        if story.name in self._stories:
          output_stories.append(story)
          output_stories_names.append(story.name)
      unmatched_stories = (
          frozenset(self._stories) - frozenset(output_stories_names))
      for story in unmatched_stories:
        raise ValueError('story %s was asked for but does not exist.' % story)
      return output_stories

    if self._abridged_story_set_tag:
      stories = [story for story in stories
                 if self._abridged_story_set_tag in story.tags]
    included_stories = []
    for story in stories:
      # Exclude filters take priority.
      if self._exclude_tags.HasLabelIn(story):
        continue
      if self._exclude_regex.HasMatch(story):
        continue
      if self._include_tags and not self._include_tags.HasLabelIn(story):
        continue
      if self._include_regex and not self._include_regex.HasMatch(story):
        continue
      included_stories.append(story)
    return self._ApplyShards(included_stories)

  def ShouldSkip(self, story, should_log=False):
    """Decides whether a story should be marked skipped.

    The difference between marking a story skipped and simply not running it is
    important for tracking purposes. Officially skipped stories show up in test
    results outputs.

    Args:
      story: A story.Story object.
      should_log: Whether the reason should be logged via logging. Default False
                  to avoid logging the reason multiple times.

    Returns:
      An empty string if the story should *not* be skipped, otherwise a string
      with the reason why the story should be skipped.
    """
    disabled_reason = self._expectations.IsStoryDisabled(story)
    if not disabled_reason:
      return ''

    is_explicitly_named = self._stories and story.name in self._stories
    is_enabled_by_flag = self._run_disabled_stories
    if should_log:
      if is_explicitly_named:
        logging.warning(
            'Running story %s even though it is disabled because '
            'it was specifically asked for by name in the --story '
            'flag.', story.name)
      elif is_enabled_by_flag:
        logging.warning(
            'Force running a disabled story %s even though it was disabled '
            'with the following reason: %s', story.name, disabled_reason)

    if is_explicitly_named or is_enabled_by_flag:
      return ''
    return disabled_reason

  def _GetSelectedIndexes(self, length):
    indexes = []
    index_ranges = self._shard_indexes.split(',')
    cur_end = 0
    for index_range in index_ranges:
      if '-' in index_range:
        begin, end = index_range.split('-')
        begin = int(begin) if begin else 0
        end = int(end) if end else length
        if begin >= end or begin < cur_end or end > length:
          raise ValueError(
              'The index ranges have overlaps or not sorted correctly: %s' %
              self._shard_indexes)
        indexes = indexes + list(range(begin, end))
        cur_end = end
      else:
        index = int(index_range)
        if index < cur_end:
          raise ValueError(
              'The index ranges have overlaps or not sorted correctly: %s' %
              self._shard_indexes)
        indexes.append(index)
        cur_end = index + 1
    return indexes
