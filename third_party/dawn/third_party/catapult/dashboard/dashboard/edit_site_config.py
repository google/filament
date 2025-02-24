# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Provides the web interface for adding and editing stored configs."""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import difflib
import json

from google.appengine.api import app_identity
from google.appengine.api import mail
from google.appengine.api import users

from dashboard.common import namespaced_stored_object
from dashboard.common import request_handler
from dashboard.common import stored_object
from dashboard.common import utils
from dashboard.common import xsrf


_NOTIFICATION_EMAIL_BODY = """
The configuration of %(hostname)s was changed by %(user)s.

Key: %(key)s

Non-namespaced value diff:
%(value_diff)s

Externally-visible value diff:
%(external_value_diff)s

Internal-only value diff:
%(internal_value_diff)s
"""

# The mailing list to which config change notifications are sent,
# so that the team can keep an audit record of these changes.
# The "gasper-alerts" address is a historic legacy and not important.
_NOTIFICATION_ADDRESS = 'browser-perf-engprod@google.com'
_SENDER_ADDRESS = 'gasper-alerts@google.com'

from flask import request, make_response
# Handles editing of site config values stored with stored_entity.


def EditSiteConfigHandlerGet():
  """Renders the UI with the form."""
  key = request.args.get('key')
  if not key:
    return request_handler.RequestHandlerRenderHtml('edit_site_config.html', {})

  return_format = request.args.get('format')

  value = stored_object.Get(key)
  external_value = namespaced_stored_object.GetExternal(key)
  internal_value = namespaced_stored_object.Get(key)
  body = {
      'key': key,
      'value': _FormatJson(value),
      'external_value': _FormatJson(external_value),
      'internal_value': _FormatJson(internal_value),
  }
  if return_format == 'json':
    res = make_response(body, 200)
  else:
    res = request_handler.RequestHandlerRenderHtml('edit_site_config.html',
                                                   body)

  return res


@xsrf.TokenRequired
def EditSiteConfigHandlerPost():
  """Accepts posted values, makes changes, and shows the form again."""
  key = request.values.get('key')

  if not utils.IsInternalUser():
    res = request_handler.RequestHandlerRenderHtml(
        'edit_site_config.html',
        {'error': 'Only internal users can post to this end-point.'})
    return res

  if not key:
    return request_handler.RequestHandlerRenderHtml('edit_site_config.html', {})

  new_value_json = request.values.get('value', '').strip()
  new_external_value_json = request.values.get('external_value', '').strip()
  new_internal_value_json = request.values.get('internal_value', '').strip()

  template_params = {
      'key': key,
      'value': new_value_json,
      'external_value': new_external_value_json,
      'internal_value': new_internal_value_json,
  }

  try:
    new_value = json.loads(new_value_json or 'null')
    new_external_value = json.loads(new_external_value_json or 'null')
    new_internal_value = json.loads(new_internal_value_json or 'null')
  except ValueError:
    template_params['error'] = 'Invalid JSON in at least one field.'
    return request_handler.RequestHandlerRenderHtml('edit_site_config.html',
                                                    template_params)

  old_value = stored_object.Get(key)
  old_external_value = namespaced_stored_object.GetExternal(key)
  old_internal_value = namespaced_stored_object.Get(key)

  stored_object.Set(key, new_value)
  namespaced_stored_object.SetExternal(key, new_external_value)
  namespaced_stored_object.Set(key, new_internal_value)

  _SendNotificationEmail(key, old_value, old_external_value, old_internal_value,
                         new_value, new_external_value, new_internal_value)

  return request_handler.RequestHandlerRenderHtml('edit_site_config.html',
                                                  template_params)


def _SendNotificationEmail(key, old_value, old_external_value,
                           old_internal_value, new_value, new_external_value,
                           new_internal_value):
  user_email = users.get_current_user().email()
  subject = 'Config "%s" changed by %s' % (key, user_email)
  email_body = _NOTIFICATION_EMAIL_BODY % {
      'key': key,
      'value_diff': _DiffJson(old_value, new_value),
      'external_value_diff': _DiffJson(old_external_value, new_external_value),
      'internal_value_diff': _DiffJson(old_internal_value, new_internal_value),
      'hostname': app_identity.get_default_version_hostname(),
      'user': users.get_current_user().email(),
  }
  mail.send_mail(
      sender=_SENDER_ADDRESS,
      to=_NOTIFICATION_ADDRESS,
      subject=subject,
      body=email_body)


def _DiffJson(obj1, obj2):
  """Returns a string diff of two JSON-serializable objects."""
  differ = difflib.Differ()
  return '\n'.join(
      differ.compare(
          _FormatJson(obj1).splitlines(),
          _FormatJson(obj2).splitlines()))


def _FormatJson(obj):
  return json.dumps(obj, indent=2, sort_keys=True)
