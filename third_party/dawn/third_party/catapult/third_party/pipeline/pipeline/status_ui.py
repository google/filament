#!/usr/bin/env python
#
# Copyright 2010 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Status UI for Google App Engine Pipeline API."""

import logging
import os
import pkgutil
import traceback

from google.appengine.api import users
from google.appengine.ext import webapp

try:
  import json
except ImportError:
  import simplejson as json

# Relative imports
import util


class _StatusUiHandler(webapp.RequestHandler):
  """Render the status UI."""

  _RESOURCE_MAP = {
    '/status': ('ui/status.html', 'text/html'),
    '/status.css': ('ui/status.css', 'text/css'),
    '/status.js': ('ui/status.js', 'text/javascript'),
    '/list': ('ui/root_list.html', 'text/html'),
    '/list.css': ('ui/root_list.css', 'text/css'),
    '/list.js': ('ui/root_list.js', 'text/javascript'),
    '/common.js': ('ui/common.js', 'text/javascript'),
    '/common.css': ('ui/common.css', 'text/css'),
    '/jquery-1.4.2.min.js': ('ui/jquery-1.4.2.min.js', 'text/javascript'),
    '/jquery.treeview.min.js': ('ui/jquery.treeview.min.js', 'text/javascript'),
    '/jquery.cookie.js': ('ui/jquery.cookie.js', 'text/javascript'),
    '/jquery.timeago.js': ('ui/jquery.timeago.js', 'text/javascript'),
    '/jquery.ba-hashchange.min.js': (
        'ui/jquery.ba-hashchange.min.js', 'text/javascript'),
    '/jquery.json.min.js': ('ui/jquery.json.min.js', 'text/javascript'),
    '/jquery.treeview.css': ('ui/jquery.treeview.css', 'text/css'),
    '/treeview-default.gif': ('ui/images/treeview-default.gif', 'image/gif'),
    '/treeview-default-line.gif': (
        'ui/images/treeview-default-line.gif', 'image/gif'),
    '/treeview-black.gif': ('ui/images/treeview-black.gif', 'image/gif'),
    '/treeview-black-line.gif': (
        'ui/images/treeview-black-line.gif', 'image/gif'),
    '/images/treeview-default.gif': (
        'ui/images/treeview-default.gif', 'image/gif'),
    '/images/treeview-default-line.gif': (
        'ui/images/treeview-default-line.gif', 'image/gif'),
    '/images/treeview-black.gif': (
        'ui/images/treeview-black.gif', 'image/gif'),
    '/images/treeview-black-line.gif': (
        'ui/images/treeview-black-line.gif', 'image/gif'),
  }

  def get(self, resource=''):
    import pipeline  # Break circular dependency
    if pipeline._ENFORCE_AUTH:
      if users.get_current_user() is None:
        logging.debug('User is not logged in')
        self.redirect(users.create_login_url(self.request.url))
        return

      if not users.is_current_user_admin():
        logging.debug('User is not admin: %r', users.get_current_user())
        self.response.out.write('Forbidden')
        self.response.set_status(403)
        return

    if resource not in self._RESOURCE_MAP:
      logging.debug('Could not find: %s', resource)
      self.response.set_status(404)
      self.response.out.write("Resource not found.")
      self.response.headers['Content-Type'] = 'text/plain'
      return

    relative_path, content_type = self._RESOURCE_MAP[resource]
    path = os.path.join(os.path.dirname(__file__), relative_path)
    if not pipeline._DEBUG:
      self.response.headers["Cache-Control"] = "public, max-age=300"
    self.response.headers["Content-Type"] = content_type
    try:
      data = pkgutil.get_data(__name__, relative_path)
    except AttributeError:  # Python < 2.6.
      data = None
    self.response.out.write(data or open(path, 'rb').read())


class _BaseRpcHandler(webapp.RequestHandler):
  """Base handler for JSON-RPC responses.

  Sub-classes should fill in the 'json_response' property. All exceptions will
  be returned.
  """

  def get(self):
    import pipeline  # Break circular dependency
    if pipeline._ENFORCE_AUTH:
      if not users.is_current_user_admin():
        logging.debug('User is not admin: %r', users.get_current_user())
        self.response.out.write('Forbidden')
        self.response.set_status(403)
        return

    # XSRF protection
    if (not pipeline._DEBUG and
        self.request.headers.get('X-Requested-With') != 'XMLHttpRequest'):
      logging.debug('Request missing X-Requested-With header')
      self.response.out.write('Request missing X-Requested-With header')
      self.response.set_status(403)
      return

    self.json_response = {}
    try:
      self.handle()
      output = json.dumps(self.json_response, cls=util.JsonEncoder)
    except Exception, e:
      self.json_response.clear()
      self.json_response['error_class'] = e.__class__.__name__
      self.json_response['error_message'] = str(e)
      self.json_response['error_traceback'] = traceback.format_exc()
      output = json.dumps(self.json_response, cls=util.JsonEncoder)

    self.response.set_status(200)
    self.response.headers['Content-Type'] = 'application/json'
    self.response.headers['Cache-Control'] = 'no-cache'
    self.response.out.write(output)

  def handle(self):
    raise NotImplementedError('To be implemented by sub-classes.')


class _TreeStatusHandler(_BaseRpcHandler):
  """RPC handler for getting the status of all children of root pipeline."""

  def handle(self):
    import pipeline  # Break circular dependency
    self.json_response.update(
        pipeline.get_status_tree(self.request.get('root_pipeline_id')))


class _ClassPathListHandler(_BaseRpcHandler):
  """RPC handler for getting the list of all Pipeline classes defined."""

  def handle(self):
    import pipeline  # Break circular dependency
    self.json_response['classPaths'] = pipeline.get_pipeline_names()


class _RootListHandler(_BaseRpcHandler):
  """RPC handler for getting the status of all root pipelines."""

  def handle(self):
    import pipeline  # Break circular dependency
    self.json_response.update(
        pipeline.get_root_list(
            class_path=self.request.get('class_path'),
            cursor=self.request.get('cursor')))
