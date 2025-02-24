# Copyright 2022 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from dashboard.models import anomaly
from dashboard.pinpoint.models.change import change
from dashboard.pinpoint.models.change import commit
from dashboard.pinpoint.models.change import patch_test
from dashboard.pinpoint.models import job_bug_update
from dashboard.pinpoint import test


class OrderedDifferenceTest(test.TestCase):

  def testOrderedDifference(self):
    # populate differences
    d = job_bug_update.DifferencesFoundBugUpdateBuilder(metric='metric')
    kind = 'commit'
    commit_dict = {}

    values_a = [0]
    values_b = [1]
    d._differences.append(
        job_bug_update._Difference(kind, commit_dict, values_a, values_b))
    values_a = [0]
    values_b = [-2]
    d._differences.append(
        job_bug_update._Difference(kind, commit_dict, values_a, values_b))
    values_a = [0]
    values_b = [3]
    d._differences.append(
        job_bug_update._Difference(kind, commit_dict, values_a, values_b))

    # check improvement direction UP
    ordered_diff = d._OrderedDifferencesByDelta(anomaly.UP)
    self.assertEqual(ordered_diff[0].MeanDelta(), -2)
    self.assertEqual(ordered_diff[1].MeanDelta(), 1)
    self.assertEqual(ordered_diff[2].MeanDelta(), 3)

    # check improvement direction DOWN
    d._cached_ordered_diffs_by_delta = None  # reset cache
    ordered_diff = d._OrderedDifferencesByDelta(anomaly.DOWN)
    self.assertEqual(ordered_diff[0].MeanDelta(), 3)
    self.assertEqual(ordered_diff[1].MeanDelta(), 1)
    self.assertEqual(ordered_diff[2].MeanDelta(), -2)

    # check improvement direction UNKNOWN
    d._cached_ordered_diffs_by_delta = None  # reset cache
    ordered_diff = d._OrderedDifferencesByDelta(anomaly.UNKNOWN)
    self.assertEqual(ordered_diff[0].MeanDelta(), 3)
    self.assertEqual(ordered_diff[1].MeanDelta(), -2)
    self.assertEqual(ordered_diff[2].MeanDelta(), 1)


class DifferenceTest(test.TestCase):

  def testAddDifference(self):
    # populate differences
    d = job_bug_update.DifferencesFoundBugUpdateBuilder(metric='metric')

    # difference is from commit_dict with kind patch
    commit_dict = {
        'server': 'https://codereview.com',
        'change': 'repo~branch~id',
        'revision': 'abc123'
    }
    d.AddDifference(None, [0.1], [1.1], 'patch', commit_dict)
    self.assertEqual(len(d._differences), 1)
    self.assertEqual(d._differences[0].commit_kind, 'patch')

    # difference is from commit_dict with kind commit
    commit_dict = {'repository': 'catapult', 'git_hash': 'commit_456'}
    d.AddDifference(None, [0.2], [1.2], 'commit', commit_dict)
    self.assertEqual(len(d._differences), 2)
    self.assertEqual(d._differences[1].commit_kind, 'commit')

    # difference is from change with kind patch
    base_commit = commit.Commit('chromium', 'aaa7336c821888839f759c6c0a36b56c')
    dep = commit.Commit('catapult', 'e0a2efbb3d1a81aac3c90041eefec24f066d26ba')
    c = change.Change([base_commit, dep])
    d.AddDifference(c, [0.3], [1.3])
    self.assertEqual(len(d._differences), 3)
    self.assertEqual(d._differences[2].commit_kind, 'commit')

    # difference is from change with kind commit
    c = change.Change([base_commit, dep], patch_test.Patch())
    d.AddDifference(c, [0.4], [1.4])
    self.assertEqual(len(d._differences), 4)
    self.assertEqual(d._differences[3].commit_kind, 'patch')
