# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Adds pre hook to data store queries to hide internal-only data.

Checks if the user has a google.com address, and hides data with the
internal_only property set if not.
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from flask import g as flask_global
from flask import request as flask_request
import logging
import os

from google.appengine.api import apiproxy_stub_map
from google.appengine.api import users
from google.appengine.datastore import datastore_pb

from dashboard.common import utils

# The list below contains all kinds that have an internal_only property.
# IMPORTANT: any new data types with internal_only properties must be added
# here in order to be restricted to internal users.
_INTERNAL_ONLY_KINDS = [
    'Bot',
    'TestMetadata',
    'Sheriff',
    'Anomaly',
    'TryJob',
    'TableConfig',
    'Histogram',
    'SparseDiagnostic',
    'ReportTemplate',
]

# Permissions namespaces.
EXTERNAL = 'externally_visible'
INTERNAL = 'internal_only'


def InstallHooks():
  """Installs datastore pre hook to add access checks to queries.

  This only needs to be called once, when doing config (currently in
  appengine_config.py).
  """
  apiproxy_stub_map.apiproxy.GetPreCallHooks().Push('_DatastorePreHook',
                                                    _DatastorePreHook,
                                                    'datastore_v3')


def SetPrivilegedRequest():
  """Allows the current request to act as a privileged user.

  This should ONLY be called for handlers that are restricted from end users
  by some other mechanism (IP allowlist, admin-only pages).

  This should be set once per request, before accessing the data store.
  """
  flask_global.privileged = True


def SetSinglePrivilegedRequest():
  """Allows the current request to act as a privileged user only ONCE.

  This should be called ONLY by handlers that have checked privilege immediately
  before making a query. It will be automatically unset when the next query is
  made.
  """
  flask_global.single_privileged = True


def CancelSinglePrivilegedRequest():
  """Disallows the current request to act as a privileged user only.

  """
  flask_global.single_privileged = False


def _IsServicingPrivilegedRequest():
  """Checks whether the request is considered privileged.

  """
  try:
    if 'privileged' in flask_global and flask_global.privileged:
      return True
    if 'single_privileged' in flask_global and flask_global.single_privileged:
      flask_global.pop('single_privileged')
      return True
    path = flask_request.path
  except RuntimeError:
    # This happens in defer queue and unit tests, when code gets called
    # without any context of a flask request.
    try:
      path = os.environ['PATH_INFO']
    except KeyError:
      logging.error(
          'Cannot tell whether a request is privileged without request path.')
      return False
  if path.startswith('/mapreduce'):
    return True
  if path.startswith('/_ah/queue/deferred'):
    return True
  if path.startswith('/_ah/pipeline/'):
    return True
  # We have been checking on utils.GetIpAllowlist() here. Though, the list
  # has been empty and we are infinite recursive calls in crbug/1402197.
  # Thus, we remove the check here.
  return False


def IsUnalteredQueryPermitted():
  """Checks if the current user is internal, or the request is privileged.

  "Internal users" are users whose email address belongs to a certain
  privileged domain; but some privileged requests, such as task queue tasks,
  are also considered privileged.

  Returns:
    True for users with google.com emails and privileged requests.
  """
  if _IsServicingPrivilegedRequest():
    return True
  if utils.IsInternalUser():
    return True
  # It's possible to be an admin with a non-internal account; For example,
  # the default login for dev appserver instances is test@example.com.
  return users.is_current_user_admin()


def GetNamespace():
  """Returns a namespace prefix string indicating internal or external user."""
  return INTERNAL if IsUnalteredQueryPermitted() else EXTERNAL


def _DatastorePreHook(service, call, request, _):
  """Adds a filter which checks whether to return internal data for queries.

  If the user is not privileged, we don't want to return any entities that
  have internal_only set to True. That is done here in a datastore hook.
  See: https://developers.google.com/appengine/articles/hooks

  Args:
    service: Service name, must be 'datastore_v3'.
    call: String representing function to call. One of 'Put', Get', 'Delete',
        or 'RunQuery'.
    request: Request protobuf.
    _: Response protobuf (not used).
  """
  assert service == 'datastore_v3'
  if call != 'RunQuery':
    return
  if IsUnalteredQueryPermitted():
    return

  # Add a filter for internal_only == False, because the user is external.
  if request.kind not in _INTERNAL_ONLY_KINDS:
    return
  query_filter = request.filter.add()
  query_filter.op = datastore_pb.Query.Filter.EQUAL
  filter_property = query_filter.property.add()
  filter_property.name = 'internal_only'
  filter_property.value.booleanValue = False
  filter_property.multiple = False
