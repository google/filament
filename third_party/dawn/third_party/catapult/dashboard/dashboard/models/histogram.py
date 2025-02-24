# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""The datastore models for histograms and diagnostics."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import datetime
import itertools
import json
import logging
import six
import sys

from google.appengine.ext import ndb

from dashboard.common import utils
from dashboard.models import graph_data
from dashboard.models import internal_only_model
from dateutil.relativedelta import relativedelta
from tracing.value.diagnostics import diagnostic as diagnostic_module


class HistogramRevisionRecord(ndb.Model):
  _use_memcache = False
  revision = ndb.IntegerProperty(indexed=True)
  test = ndb.KeyProperty(graph_data.TestMetadata, indexed=True)

  @staticmethod
  @ndb.tasklet
  def GetLatest(suite_key):
    query = HistogramRevisionRecord.query(
        HistogramRevisionRecord.test == suite_key)
    query = query.order(-HistogramRevisionRecord.revision)
    revisions = query.fetch(limit=1)
    if revisions:
      return revisions[0]
    return None

  @staticmethod
  def GetOrCreate(suite_key, revision):
    q = HistogramRevisionRecord.query()
    q = q.filter(HistogramRevisionRecord.test == suite_key)
    q = q.filter(HistogramRevisionRecord.revision == revision)
    records = q.fetch(limit=1)
    if records:
      return records[0]

    h = HistogramRevisionRecord()
    h.test = suite_key
    h.revision = revision
    return h

  @staticmethod
  @ndb.tasklet
  def FindNextRevision(suite_key, revision):
    q = HistogramRevisionRecord.query()
    q = q.filter(HistogramRevisionRecord.test == suite_key)
    q = q.filter(HistogramRevisionRecord.revision > revision)
    q = q.order(graph_data.Row.revision)

    records = yield q.fetch_async(limit=1)
    if records:
      raise ndb.Return(records[0].revision - 1)
    raise ndb.Return(sys.maxsize)

# pylint: disable=invalid-name
class ErrorTolerantJsonProperty(ndb.BlobProperty):
  # Adapted from
  # https://googleapis.dev/python/python-ndb/latest/_modules/google/cloud/ndb/model.html#JsonProperty
  def __init__(self, compressed=None):
    super().__init__(compressed=compressed)

  def _to_base_type(self, value):
    as_str = json.dumps(value, separators=(",", ":"), ensure_ascii=True)
    return as_str.encode("ascii")

  def _from_base_type(self, value):
    try:
      if not isinstance(value, six.text_type):
        value = value.decode("ascii")
      return json.loads(value)
    except ValueError:
      return None
# pylint: enable=invalid-name

class JsonModel(internal_only_model.InternalOnlyModel):
  # Similarly to Row, we don't need to memcache these as we don't expect to
  # access them repeatedly.
  _use_memcache = False

  data = ErrorTolerantJsonProperty(compressed=True)
  test = ndb.KeyProperty(graph_data.TestMetadata)
  internal_only = ndb.BooleanProperty(default=False, indexed=True)


class Histogram(JsonModel):
  # Needed for timeseries queries (e.g. for alerting).
  revision = ndb.IntegerProperty(indexed=True)

  # The time the histogram was added to the dashboard.
  timestamp = ndb.DateTimeProperty(auto_now_add=True, indexed=True)

  @ndb.ComputedProperty
  def expiry(self):  # pylint: disable=invalid-name
    if self.timestamp:
      return self.timestamp + relativedelta(years=3)

    return datetime.datetime.utcnow() + relativedelta(years=3)

def RemoveInvalidSparseDiagnostics(diagnostics):
  # Returns a list of SparseDiagnostics with invalid entries removed
  #   (data == None).
  valid_diagnostics = []
  for d in diagnostics:
    if not d.data:
      logging.error("Found invalid diagnostic %s", d)
    else:
      valid_diagnostics.append(d)
  return valid_diagnostics

class SparseDiagnostic(JsonModel):
  # Need for intersecting range queries.
  name = ndb.StringProperty(indexed=False)
  start_revision = ndb.IntegerProperty(indexed=True)
  end_revision = ndb.IntegerProperty(indexed=True)

  def IsValid(self):
    try:
      diagnostic_module.Diagnostic.FromDict(self.data)
      return True
    except Exception:  # pylint: disable=broad-except
      return False

  def IsDifferent(self, rhs):
    return (diagnostic_module.Diagnostic.FromDict(self.data) !=
            diagnostic_module.Diagnostic.FromDict(rhs.data))

  @classmethod
  @ndb.synctasklet
  def GetMostRecentDataByNamesSync(cls, test_key, diagnostic_names):
    data_by_name = yield cls.GetMostRecentDataByNamesAsync(
        test_key, diagnostic_names)
    raise ndb.Return(data_by_name)

  @classmethod
  @ndb.tasklet
  def GetMostRecentDataByNamesAsync(cls, test_key, diagnostic_names):
    diagnostics = yield cls.query(cls.end_revision == sys.maxsize,
                                  cls.test == test_key).fetch_async()
    diagnostics = RemoveInvalidSparseDiagnostics(diagnostics)
    diagnostics_by_name = {}
    for diagnostic in diagnostics:
      if diagnostic.name not in diagnostic_names:
        continue
      if (diagnostic.name in diagnostics_by_name and diagnostic.start_revision <
          diagnostics_by_name[diagnostic.name].start_revision):
        # TODO(crbug.com/877809) Assert
        continue
      assert diagnostic.data, diagnostic
      diagnostics_by_name[diagnostic.name] = diagnostic
    raise ndb.Return({
        diagnostic.name: diagnostic.data
        for diagnostic in diagnostics_by_name.values()
    })

  @staticmethod
  @ndb.tasklet
  def FixDiagnostics(test_key):
    diagnostics_for_test = yield SparseDiagnostic.query(
        SparseDiagnostic.test == test_key).fetch_async()
    diagnostics_for_test = RemoveInvalidSparseDiagnostics(
        diagnostics_for_test)
    diagnostics_by_name = collections.defaultdict(list)

    for d in diagnostics_for_test:
      diagnostics_by_name[d.name].append(d)

    futures = []

    for diagnostics in diagnostics_by_name.values():
      sorted_diagnostics = sorted(diagnostics, key=lambda d: d.start_revision)
      unique_diagnostics = []

      # Remove any possible duplicates first.
      prev = None
      for d in sorted_diagnostics:
        if not prev:
          unique_diagnostics.append(d)
          prev = d
          continue
        if not prev.IsDifferent(d):
          futures.append(d.key.delete_async())
          continue
        unique_diagnostics.append(d)
        prev = d

      # Now fixup all the start/end revisions.
      for i, diagnostic in enumerate(unique_diagnostics):
        if i == len(unique_diagnostics) - 1:
          diagnostic.end_revision = sys.maxsize
        else:
          diagnostic.end_revision = (
              unique_diagnostics[i + 1].start_revision - 1)

      futures.extend(ndb.put_multi_async(unique_diagnostics))

    yield futures

  @staticmethod
  @ndb.tasklet
  def FindOrInsertDiagnostics(new_entities, test, rev, last_rev):
    """Takes a list of diagnostic entities, test path, revision, and known
    last revision, and inserts the diagnostics into datastore. If they're
    duplicates of existing diagnostics in the same range, a mapping of guids
    from the new ones to the ones in datastore is return.

    Returns:
      dict: A dict of new guids to existing diagnostics.
    """

    if rev >= last_rev:
      # If this is the latest commit, we can go through usual path of checking
      # if the diagnostic changed and updating the previous one.
      results = yield _FindOrInsertDiagnosticsLast(new_entities, test, rev)
    else:
      # This came in out of order, so figure out where to insert the diagnostic
      # and split ranges if necessary.
      results = yield _FindOrInsertDiagnosticsOutOfOrder(
          new_entities, test, rev)
    raise ndb.Return(results)


@ndb.tasklet
def _FindOrInsertDiagnosticsLast(new_entities, test, rev):
  logging.info('Appending diagnostics: %r, revision: %d', new_entities, rev)

  query = SparseDiagnostic.query(
      ndb.AND(SparseDiagnostic.end_revision == sys.maxsize,
              SparseDiagnostic.test == test))
  existing_entities = yield query.fetch_async()
  existing_entities = RemoveInvalidSparseDiagnostics(existing_entities)
  existing_entities = dict((d.name, d) for d in existing_entities)
  entity_futures = []
  new_guids_to_existing_diagnostics = {}

  for new_entity in new_entities:
    existing_entity = existing_entities.get(new_entity.name)
    if existing_entity is not None and existing_entity.IsValid():
      # Case 1: One in datastore, different from new one.
      if existing_entity.IsDifferent(new_entity):
        # Special case, they're overwriting the head value.
        if existing_entity.start_revision == new_entity.start_revision:
          existing_entity.data = new_entity.data
          entity_futures.append(existing_entity.put_async())
        else:
          existing_entity.end_revision = rev - 1
          entity_futures.append(existing_entity.put_async())
          new_entity.start_revision = rev
          new_entity.end_revision = sys.maxsize
          entity_futures.append(new_entity.put_async())
      # Case 2: One in datastore, same as new one.
      else:
        new_guids_to_existing_diagnostics[new_entity.key.id()] = (
            existing_entity.data)
      continue
    # Case 3: Nothing in datastore.
    entity_futures.append(new_entity.put_async())

  yield entity_futures

  raise ndb.Return(new_guids_to_existing_diagnostics)


@ndb.tasklet
def _FindOrInsertNamedDiagnosticsOutOfOrder(new_diagnostic, old_diagnostics,
                                            rev):
  logging.info('Inserting diagnostic out of order. Diagnostic: %r,'
               ' revision: %d', new_diagnostic, rev)

  new_guid = new_diagnostic.key.id()
  guid_mapping = {}

  # It happens when all old diagnostics are invalid and are not added to the
  # list.
  if len(old_diagnostics) == 0:
    guid_mapping[new_guid] = new_diagnostic.data
    yield new_diagnostic.put_async()
    raise ndb.Return(guid_mapping)

  for i in itertools.islice(itertools.count(0), len(old_diagnostics)):
    cur = old_diagnostics[i]

    suite_key = utils.TestKey('/'.join(cur.test.id().split('/')[:3]))

    next_diagnostic = None if i == 0 else old_diagnostics[i - 1]

    # Overall there are 2 major cases to handle. Either you're clobbering an
    # existing diagnostic by uploading right to the start of that diagnostic's
    # range, or you're splitting the range.
    #
    # We treat insertions by assuming that the new diagnostic is valid until the
    # next uploaded commit, since that commit will have had a diagnostic on it
    # which will have been diffed and inserted appropriately at the time.

    # Case 1, clobber the existing diagnostic.
    if rev == cur.start_revision:
      if not cur.IsDifferent(new_diagnostic):
        raise ndb.Return(guid_mapping)

      next_revision = yield HistogramRevisionRecord.FindNextRevision(
          suite_key, rev)

      futures = []

      # There's either a next diagnostic or there isn't, check each separately.
      if not next_diagnostic:
        # If this is the last diagnostic in the range, there are only 2 cases
        # to consider.
        #  1. There are no commits after this diagnostic.
        #  2. There are commits, in which case we need to split the range.

        # 1. There are no commits.
        if next_revision == sys.maxsize:
          cur.data = new_diagnostic.data
          cur.data['guid'] = cur.key.id()

          guid_mapping[new_guid] = cur.data
          new_diagnostic = None

        # 2. There are commits, in which case we need to split the range.
        else:
          new_diagnostic.start_revision = cur.start_revision
          new_diagnostic.end_revision = next_revision

          # Nudge the old diagnostic range forward, that way you don't have to
          # resave the histograms.
          cur.start_revision = next_revision + 1

      # There is another diagnostic range after this one.
      else:
        # If there is another diagnostic range after this, we need to check:
        #  1. Are there any commits between this revision and the next
        #     diagnostic
        #   a. If there are, we need to split the range
        #   b. If there aren't, we just overwrite the diagnostic.

        # 1a. There are commits after this revision before the start of the next
        #     diagnostic.
        if next_revision != next_diagnostic.start_revision - 1:
          new_diagnostic.start_revision = cur.start_revision
          new_diagnostic.end_revision = next_revision

          # Nudge the old diagnostic range forward, that way you don't have to
          # resave the histograms.
          cur.start_revision = next_revision + 1

        # No commits after before next diagnostic, just straight up overwrite.
        else:
          # A. They're not the same.
          if new_diagnostic.IsDifferent(next_diagnostic):
            cur.data = new_diagnostic.data
            cur.data['guid'] = cur.key.id()

            guid_mapping[new_guid] = cur.data
            new_diagnostic = None

          # B. They're the same, in which case we just want to extend the next
          #    diagnostic's range backwards.
          else:
            guid_mapping[new_guid] = next_diagnostic.data
            next_diagnostic.start_revision = cur.start_revision
            new_diagnostic = None
            futures.append(cur.key.delete_async())
            cur = next_diagnostic

      # Finally, check if there was a diagnostic range before this, and wheter
      # it's different than the new one.
      prev_diagnostic = None
      if i + 1 < len(old_diagnostics):
        prev_diagnostic = old_diagnostics[i + 1]

      cur_diagnostic = cur
      if new_diagnostic:
        cur_diagnostic = new_diagnostic

      # Previous diagnostic range is different, so just ignore it.
      if not prev_diagnostic or cur_diagnostic.IsDifferent(prev_diagnostic):
        futures.append(cur.put_async())
        if new_diagnostic:
          futures.append(new_diagnostic.put_async())

      # Previous range is the same, so merge.
      else:
        guid_mapping[new_guid] = prev_diagnostic.data
        prev_diagnostic.end_revision = cur_diagnostic.end_revision

        futures.append(prev_diagnostic.put_async())
        if new_diagnostic:
          new_diagnostic = None
          futures.append(cur.put_async)
        else:
          futures.append(cur.key.delete_async())

      yield futures
      raise ndb.Return(guid_mapping)

    # Case 2, split the range.
    if cur.start_revision < rev <= cur.end_revision:
      if not cur.IsDifferent(new_diagnostic):
        raise ndb.Return(guid_mapping)

      next_revision = yield HistogramRevisionRecord.FindNextRevision(
          suite_key, rev)

      cur.end_revision = rev - 1
      new_diagnostic.start_revision = rev
      new_diagnostic.end_revision = next_revision

      futures = [cur.put_async()]

      # There's either a next diagnostic or there isn't, check each separately.
      if not next_diagnostic:
        # There's no commit after this revision, which means we can extend this
        # diagnostic range to infinity.
        if next_revision == sys.maxsize:
          new_diagnostic.end_revision = next_revision
        else:
          new_diagnostic.end_revision = next_revision

          clone_of_cur = SparseDiagnostic(
              data=cur.data,
              test=cur.test,
              start_revision=next_revision + 1,
              end_revision=sys.maxsize,
              name=cur.name,
              internal_only=cur.internal_only)
          futures.append(clone_of_cur.put_async())

        futures.append(new_diagnostic.put_async())
      else:
        # If there is another diagnostic range after this, we need to check:
        #  1. Are there any commits between this revision and the next
        #     diagnostic
        #   a. If there are, we need to split the range
        #   b. If there aren't, we need to check if the next diagnostic is
        #      any different than the current one, because we may just merge
        #      them together.

        # 1a. There are commits after this revision before the start of the next
        #     diagnostic.
        if next_revision != next_diagnostic.start_revision - 1:
          new_diagnostic.end_revision = next_revision

          clone_of_cur = SparseDiagnostic(
              data=cur.data,
              test=cur.test,
              start_revision=next_revision + 1,
              end_revision=next_diagnostic.start_revision - 1,
              name=cur.name,
              internal_only=cur.internal_only)

          futures.append(clone_of_cur.put_async())
          futures.append(new_diagnostic.put_async())

        # 1b. There aren't commits between this revision and the start of the
        #     next diagnostic range. In this case there are 2 possible outcomes.
        #   A. They're not the same, so just split the range as normal.
        #   B. That the new diagnostic we're inserting and the next one are the
        #      same, in which case they can be merged.
        else:
          # A. They're not the same.
          if new_diagnostic.IsDifferent(next_diagnostic):
            new_diagnostic.end_revision = next_diagnostic.start_revision - 1
            futures.append(new_diagnostic.put_async())

          # B. They're the same, in which case we just want to extend the next
          #    diagnostic's range backwards.
          else:
            guid_mapping[new_guid] = next_diagnostic.data
            next_diagnostic.start_revision = new_diagnostic.start_revision
            new_diagnostic = None
            futures.append(next_diagnostic.put_async())

      yield futures
      raise ndb.Return(guid_mapping)

  # Can't find a spot to put it, which indicates that it should go before any
  # existing diagnostic.
  next_diagnostic = old_diagnostics[-1]

  if not next_diagnostic.IsDifferent(new_diagnostic):
    next_diagnostic.start_revision = rev
    guid_mapping[new_guid] = next_diagnostic.data
    yield next_diagnostic.put_async()
    raise ndb.Return(guid_mapping)

  new_diagnostic.start_revision = rev
  new_diagnostic.end_revision = next_diagnostic.start_revision - 1
  yield new_diagnostic.put_async()
  raise ndb.Return(guid_mapping)


@ndb.tasklet
def _FindOrInsertDiagnosticsOutOfOrder(new_entities, test, rev):
  query = SparseDiagnostic.query(
      ndb.AND(SparseDiagnostic.end_revision >= rev - 1,
              SparseDiagnostic.test == test))
  query = query.order(-SparseDiagnostic.end_revision)
  diagnostic_entities = yield query.fetch_async()
  diagnostic_entities = RemoveInvalidSparseDiagnostics(diagnostic_entities)

  new_entities_by_name = dict((d.name, d) for d in new_entities)
  diagnostics_by_name = collections.defaultdict(list)

  for d in diagnostic_entities:
    if d.IsValid():
      diagnostics_by_name[d.name].append(d)
    else:
      diagnostics_by_name.setdefault(d.name, [])

  futures = []

  for name in diagnostics_by_name.keys():
    if not name in new_entities_by_name:
      continue

    futures.append(
        _FindOrInsertNamedDiagnosticsOutOfOrder(new_entities_by_name[name],
                                                diagnostics_by_name[name], rev))

  guid_mappings = yield futures

  new_guids_to_existing_diagnostics = {}

  for guid_mapping in guid_mappings:
    new_guids_to_existing_diagnostics.update(guid_mapping)

  raise ndb.Return(new_guids_to_existing_diagnostics)
