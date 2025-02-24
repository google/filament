# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import collections
import six
from six.moves import range
from six.moves import cPickle
import json
import logging
import os
import re
import socket
import time
import six.moves.urllib.request
import six.moves.urllib.parse
import six.moves.urllib.error


PENDING = None
SUCCESS = 0
WARNING = 1
FAILURE = 2
EXCEPTION = 4
SLAVE_LOST = 5


BASE_URL = 'http://build.chromium.org/p'
CACHE_FILE_NAME = 'cache.dat'


StackTraceLine = collections.namedtuple(
    'StackTraceLine', ('file', 'function', 'line', 'source'))


def _FetchData(master, url):
  url = '%s/%s/json/%s' % (BASE_URL, master, url)
  try:
    logging.info('Retrieving %s', url)
    return json.load(six.moves.urllib.request.urlopen(url))
  except (six.moves.urllib.error.HTTPError, socket.error):
    # Could be intermittent; try again.
    try:
      return json.load(six.moves.urllib.request.urlopen(url))
    except (six.moves.urllib.error.HTTPError, socket.error):
      logging.error('Error retrieving URL %s', url)
      raise
  except:
    logging.error('Error retrieving URL %s', url)
    raise


def Builders(master):
  builders = {}

  # Load builders from cache file.
  if os.path.exists(master):
    start_time = time.time()
    for builder_name in os.listdir(master):
      cache_file_path = os.path.join(master, builder_name, CACHE_FILE_NAME)
      if os.path.exists(cache_file_path):
        with open(cache_file_path, 'rb') as cache_file:
          try:
            builders[builder_name] = cPickle.load(cache_file)
          except EOFError:
            logging.error('File is corrupted: %s', cache_file_path)
            raise
    logging.info('Loaded builder caches in %0.2f seconds.',
                 time.time() - start_time)

  return builders


def Update(master, builders):
  # Update builders with latest information.
  builder_data = _FetchData(master, 'builders')
  for builder_name, builder_info in six.iteritems(builder_data):
    if builder_name in builders:
      builders[builder_name].Update(builder_info)
    else:
      builders[builder_name] = Builder(master, builder_name, builder_info)

  return builders


class Builder(object):
  # pylint: disable=too-many-instance-attributes

  def __init__(self, master, name, data):
    self._master = master
    self._name = name

    self.Update(data)

    self._builds = {}

  def __setstate__(self, state):
    self.__dict__ = state  # pylint: disable=attribute-defined-outside-init
    if not hasattr(self, '_builds'):
      self._builds = {}

  def __lt__(self, other):
    return self.name < other.name

  def __str__(self):
    return self.name

  def __getitem__(self, key):
    if not isinstance(key, int):
      raise TypeError('build numbers must be integers, not %s' %
                      type(key).__name__)

    self._FetchBuilds(key)
    return self._builds[key]

  def _FetchBuilds(self, *build_numbers):
    """Download build details, if not already cached.

    Returns:
      A tuple of downloaded build numbers.
    """
    build_numbers = tuple(build_number for build_number in build_numbers
                          if not (build_number in self._builds and
                                  self._builds[build_number].complete))
    if not build_numbers:
      return ()

    for build_number in build_numbers:
      if build_number < 0:
        raise ValueError('Invalid build number: %d' % build_number)

    build_query = six.moves.urllib.parse.urlencode(
        [('select', build) for build in build_numbers])
    url = 'builders/%s/builds/?%s' % (six.moves.urllib.parse.quote(self.name), build_query)
    builds = _FetchData(self.master, url)
    for build_info in six.itervalues(builds):
      self._builds[build_info['number']] = Build(self.master, build_info)

    self._Cache()

    return build_numbers

  def FetchRecentBuilds(self, number_of_builds):
    min_build = max(self.last_build - number_of_builds, -1)
    return self._FetchBuilds(*range(self.last_build, min_build, -1))

  def Update(self, data=None):
    if not data:
      data = _FetchData(self.master, 'builders/%s' % six.moves.urllib.parse.quote(self.name))
    self._state = data['state']
    self._pending_build_count = data['pendingBuilds']
    self._current_builds = tuple(data['currentBuilds'])
    self._cached_builds = tuple(data['cachedBuilds'])
    self._slaves = tuple(data['slaves'])

    self._Cache()

  def _Cache(self):
    cache_dir_path = os.path.join(self.master, self.name)
    if not os.path.exists(cache_dir_path):
      os.makedirs(cache_dir_path)
    cache_file_path = os.path.join(cache_dir_path, CACHE_FILE_NAME)
    with open(cache_file_path, 'wb') as cache_file:
      cPickle.dump(self, cache_file, -1)

  def LastBuilds(self, count):
    min_build = max(self.last_build - count, -1)
    for build_number in range(self.last_build, min_build, -1):
      yield self._builds[build_number]

  @property
  def master(self):
    return self._master

  @property
  def name(self):
    return self._name

  @property
  def state(self):
    return self._state

  @property
  def pending_build_count(self):
    return self._pending_build_count

  @property
  def current_builds(self):
    """List of build numbers currently building.

    There may be multiple entries if there are multiple build slaves."""
    return self._current_builds

  @property
  def cached_builds(self):
    """Builds whose data are visible on the master in increasing order.

    More builds may be available than this."""
    return self._cached_builds

  @property
  def last_build(self):
    """Last completed build."""
    for build_number in reversed(self.cached_builds):
      if build_number not in self.current_builds:
        return build_number
    return None

  @property
  def slaves(self):
    return self._slaves


class Build(object):
  def __init__(self, master, data):
    self._master = master
    self._builder_name = data['builderName']
    self._number = data['number']
    self._complete = not ('currentStep' in data and data['currentStep'])
    self._start_time, self._end_time = data['times']

    self._steps = {
        step_info['name']:
            Step(self._master, self._builder_name, self._number, step_info)
        for step_info in data['steps']
    }

  def __str__(self):
    return str(self.number)

  def __lt__(self, other):
    return self.number < other.number

  @property
  def builder_name(self):
    return self._builder_name

  @property
  def number(self):
    return self._number

  @property
  def complete(self):
    return self._complete

  @property
  def start_time(self):
    return self._start_time

  @property
  def end_time(self):
    return self._end_time

  @property
  def steps(self):
    return self._steps


def _ParseTraceFromLog(log):
  """Search the log for a stack trace and return a structured representation.

  This function supports both default Python-style stacks and Telemetry-style
  stacks. It returns the first stack trace found in the log - sometimes a bug
  leads to a cascade of failures, so the first one is usually the root cause.
  """
  log_iterator = iter(log.splitlines())
  for line in log_iterator:
    if line == 'Traceback (most recent call last):':
      break
  else:
    return (None, None)

  stack_trace = []
  while True:
    line = next(log_iterator)
    match1 = re.match(r'\s*File "(?P<file>.+)", line (?P<line>[0-9]+), '
                      'in (?P<function>.+)', line)
    match2 = re.match(r'\s*(?P<function>.+) at '
                      '(?P<file>.+):(?P<line>[0-9]+)', line)
    match = match1 or match2
    if not match:
      exception = line
      break
    trace_line = match.groupdict()
    # Use the base name, because the path will be different
    # across platforms and configurations.
    file_base_name = trace_line['file'].split('/')[-1].split('\\')[-1]
    source = log_iterator.next().strip()
    stack_trace.append(StackTraceLine(
        file_base_name, trace_line['function'], trace_line['line'], source))

  return tuple(stack_trace), exception


class Step(object):
  # pylint: disable=too-many-instance-attributes

  def __init__(self, master, builder_name, build_number, data):
    self._master = master
    self._builder_name = builder_name
    self._build_number = build_number
    self._name = data['name']
    self._result = data['results'][0]
    self._start_time, self._end_time = data['times']

    self._log_link = None
    self._results_link = None
    for link_name, link_url in data['logs']:
      if link_name == 'stdio':
        self._log_link = link_url + '/text'
      elif link_name == 'json.output':
        self._results_link = link_url + '/text'

    self._log = None
    self._results = None
    self._stack_trace = None

  def __getstate__(self):
    return {
        '_master': self._master,
        '_builder_name': self._builder_name,
        '_build_number': self._build_number,
        '_name': self._name,
        '_result': self._result,
        '_start_time': self._start_time,
        '_end_time': self._end_time,
        '_log_link': self._log_link,
        '_results_link': self._results_link,
    }

  def __setstate__(self, state):
    self.__dict__ = state  # pylint: disable=attribute-defined-outside-init
    self._log = None
    self._results = None
    self._stack_trace = None

  def __str__(self):
    return self.name

  @property
  def name(self):
    return self._name

  @property
  def result(self):
    return self._result

  @property
  def start_time(self):
    return self._start_time

  @property
  def end_time(self):
    return self._end_time

  @property
  def log_link(self):
    return self._log_link

  @property
  def results_link(self):
    return self._results_link

  @property
  def log(self):
    if self._log is None:
      if not self.log_link:
        return None
      cache_file_path = os.path.join(
          self._master, self._builder_name,
          str(self._build_number), self._name, 'log')
      if os.path.exists(cache_file_path):
        # Load cache file, if it exists.
        with open(cache_file_path, 'rb') as cache_file:
          self._log = cache_file.read()
      else:
        # Otherwise, download it.
        logging.info('Retrieving %s', self.log_link)
        try:
          data = six.moves.urllib.request.urlopen(self.log_link).read()
        except (six.moves.urllib.error.HTTPError, socket.error):
          # Could be intermittent; try again.
          try:
            data = six.moves.urllib.request.urlopen(self.log_link).read()
          except (six.moves.urllib.error.HTTPError, socket.error):
            logging.error('Error retrieving URL %s', self.log_link)
            raise
        except:
          logging.error('Error retrieving URL %s', self.log_link)
          raise
        # And cache the newly downloaded data.
        cache_dir_path = os.path.dirname(cache_file_path)
        if not os.path.exists(cache_dir_path):
          os.makedirs(cache_dir_path)
        with open(cache_file_path, 'wb') as cache_file:
          cache_file.write(data)
        self._log = data
    return self._log

  @property
  def results(self):
    if self._results is None:
      if not self.results_link:
        return None
      cache_file_path = os.path.join(
          self._master, self._builder_name,
          str(self._build_number), self._name, 'results')
      if os.path.exists(cache_file_path):
        # Load cache file, if it exists.
        try:
          with open(cache_file_path, 'rb') as cache_file:
            self._results = cPickle.load(cache_file)
        except EOFError:
          os.remove(cache_file_path)
          return self.results
      else:
        # Otherwise, download it.
        logging.info('Retrieving %s', self.results_link)
        try:
          data = json.load(six.moves.urllib.request.urlopen(self.results_link))
        except (six.moves.urllib.error.HTTPError, socket.error):
          # Could be intermittent; try again.
          try:
            data = json.load(six.moves.urllib.request.urlopen(self.results_link))
          except (six.moves.urllib.error.HTTPError, socket.error):
            logging.error('Error retrieving URL %s', self.results_link)
            raise
        except ValueError:
          # If the build had an exception, the results might not be valid.
          data = None
        except:
          logging.error('Error retrieving URL %s', self.results_link)
          raise
        # And cache the newly downloaded data.
        cache_dir_path = os.path.dirname(cache_file_path)
        if not os.path.exists(cache_dir_path):
          os.makedirs(cache_dir_path)
        with open(cache_file_path, 'wb') as cache_file:
          cPickle.dump(data, cache_file, -1)
        self._results = data
    return self._results

  @property
  def stack_trace(self):
    if self._stack_trace is None:
      self._stack_trace = _ParseTraceFromLog(self.log)
    return self._stack_trace
