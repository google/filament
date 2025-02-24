# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from flask import Flask
import unittest

import webtest

from google.appengine.api import users

from dashboard import edit_site_config
from dashboard.common import namespaced_stored_object
from dashboard.common import stored_object
from dashboard.common import testing_common
from dashboard.common import xsrf

flask_app = Flask(__name__)


@flask_app.route('/edit_site_config', methods=['GET'])
def EditSiteConfigHandlerGet():
  return edit_site_config.EditSiteConfigHandlerGet()


@flask_app.route('/edit_site_config', methods=['POST'])
def EditSiteConfigHandlerPost():
  return edit_site_config.EditSiteConfigHandlerPost()


class EditSiteConfigTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.testapp = webtest.TestApp(flask_app)
    testing_common.SetIsInternalUser('internal@chromium.org', True)
    testing_common.SetIsInternalUser('foo@chromium.org', False)
    self.SetCurrentUser('internal@chromium.org', is_admin=True)

  def testGet_NoKey_ShowsPageWithNoTextArea(self):
    response = self.testapp.get('/edit_site_config')
    self.assertEqual(0, len(response.html('textarea')))

  def testGet_WithNonNamespacedKey_ShowsPageWithCurrentValue(self):
    stored_object.Set('foo', 'XXXYYY')
    response = self.testapp.get('/edit_site_config?key=foo')
    self.assertEqual(1, len(response.html('form')))
    self.assertIn(b'XXXYYY', response.body)

  def testGet_WithNamespacedKey_ShowsPageWithBothVersions(self):
    namespaced_stored_object.Set('foo', 'XXXYYY')
    namespaced_stored_object.SetExternal('foo', 'XXXinternalYYY')
    response = self.testapp.get('/edit_site_config?key=foo')
    self.assertEqual(1, len(response.html('form')))
    self.assertIn(b'XXXYYY', response.body)
    self.assertIn(b'XXXinternalYYY', response.body)

  def testPost_NoXsrfToken_ReturnsErrorStatus(self):
    self.testapp.post(
        '/edit_site_config', {
            'key': 'foo',
            'value': '[1, 2, 3]',
        }, status=403)

  def testPost_ExternalUser_ShowsErrorMessage(self):
    self.SetCurrentUser('foo@chromium.org')
    response = self.testapp.post(
        '/edit_site_config', {
            'key': 'foo',
            'value': '[1, 2, 3]',
            'xsrf_token': xsrf.GenerateToken(users.get_current_user()),
        })
    self.assertIn(b'Only internal users', response.body)

  def testPost_WithKey_UpdatesNonNamespacedValues(self):
    self.testapp.post(
        '/edit_site_config', {
            'key': 'foo',
            'value': '[1, 2, 3]',
            'xsrf_token': xsrf.GenerateToken(users.get_current_user()),
        })
    self.assertEqual([1, 2, 3], stored_object.Get('foo'))

  def testPost_WithSomeInvalidJSON_ShowsErrorAndDoesNotModify(self):
    stored_object.Set('foo', 'XXX')
    response = self.testapp.post(
        '/edit_site_config', {
            'key': 'foo',
            'value': '[1, 2, this is not json',
            'xsrf_token': xsrf.GenerateToken(users.get_current_user()),
        })
    self.assertIn(b'Invalid JSON', response.body)
    self.assertEqual('XXX', stored_object.Get('foo'))

  def testPost_WithKey_UpdatesNamespacedValues(self):
    namespaced_stored_object.Set('foo', 'XXXinternalYYY')
    namespaced_stored_object.SetExternal('foo', 'XXXYYY')
    self.testapp.post(
        '/edit_site_config', {
            'key': 'foo',
            'external_value': '{"x": "y"}',
            'internal_value': '{"x": "yz"}',
            'xsrf_token': xsrf.GenerateToken(users.get_current_user()),
        })
    self.assertEqual({'x': 'yz'}, namespaced_stored_object.Get('foo'))
    self.assertEqual({'x': 'y'}, namespaced_stored_object.GetExternal('foo'))

  def testPost_SendsNotificationEmail(self):
    namespaced_stored_object.SetExternal('foo', {'x': 10, 'y': 2})
    namespaced_stored_object.Set('foo', {'z': 3, 'x': 1})
    self.testapp.post(
        '/edit_site_config', {
            'key': 'foo',
            'external_value': '{"x": 1, "y": 2}',
            'internal_value': '{"x": 1, "z": 3, "y": 2}',
            'xsrf_token': xsrf.GenerateToken(users.get_current_user()),
        })
    messages = self.mail_stub.get_sent_messages()
    self.assertEqual(1, len(messages))
    self.assertEqual('gasper-alerts@google.com', messages[0].sender)
    self.assertEqual('browser-perf-engprod@google.com',
                     messages[0].to)
    self.assertEqual('Config "foo" changed by internal@chromium.org',
                     messages[0].subject)

    self.assertIn(
        'Non-namespaced value diff:\n'
        '  null\n'
        '\n'
        'Externally-visible value diff:\n'
        '  {\n'
        '-   "x": 10,\n'
        '?         -\n'
        '\n'
        '+   "x": 1,\n'
        '    "y": 2\n'
        '  }\n'
        '\n'
        'Internal-only value diff:\n'
        '  {\n'
        '    "x": 1,\n'
        '+   "y": 2,\n'
        '    "z": 3\n'
        '  }\n', str(messages[0].body))


class HelperFunctionTests(unittest.TestCase):

  def testDiffJson_NoneToEmptyString(self):
    self.assertEqual('- null\n+ ""', edit_site_config._DiffJson(None, ''))

  def testDiffJson_AddListItem(self):
    self.assertEqual(
        '  [\n    1,\n+   2,\n    3\n  ]',
        edit_site_config._DiffJson([1, 3], [1, 2, 3]).replace(", ", ","))


if __name__ == '__main__':
  unittest.main()
