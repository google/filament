# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Change Exploration Module

In this module we expose the Speculate function which generates a list of
potential Change instances for exploration in the effort to find candidate
Changes to identify culprits. We use a binary search within a range of changes
to identify potential culprits and expose the list of changes in between
revisions in a commit range.

If we think about the range of changes as a list of revisions, each represented
as a subscripted 'c' in the list below:

  [c[0], c[1], c[2], ..., c[N]]

We can treat a binary search through the range c[0]..c[N] to be a binary tree of
subscripts in the range 0..N as shown below:

                 N1/2
       N1/4                N3/4
  N1/8      N3/8      N5/8      N7/8
                 ....


This is useful when attempting to bisect a potentially large range of revisions
quickly when finding one or more culprit changes for performance regressions.
Being able to speculate levels ahead of the bisection range gives us a means for
trading machine resources to reduce overall time-to-culprits when bisecting
large revision ranges.
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import functools

__all__ = ['Speculate']


def _BinaryInfixTraversal(change_a, change_b, levels, midpoint, callback):
  if levels == 0:
    return

  m = midpoint(change_a, change_b)

  if m is None:
    return

  _BinaryInfixTraversal(change_a, m, levels - 1, midpoint, callback)
  callback(m)
  _BinaryInfixTraversal(m, change_b, levels - 1, midpoint, callback)


def Speculate(changes,
              change_detected,
              on_unknown,
              midpoint,
              levels=1,
              benchmark_arguments=None,
              job_id=None):
  """Speculate on a range of changes.

  This function yields a list of tuples with the following form:

    (insertion index, change)

  Where `insertion index` refers to an index in the `changes` argument. The
  returned list is in insertion order with the property that if applied in the
  given order to the `changes` list that the resulting `changes` list is in a
  valid relative ordering of revisions to explore.

  Arguments:
  - changes: a list of Change instances.
  - change_detected: a predicate returning True whether we can detect a change
    between two Change instances, None if the result is inconclusive.
  - on_unknown: a callable invoked when change_detected returns None
    (or is inconclusive) taking both changes.
  - midpoint: a callable invoked returning the midpoint between two changes,
    returning an object of the same type as the arguments or None;
    midpoint(a, b) -> m|None where type(m) == type(a) && type(m) == type(b).
  - levels: the depth of the binary search to explore for speculation; default
    is 1.
  """
  if not changes:
    return []

  additional_changes = []

  def Speculator(change_a_index, change_b_index):
    _, change_a = change_a_index
    index_b, change_b = change_b_index
    if benchmark_arguments is not None and job_id is not None:
      result = change_detected(change_a, change_b, benchmark_arguments, job_id)
    else:
      result = change_detected(change_a, change_b)

    if result is None:
      on_unknown(change_a, change_b)
    elif result:
      accumulated_changes = []

      # This inserter is used to capture the change and the index in `changes`
      # to which the found change is to be inserted.
      def Inserter(change):
        # We only add changes that we've not encountered yet in this traversal.
        if change not in accumulated_changes and change not in changes:
          accumulated_changes.append(change)
          additional_changes.append(tuple([index_b, change]))

      # We explore the space with a binary infix traversal, so that we can get
      # the changes already in insertion order.
      _BinaryInfixTraversal(change_a, change_b, levels, midpoint, Inserter)

    # We return the change_b_index so that the next invocation of this reducer
    # function will always get the second argument provided to this call as the
    # first argument.
    return change_b_index

  # We apply the speculator on each adjacent pair of Change elements in the
  # changes we're provided.
  functools.reduce(Speculator, enumerate(changes))

  # At this point in the function, we have the additional changes in infix
  # traversal order (left, node, right), but we want to return the results in
  # stable insertion order so we reverse this list. This way, we have the
  # elements that will fall in the same insertion index to be inserted at the
  # same index one after another, which will restore the traversal order in the
  # final list.
  #
  # For example:
  #
  #   c = [(0, change2), (0, change1), (0, change0)]
  #
  # When inserted to an empty list by insertion index:
  #
  #   a = []
  #   for index, change in c:
  #     a.insert(index, change)
  #
  # We end up with:
  #
  #   a = [change0, change1, change2]
  return reversed(additional_changes)
