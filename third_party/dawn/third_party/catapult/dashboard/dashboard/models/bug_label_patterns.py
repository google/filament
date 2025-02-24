# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""A Model for holding a mapping of bug labels to sets of tests.

This is used to decide which bug labels should be applied by default
to bugs filed for alerts on particular tests.
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from google.appengine.ext import ndb

from dashboard.common import utils

# String ID for the single BugLabelPatterns entity.
_ID = 'patterns'


class BugLabelPatterns(ndb.Model):
  """A model for storing the mapping of bug labels to test path patterns.

  There should only ever be one BugLabelPatterns entity, and it has a single
  property, which is a dict mapping bug label strings to lists of test path
  pattern strings.
  """
  labels_to_patterns = ndb.JsonProperty(indexed=False)


def _Get():
  """Fetches the single BugLabelPatterns entity."""
  entity = ndb.Key(BugLabelPatterns, _ID).get()
  if entity:
    return entity
  entity = BugLabelPatterns(id=_ID)
  entity.labels_to_patterns = {}
  entity.put()
  return entity


def GetBugLabelPatterns():
  """Returns the dict of bug labels to test path patterns."""
  return _Get().labels_to_patterns


def GetBugLabelsForTest(test):
  """Returns a list of bug labels to be applied to the test."""
  matching = []
  for label, patterns in GetBugLabelPatterns().items():
    for pattern in patterns:
      if utils.TestMatchesPattern(test, pattern):
        matching.append(label)
  return sorted(matching)


def AddBugLabelPattern(label, pattern):
  """Adds the given test path pattern for the given bug label."""
  entity = _Get()
  if label not in entity.labels_to_patterns:
    entity.labels_to_patterns[label] = []
  entity.labels_to_patterns[label].append(pattern)
  entity.put()


def RemoveBugLabel(label):
  """Adds the given test path pattern for the given bug label."""
  entity = _Get()
  if label in entity.labels_to_patterns:
    del entity.labels_to_patterns[label]
  entity.put()
