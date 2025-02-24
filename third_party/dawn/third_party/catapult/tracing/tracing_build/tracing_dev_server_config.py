# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import os

import tracing_project


import webapp2
from webapp2 import Route


def _RelPathToUnixPath(p):
  return p.replace(os.sep, '/')


class TestListHandler(webapp2.RequestHandler):

  def get(self, *args, **kwargs):  # pylint: disable=unused-argument
    project = tracing_project.TracingProject()
    test_relpaths = ['/' + _RelPathToUnixPath(x)
                     for x in project.FindAllTestModuleRelPaths()]

    tests = {'test_relpaths': test_relpaths}
    tests_as_json = json.dumps(tests)
    self.response.content_type = 'application/json'
    return self.response.write(tests_as_json)


class TracingDevServerConfig(object):

  def __init__(self):
    self.project = tracing_project.TracingProject()

  def GetName(self):
    return 'tracing'

  def GetRunUnitTestsUrl(self):
    return '/tracing/tests.html'

  def AddOptionstToArgParseGroup(self, g):
    g.add_argument('-d', '--data-dir', default=self.project.test_data_path)
    g.add_argument('-s', '--skp-data-dir', default=self.project.skp_data_path)

  def GetRoutes(self, args):  # pylint: disable=unused-argument
    return [Route('/tracing/tests', TestListHandler)]

  def GetSourcePaths(self, args):  # pylint: disable=unused-argument
    return list(self.project.source_paths)

  def GetTestDataPaths(self, args):  # pylint: disable=unused-argument
    return [
        ('/tracing/test_data/', os.path.expanduser(args.data_dir)),
        ('/tracing/skp_data/', os.path.expanduser(args.skp_data_dir)),
    ]
