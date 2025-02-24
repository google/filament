# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""This library contains functions that are to be used to keep a track
of all the ongoing migration tasks in data store.
"""

from __future__ import absolute_import
import uuid
from typing import Optional

from google.appengine.ext import ndb
from dashboard.models import graph_data


# Data for the root entry
class RootJobData:
  total_tests: int = 0
  completed_tests: int = 0
  requested_by: str
  old_pattern: str
  new_pattern: str

  def __init__(self, dict=None):  # pylint: disable=W0622
    if dict:
      self.__dict__ = dict


# Data for each test migration entry
class TestJobData:
  old_test_key: str
  new_test_key: str
  child_count: int = 0
  completed_child_count: int = 0

  def __init__(self, dict=None):  # pylint: disable=W0622
    if dict:
      self.__dict__ = dict


@ndb.transactional(retries=100)
def UpdateParentChildCompleted(job_id: str, parent_id: str) -> bool:
  """ Update the data of the parent and do the following
  1. Increment the completed child count
  2. Update the status of the specific entry

  Since all the migration tasks are potentially spread across server
  instances, we need this to be transactional.
  """

  job_key = ndb.Key(graph_data.MigrationJob, _MigrationJobKey(
      job_id, parent_id))
  job_entry = job_key.get()
  if job_entry:
    if job_entry.entry_type == 'root':
      job_entry_data = job_entry.GetEntryData(RootJobData)
      job_entry_data.completed_tests += 1

      if job_entry_data.completed_tests == job_entry_data.total_tests:
        job_entry.status = 'Completed'
    else:
      job_entry_data = job_entry.GetEntryData(TestJobData)
      job_entry_data.completed_child_count += 1

      if not job_entry_data.child_count or \
          job_entry_data.completed_child_count == \
          job_entry_data.child_count:
        job_entry.status = 'Completed'

    job_entry.SetEntryData(job_entry_data)
    job_entry.put()
    if job_entry.status == 'Completed':
      next_parent = job_entry.key.parent()
      if next_parent:
        is_root_complete = UpdateParentChildCompleted(
            job_id, next_parent.id)
        return is_root_complete

      # No parent, which means this was the root entry
      # that's now complete
      return True

  return False


@ndb.transactional(retries=100)
def UpdateParentChildStarted(job_id: str,
                             parent_id: str) -> graph_data.MigrationJob:
  """ Update the data of the parent and increment the child count

  Since all the migration tasks are potentially spread across server
  instances, we need this to be transactional.
  """

  job_key = ndb.Key(graph_data.MigrationJob, _MigrationJobKey(
      job_id, parent_id))
  job_entry = job_key.get()
  if job_entry.entry_type == 'root':
    job_entry_data = job_entry.GetEntryData(RootJobData)
    job_entry_data.total_tests += 1
  else:
    job_entry_data = job_entry.GetEntryData(TestJobData)
    job_entry_data.child_count += 1

  job_entry.SetEntryData(job_entry_data)
  job_entry.put()
  return job_entry


def GetRootJobEntry(job_id: str) -> Optional[graph_data.MigrationJob]:
  """ Returns the requester email for the job """

  root_data_key = ndb.Key(graph_data.MigrationJob, _MigrationJobKey(
      job_id, 'root'))
  root_job = root_data_key.get()
  return root_job


def _MigrationJobKey(job_id: str, entry_id: str) -> str:
  """Creates a key for the MigrationJobEntry."""

  return "%s##%s" % (job_id, entry_id)


def _GetEntryIdFromJobKey(job_key: str) -> str:
  """Extracts the entry id from job key string"""
  return job_key.split('##', 1)[1]


def CompleteTestJobData(job_id: str, new_test_key: str, parent_id: str) -> bool:
  """ Adds the data of a migrated test to datastore."""
  parent_key = ndb.Key(graph_data.MigrationJob, _MigrationJobKey(
      job_id, parent_id))
  test_key = ndb.Key(graph_data.MigrationJob, _MigrationJobKey(
      job_id, new_test_key), parent=parent_key)
  test_data = test_key.get()
  if test_data:
    test_data.status = 'Completed'
    test_data.put()

  return UpdateParentChildCompleted(job_id, parent_id)


def AddNewTestJobData(
    job_id: str, old_test_key: str, new_test_key: str, parent_id: str) -> str:
  """ Adds a new TestJob entry. """
  parent_key = ndb.Key(graph_data.MigrationJob,
                       _MigrationJobKey(job_id, parent_id))
  test_key = _MigrationJobKey(job_id, new_test_key)
  test_data = graph_data.MigrationJob(id=test_key, parent=parent_key)
  entry_data = TestJobData()
  entry_data.old_test_key = old_test_key
  entry_data.new_test_key = new_test_key
  test_data.SetEntryData(entry_data)
  test_data.job_id = job_id
  test_data.entry_type = 'test'
  test_data.status = 'Running'

  test_data.put()
  UpdateParentChildStarted(job_id, parent_id)
  return _GetEntryIdFromJobKey(test_data.key.id())


def AddRootJobData(old_pattern, new_pattern, requested_by) -> str:
  """ Create a new root entry for the job """
  job_id = uuid.uuid4()
  job_data = graph_data.MigrationJob(id=_MigrationJobKey(job_id, 'root'))
  job_data.job_id = str(job_id)
  job_data.entry_type = 'root'
  job_data.status = 'Running'

  entry_data = RootJobData()
  entry_data.requested_by = requested_by
  entry_data.old_pattern = old_pattern
  entry_data.new_pattern = new_pattern

  job_data.SetEntryData(entry_data)

  job_data.put()

  return job_data.job_id


def GetAllMigratedTests(job_id: str, parent_id: str):
  """ Returns all the migrated test entry for the job id."""

  root_key = ndb.Key(graph_data.MigrationJob,
                     _MigrationJobKey(job_id, parent_id))
  child_items = graph_data.MigrationJob.query(ancestor=root_key).fetch()
  result = []
  for child_item in child_items:
    sub_tests = GetAllMigratedTests(job_id, child_item.key.id())
    if sub_tests:
      result.append(sub_tests)
    else:
      result.append(child_item)

  return result


def CompleteJob(job_id: str):
  """ Housekeeping work after the migration job is complete."""
  child_tests = GetAllMigratedTests(job_id, 'root')
  root_job_id = _MigrationJobKey(job_id, 'root')
  # Delete the child entries for migration since the summary data is
  # now all there in the root entry.
  for child_test in child_tests:
    if child_test.key.id() != root_job_id:
      child_test.key.delete()
