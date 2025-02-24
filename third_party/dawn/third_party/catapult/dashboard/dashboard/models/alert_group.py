# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""The database model for an "Anomaly", which represents a step up or down."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging
import uuid

from google.appengine.ext import ndb

# Move import of protobuf-dependent code here so that all AppEngine work-arounds
# have a chance to be live before we import any proto code.
from dashboard import sheriff_config_client

NONOVERLAP_THRESHOLD = 100


class RevisionRange(ndb.Model):
  repository = ndb.StringProperty()
  start = ndb.IntegerProperty()
  end = ndb.IntegerProperty()

  def IsOverlapping(self, b):
    if not b or self.repository != b.repository:
      return False
    return max(self.start, b.start) <= min(self.end, b.end)

  def HasSmallNonoverlap(self, b):
    return ((abs(self.start - b.start) + abs(self.end - b.end)) <=
            NONOVERLAP_THRESHOLD)


class BugInfo(ndb.Model):
  project = ndb.StringProperty(indexed=True)
  bug_id = ndb.IntegerProperty(indexed=True)


class AlertGroup(ndb.Model):
  name = ndb.StringProperty(indexed=True)
  domain = ndb.StringProperty(indexed=True)
  subscription_name = ndb.StringProperty(indexed=True)
  created = ndb.DateTimeProperty(indexed=False, auto_now_add=True)
  updated = ndb.DateTimeProperty(indexed=False, auto_now_add=True)

  class Status:
    unknown = 0
    untriaged = 1
    triaged = 2
    bisected = 3
    closed = 4
    sandwiched = 5

  status = ndb.IntegerProperty(indexed=False)

  class Type:
    test_suite = 0
    logical = 1
    reserved = 2
    test_suite_skia = 3

  group_type = ndb.IntegerProperty(
      indexed=False,
      choices=[
          Type.test_suite, Type.logical, Type.reserved, Type.test_suite_skia
      ],
      default=Type.test_suite,
  )
  active = ndb.BooleanProperty(indexed=True)
  revision = ndb.LocalStructuredProperty(RevisionRange)
  bug = ndb.StructuredProperty(BugInfo, indexed=True)
  project_id = ndb.StringProperty(indexed=True, default='chromium')
  bisection_ids = ndb.StringProperty(repeated=True)
  anomalies = ndb.KeyProperty(repeated=True)
  # Key of canonical AlertGroup. If not None the group is considered to be
  # duplicate.
  canonical_group = ndb.KeyProperty(indexed=True)

  sandwich_verification_workflow_id = ndb.StringProperty(indexed=True)

  def IsOverlapping(self, b):
    return (self.name == b.name and self.domain == b.domain
            and self.subscription_name == b.subscription_name
            and self.project_id == b.project_id
            and self.group_type == b.group_type
            and self.revision.IsOverlapping(b.revision)
            and (self.revision.HasSmallNonoverlap(b.revision)
                 if self.domain == 'ChromiumPerf' else True))

  @classmethod
  def GetType(cls, anomaly_entity):
    if anomaly_entity.alert_grouping:
      return cls.Type.logical

    if anomaly_entity.source and anomaly_entity.source == 'skia':
      return cls.Type.test_suite_skia

    return cls.Type.test_suite

  @classmethod
  def GenerateAllGroupsForAnomaly(cls,
                                  anomaly_entity,
                                  sheriff_config=None,
                                  subscriptions=None):
    if subscriptions is None:
      sheriff_config = (sheriff_config
                        or sheriff_config_client.GetSheriffConfigClient())
      subscriptions, _ = sheriff_config.Match(anomaly_entity.test.string_id(),
                                              check=True)
    names = anomaly_entity.alert_grouping or [anomaly_entity.benchmark_name]

    return [
        cls(
            id=str(uuid.uuid4()),
            name=group_name,
            domain=anomaly_entity.master_name,
            subscription_name=s.name,
            project_id=s.monorail_project_id,
            status=cls.Status.untriaged,
            group_type=cls.GetType(anomaly_entity),
            active=True,
            revision=RevisionRange(
                repository='chromium',
                start=anomaly_entity.start_revision,
                end=anomaly_entity.end_revision,
            ),
        ) for s in subscriptions for group_name in names
    ]

  @classmethod
  def GetGroupsForAnomaly(cls, anomaly_entity, subscriptions):
    names = anomaly_entity.alert_grouping or [anomaly_entity.benchmark_name]
    if anomaly_entity.alert_grouping:
      logging.warning('alert_grouping is still in use: %s',
                      anomaly_entity.alert_grouping)
    group_type = cls.GetType(anomaly_entity)
    # all_possible_groups including all groups for the anomaly. Some of groups
    # may havn't been created yet.
    all_possible_groups = cls.GenerateAllGroupsForAnomaly(
        anomaly_entity,
        subscriptions=subscriptions,
    )
    # existing_groups are groups which have been created and overlaped with at
    # least one of the possible group.
    existing_groups = [
        g1 for name in names
        for g1 in cls.Get(name, group_type)
        if any(g1.IsOverlapping(g2) for g2 in all_possible_groups)]
    # Each of the all_possible_groups should either has overlap with an
    # existing one, or be created.
    # If any of the group in the all_possible_groups doesn't overlap with any
    # of the existing group, we put it into a special group for creating the
    # non-existent group in next alert_group_workflow iteration.
    if not existing_groups or not all(
        any(g1.IsOverlapping(g2) for g2 in existing_groups)
        for g1 in all_possible_groups):
      existing_groups += cls.Get('Ungrouped', cls.Type.reserved)
    return [g.key for g in existing_groups]

  @classmethod
  def GetByID(cls, group_id):
    return ndb.Key('AlertGroup', group_id).get()

  @classmethod
  def Get(cls, group_name, group_type, active=True):
    query = cls.query(
        cls.active == active,
        cls.name == group_name,
    )
    return [g for g in query.fetch() if g.group_type == group_type]

  @classmethod
  def GetAll(cls, active=True, group_type=Type.test_suite):
    groups = cls.query(cls.active == active).fetch()
    return [g for g in groups if g.group_type == group_type] or []
