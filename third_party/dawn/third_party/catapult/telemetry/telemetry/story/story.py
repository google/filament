# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import re
import six

from telemetry.story import shared_state as shared_state_module

_next_story_id = 0


_VALID_TAG_RE = re.compile(r'^[\w]+$')


class Story():
  """A class styled on unittest.TestCase for creating story tests.

  Tests should override Run to maybe start the application and perform actions
  on it. To share state between different tests, one can define a
  shared_state which contains hooks that will be called before and
  after multiple stories run and in between runs.

  Args:
    shared_state_class: subclass of telemetry.story.shared_state.SharedState.
    name: string name of this story that can be used for identifying this story
        in results output.
    tags: A list or set of string labels that are used for filtering. See
        story.story_filter for more information.
    is_local: If True, the story does not require network.
    grouping_keys: A dict of grouping keys that will be added to values computed
        on this story.
  """

  def __init__(self, shared_state_class, name='', tags=None,
               is_local=False, make_javascript_deterministic=True,
               grouping_keys=None, platform_specific=False):
    """
    Args:
      make_javascript_deterministic: Whether JavaScript performed on
          the page is made deterministic across multiple runs. This
          requires that the web content is served via Web Page Replay
          to take effect. This setting does not affect stories containing no web
          content or where the HTTP MIME type is not text/html.See also:
          _InjectScripts method in third_party/web-page-replay/httpclient.py.
      platform_specific: Boolean indicating if a separate web page replay
          recording is required on each platform.
    """
    assert issubclass(shared_state_class,
                      shared_state_module.SharedState)
    self._shared_state_class = shared_state_class
    assert name, 'All stories must be named.'
    self._name = name
    self._platform_specific = platform_specific
    global _next_story_id # pylint: disable=global-statement
    self._id = _next_story_id
    _next_story_id += 1
    if tags is None:
      tags = set()
    elif isinstance(tags, list):
      tags = set(tags)
    else:
      assert isinstance(tags, set)
    for t in tags:
      if not _VALID_TAG_RE.match(t):
        raise ValueError(
            'Invalid tag string: %s. Tag can only contain alphanumeric and '
            'underscore characters.' % t)
      if len(t) > 50:
        raise ValueError('Invalid tag string: %s. Tag can have at most 50 '
                         'characters')
    self._tags = tags
    self._is_local = is_local
    self._make_javascript_deterministic = make_javascript_deterministic
    if grouping_keys is None:
      grouping_keys = {}
    else:
      assert isinstance(grouping_keys, dict)
    self._grouping_keys = grouping_keys
    # A cache of the shared state wpr_mode to make it available to a story.
    self.wpr_mode = None
    self._measurements = []

  def AddMeasurement(self, name, unit, samples, description=None):
    """Record an ad-hoc measurement.

    Args:
      name: A string with the name of the measurement (e.g. 'score', 'runtime',
        etc).
      unit: A string specifying the unit used for measurements (e.g. 'ms',
        'count', etc).
      samples: Either a single numeric value or a list of numeric values to
        record as part of this measurement.
      description: An optional string with a short human readable description
        of the measurement.
    """
    # TODO(crbug.com/999484): Ideally, these should be recorded directly into
    # the results object, rather than held on this temporary list. That needs,
    # however, another slight refactor to make the results object available at
    # this point.
    self._measurements.append({'name': name, 'unit': unit, 'samples': samples,
                               'description': description})

  def GetMeasurements(self):
    return self._measurements

  def Run(self, shared_state):
    """Execute the interactions with the applications and/or platforms."""
    raise NotImplementedError

  @property
  def tags(self):
    return self._tags

  @property
  def shared_state_class(self):
    return self._shared_state_class

  @property
  def id(self):
    return self._id

  @property
  def name(self):
    return self._name

  @property
  def grouping_keys(self):
    return self._grouping_keys

  @property
  def name_and_grouping_key_tuple(self):
    return self.name, tuple(six.iteritems(self.grouping_keys))

  def AsDict(self):
    """Converts a story object to a dict suitable for JSON output."""
    d = {
        'id': self._id,
    }
    if self._name:
      d['name'] = self._name
    return d

  @property
  def file_safe_name(self):
    """A version of display_name that's safe to use as a filename.

    The default implementation sanitizes special characters with underscores,
    but it's okay to override it with a more specific implementation in
    subclasses.
    """
    # This fail-safe implementation is safe for subclasses to override.
    return re.sub('[^a-zA-Z0-9]', '_', self.name)

  @property
  def is_local(self):
    """Returns True iff this story does not require network."""
    return self._is_local

  @property
  def serving_dir(self):
    """Returns the absolute path to a directory with hash files to data that
       should be updated from cloud storage, or None if no files need to be
       updated.
    """
    return None

  @property
  def make_javascript_deterministic(self):
    return self._make_javascript_deterministic

  @property
  def platform_specific(self):
    return self._platform_specific

  def GetStoryTagsList(self):
    """Return a list of strings with story tags and grouping keys."""
    return sorted(list(self.tags)) + [
        '%s:%s' % kv for kv in six.iteritems(self.grouping_keys)]

  def GetExtraTracingMetrics(self):
    """Override this to add more TBMv2 metrics to be computed.

    These metrics were originally set up by the benchmark in
    CreateCoreTimelineBasedMeasurementOptions. This method provides the page
    with a way to add more metrics in the case that certain pages need more
    metrics than others. This is reasonable to do if certain pages within
    your benchmark do not provide the
    information needed to calculate various metrics, or if those metrics
    are not important for that page.

    This option only works for TBMv2 metrics.

    You should return a list of the names of the metrics. For example,
    return ['exampleMetric']
    """
    return []

  def WillStartTracing(self, chrome_trace_config):
    """This method provides a way to add/modify tracing configs on the story
    levels. e.g. if you want to add more trace categories for some stories
    you can add them here.
    """
