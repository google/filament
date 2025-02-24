#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Print statistics about the rate of commits to a repository."""

from __future__ import print_function
from __future__ import absolute_import
import datetime
import itertools
import json
import math
import six


_BASE_URL = 'https://chromium.googlesource.com'
# Can be up to 10,000.
_REVISION_COUNT = 10000

_REPOSITORIES = [
    'chromium/src',
    'angle/angle',
    'skia',
    'v8/v8',
]


def Pairwise(iterable):
  """s -> (s0,s1), (s1,s2), (s2, s3), ..."""
  a, b = itertools.tee(iterable)
  next(b, None)
  return six.zip(a, b)


def Percentile(data, percentile):
  """Find a percentile of a list of values.

  Parameters:
    data: A sorted list of values.
    percentile: The percentile to look up, from 0.0 to 1.0.

  Returns:
    The percentile.

  Raises:
    ValueError: If data is empty.
  """
  if not data:
    raise ValueError()

  k = (len(data) - 1) * percentile
  f = math.floor(k)
  c = math.ceil(k)

  if f == c:
    return data[int(k)]
  return data[int(f)] * (c - k) + data[int(c)] * (k - f)


def CommitTimes(repository, revision_count):
  parameters = six.moves.urllib.parse.urlencode((('n', revision_count), ('format', 'JSON')))
  url = '%s/%s/+log?%s' % (_BASE_URL, six.moves.urllib.parse.quote(repository), parameters)
  data = json.loads(''.join(six.moves.urllib.request.urlopen(url).read().splitlines()[1:]))

  commit_times = []
  for revision in data['log']:
    commit_time_string = revision['committer']['time']
    commit_time = datetime.datetime.strptime(
        commit_time_string, '%a %b %d %H:%M:%S %Y')
    commit_times.append(commit_time - datetime.timedelta(hours=7))

  return commit_times


def IsWeekday(time):
  return time.weekday() >= 0 and time.weekday() < 5


def main():
  for repository in _REPOSITORIES:
    commit_times = CommitTimes(repository, _REVISION_COUNT)

    commit_durations = []
    for time1, time2 in Pairwise(commit_times):
      #if not (IsWeekday(time1) and IsWeekday(time2)):
      #  continue
      commit_durations.append((time1 - time2).total_seconds() / 60.)
    commit_durations.sort()

    print('REPOSITORY:', repository)
    print('Start Date:', min(commit_times), 'PDT')
    print('  End Date:', max(commit_times), 'PDT')
    print('  Duration:', max(commit_times) - min(commit_times))
    print('         n:', len(commit_times))

    for p in (0.25, 0.50, 0.90):
      percentile = Percentile(commit_durations, p)
      print('%3d%% commit duration:' % (p * 100), '%6.1fm' % percentile)
    mean = math.fsum(commit_durations) / len(commit_durations)
    print('Mean commit duration:', '%6.1fm' % mean)
    print()


if __name__ == '__main__':
  main()
