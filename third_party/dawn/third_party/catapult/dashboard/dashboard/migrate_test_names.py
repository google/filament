# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Request handler to kick off task that migrates a TestMetadata and
its Rows to a new name.

A rename consists of listing all TestMetadata entities which match the old_name,
and then, for each, completing these steps:
  * Create a new TestMetadata entity with the new name.
  * Re-parent all TestMetadata and Row entities from the old TestMetadata to
  * the new TestMetadata.
  * Update alerts to reference the new TestMetadata.
  * Delete the old TestMetadata.

For any rename, there could be hundreds of TestMetadatas and many thousands of
Rows. Datastore operations often time out after a few hundred puts(), so this
task is split up using the task queue.
"""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging

from dashboard import migrate_test_names_tasks
from dashboard.common import datastore_hooks
from dashboard.common import request_handler
from dashboard.common import utils

from flask import request

_ACCESS_GROUP_NAME = 'chromeperf-test-rename-access'


def IsRequestAllowed():
  """Checks if the currently logged in user is a member of the authz group"""

  if utils.IsDevAppserver():
    is_allowed = True
  else:
    user_email = utils.GetEmail()
    is_allowed = user_email and utils.IsGroupMember(user_email,
                                                    _ACCESS_GROUP_NAME)

  return is_allowed


def MigrateTestNamesGet():
  """Displays a simple UI form to kick off migrations."""

  is_allowed = IsRequestAllowed()
  if not is_allowed:
    # Display the unauthorized access page
    return request_handler.RequestHandlerRenderHtml(
        'migrate_test_names_unauthorized.html', {})

  # Display the migration tool page
  return request_handler.RequestHandlerRenderHtml('migrate_test_names.html', {})


def MigrateTestNamesPost():
  """Starts migration of old TestMetadata entity names to new ones.

  The form that's used to kick off migrations will give the parameters
  old_pattern and new_pattern, which are both test path pattern strings.

  """

  is_allowed = IsRequestAllowed()
  user_email = utils.GetEmail()
  logging.info('Test Migration request from %s. Request details:%s', user_email,
               request.values.items())

  if not is_allowed:
    logging.error('Unauthorized access: User %s', user_email)

    return request_handler.RequestHandlerReportError(
        'Unauthorized access to the test migration tool. ' +
        'Please contact browser-perf-engprod@google.com for access.',
        status=401)

  datastore_hooks.SetPrivilegedRequest()

  try:
    old_pattern = request.values.get('old_pattern')
    new_pattern = request.values.get('new_pattern')
    migrate_test_names_tasks.MigrateTestBegin(old_pattern, new_pattern,
                                              user_email)
    return request_handler.RequestHandlerRenderHtml(
        'result.html', {'headline': 'Test name migration task started.'})
  except migrate_test_names_tasks.BadInputPatternError as error:
    return request_handler.RequestHandlerReportError(
        'Error: %s' % str(error), status=400)
