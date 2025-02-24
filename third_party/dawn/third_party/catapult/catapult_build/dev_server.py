# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function

from __future__ import absolute_import
import argparse
import json
import os
import sys
import six
import six.moves.urllib.parse # pylint: disable=import-error

from hooks import install

# pylint: disable=import-error
from paste import fileapp
from paste import httpserver
# pylint: enable=import-error

import webapp2
from webapp2 import Route, RedirectHandler

from dashboard_build import dashboard_dev_server_config
from tracing_build import tracing_dev_server_config
from netlog_viewer_build import netlog_viewer_dev_server_config

_MAIN_HTML = """<html><body>
<h1>Run Unit Tests</h1>
<ul>
%s
</ul>
<h1>Quick links</h1>
<ul>
%s
</ul>
</body></html>
"""

_QUICK_LINKS = [
    ('Trace File Viewer',
     '/tracing_examples/trace_viewer.html'),
    ('Metrics debugger',
     '/tracing_examples/metrics_debugger.html'),
]

_LINK_ITEM = '<li><a href="%s">%s</a></li>'

def _GetFilesIn(basedir):
  data_files = []
  for dirpath, dirnames, filenames in os.walk(basedir, followlinks=True):
    new_dirnames = [d for d in dirnames if not d.startswith('.')]
    del dirnames[:]
    dirnames += new_dirnames

    for f in filenames:
      if f.startswith('.'):
        continue
      if f == 'README.md':
        continue
      full_f = os.path.join(dirpath, f)
      rel_f = os.path.relpath(full_f, basedir)
      data_files.append(rel_f)

  data_files.sort()
  return data_files


def _RelPathToUnixPath(p):
  return p.replace(os.sep, '/')


class TestResultHandler(webapp2.RequestHandler):
  def post(self, *args, **kwargs):  # pylint: disable=unused-argument
    msg = six.ensure_str(self.request.body)
    ostream = sys.stdout if 'PASSED' in msg else sys.stderr
    ostream.write(msg + '\n')
    return self.response.write('')


class TestsCompletedHandler(webapp2.RequestHandler):
  def post(self, *args, **kwargs):  # pylint: disable=unused-argument
    msg = six.ensure_str(self.request.body)
    sys.stdout.write(msg + '\n')
    exit_code = 0 if 'ALL_PASSED' in msg else 1
    if hasattr(self.app.server, 'please_exit'):
      self.app.server.please_exit(exit_code)
    return self.response.write('')

class TestsErrorHandler(webapp2.RequestHandler):
  def post(self, *args, **kwargs):
    del args, kwargs
    msg = six.ensure_str(self.request.body)
    sys.stderr.write(msg + '\n')
    exit_code = 1
    if hasattr(self.app.server, 'please_exit'):
      self.app.server.please_exit(exit_code)
    return self.response.write('')

class DirectoryListingHandler(webapp2.RequestHandler):
  def get(self, *args, **kwargs):  # pylint: disable=unused-argument
    source_path = kwargs.pop('_source_path', None)
    mapped_path = kwargs.pop('_mapped_path', None)
    assert mapped_path.endswith('/')

    data_files_relative_to_top = _GetFilesIn(source_path)
    data_files = [mapped_path + x
                  for x in data_files_relative_to_top]

    files_as_json = json.dumps(data_files)
    self.response.content_type = 'application/json'
    return self.response.write(files_as_json)


class FileAppWithGZipHandling(fileapp.FileApp):
  def guess_type(self):
    content_type, content_encoding = \
        super(FileAppWithGZipHandling, self).guess_type()
    if not self.filename.endswith('.gz'):
      return content_type, content_encoding
    # By default, FileApp serves gzip files as their underlying type with
    # Content-Encoding of gzip. That causes them to show up on the client
    # decompressed. That ends up being surprising to our xhr.html system.
    return None, None

class SourcePathsHandler(webapp2.RequestHandler):
  def get(self, *args, **kwargs):  # pylint: disable=unused-argument
    source_paths = kwargs.pop('_source_paths', [])

    path = self.request.path
    # This is how we do it. Its... strange, but its what we've done since
    # the dawn of time. Aka 4 years ago, lol.
    for mapped_path in source_paths:
      rel = os.path.relpath(path, '/')
      candidate = os.path.join(mapped_path, rel)
      if os.path.exists(candidate):
        app = FileAppWithGZipHandling(candidate)
        app.cache_control(no_cache=True)
        return app
    self.abort(404)
    return None

  @staticmethod
  def GetServingPathForAbsFilename(source_paths, filename):
    if not os.path.isabs(filename):
      raise Exception('filename must be an absolute path')

    for mapped_path in source_paths:
      if not filename.startswith(mapped_path):
        continue
      rel = os.path.relpath(filename, mapped_path)
      unix_rel = _RelPathToUnixPath(rel)
      return unix_rel
    return None


class SimpleDirectoryHandler(webapp2.RequestHandler):
  def get(self, *args, **kwargs):  # pylint: disable=unused-argument
    top_path = os.path.abspath(kwargs.pop('_top_path', None))
    if not top_path.endswith(os.path.sep):
      top_path += os.path.sep

    joined_path = os.path.abspath(
        os.path.join(top_path, kwargs.pop('rest_of_path')))
    if not joined_path.startswith(top_path):
      self.response.set_status(403)
      return None
    app = FileAppWithGZipHandling(joined_path)
    app.cache_control(no_cache=True)
    return app


class TestOverviewHandler(webapp2.RequestHandler):
  def get(self, *args, **kwargs):  # pylint: disable=unused-argument
    test_links = []
    for name, path in kwargs.pop('pds').items():
      test_links.append(_LINK_ITEM % (path, name))
    quick_links = []
    for name, path in _QUICK_LINKS:
      quick_links.append(_LINK_ITEM % (path, name))
    self.response.out.write(_MAIN_HTML % ('\n'.join(test_links),
                                          '\n'.join(quick_links)))

class DevServerApp(webapp2.WSGIApplication):
  def __init__(self, pds, args):
    super(DevServerApp, self).__init__(debug=True)
    self.pds = pds
    self._server = None
    self._all_source_paths = []
    self._all_mapped_test_data_paths = []
    self._InitFromArgs(args)

  @property
  def server(self):
    return self._server

  @server.setter
  def server(self, server):
    self._server = server

  def _InitFromArgs(self, args):
    default_tests = dict((pd.GetName(), pd.GetRunUnitTestsUrl())
                         for pd in self.pds)
    routes = [
        Route('/tests.html', TestOverviewHandler,
              defaults={'pds': default_tests}),
        Route('', RedirectHandler, defaults={'_uri': '/tests.html'}),
        Route('/', RedirectHandler, defaults={'_uri': '/tests.html'}),
    ]
    for pd in self.pds:
      routes += pd.GetRoutes(args)
      routes += [
          Route('/%s/notify_test_result' % pd.GetName(),
                TestResultHandler),
          Route('/%s/notify_tests_completed' % pd.GetName(),
                TestsCompletedHandler),
          Route('/%s/notify_test_error' % pd.GetName(),
                TestsErrorHandler)
      ]

    for pd in self.pds:
      # Test data system.
      for mapped_path, source_path in pd.GetTestDataPaths(args):
        self._all_mapped_test_data_paths.append((mapped_path, source_path))
        routes.append(Route('%s__file_list__' % mapped_path,
                            DirectoryListingHandler,
                            defaults={
                                '_source_path': source_path,
                                '_mapped_path': mapped_path
                            }))
        routes.append(Route('%s<rest_of_path:.+>' % mapped_path,
                            SimpleDirectoryHandler,
                            defaults={'_top_path': source_path}))

    # This must go last, because its catch-all.
    #
    # Its funky that we have to add in the root path. The long term fix is to
    # stop with the crazy multi-source-pathing thing.
    for pd in self.pds:
      self._all_source_paths += pd.GetSourcePaths(args)
    routes.append(
        Route('/<:.+>', SourcePathsHandler,
              defaults={'_source_paths': self._all_source_paths}))

    for route in routes:
      self.router.add(route)

  def GetAbsFilenameForHref(self, href):
    for source_path in self._all_source_paths:
      full_source_path = os.path.abspath(source_path)
      expanded_href_path = os.path.abspath(os.path.join(full_source_path,
                                                        href.lstrip('/')))
      if (os.path.exists(expanded_href_path) and
          os.path.commonprefix([full_source_path,
                                expanded_href_path]) == full_source_path):
        return expanded_href_path
    return None

  def GetURLForAbsFilename(self, filename):
    assert self.server is not None
    for mapped_path, source_path in self._all_mapped_test_data_paths:
      if not filename.startswith(source_path):
        continue
      rel = os.path.relpath(filename, source_path)
      unix_rel = _RelPathToUnixPath(rel)
      url = six.moves.urllib.parse.urljoin(mapped_path, unix_rel)
      return url

    path = SourcePathsHandler.GetServingPathForAbsFilename(
        self._all_source_paths, filename)
    if path is None:
      return None
    return six.moves.urllib.parse.urljoin('/', path)


def _AddPleaseExitMixinToServer(server):
  # Shutting down httpserver gracefully and yielding a return code requires
  # a bit of mixin code.

  exit_code_attempt = []
  def PleaseExit(exit_code):
    if len(exit_code_attempt) > 0:
      return
    exit_code_attempt.append(exit_code)
    server.running = False

  real_serve_forever = server.serve_forever

  def ServeForever():
    try:
      real_serve_forever()
    except KeyboardInterrupt:
      # allow CTRL+C to shutdown
      return 255

    print("Exiting dev server")
    if len(exit_code_attempt) == 1:
      return exit_code_attempt[0]
    # The serve_forever returned for some reason separate from
    # exit_please.
    return 0

  server.please_exit = PleaseExit
  server.serve_forever = ServeForever


def _AddCommandLineArguments(pds, argv):
  parser = argparse.ArgumentParser(description='Run development server')
  parser.add_argument(
      '--no-install-hooks', dest='install_hooks', action='store_false')
  parser.add_argument('-p', '--port', default=8003, type=int)
  for pd in pds:
    g = parser.add_argument_group(pd.GetName())
    pd.AddOptionstToArgParseGroup(g)
  args = parser.parse_args(args=argv[1:])
  return args


def Main(argv):
  pds = [
      dashboard_dev_server_config.DashboardDevServerConfig(),
      tracing_dev_server_config.TracingDevServerConfig(),
      netlog_viewer_dev_server_config.NetlogViewerDevServerConfig(),
  ]

  args = _AddCommandLineArguments(pds, argv)

  if args.install_hooks:
    install.InstallHooks()

  app = DevServerApp(pds, args=args)

  server = httpserver.serve(app, host='127.0.0.1', port=args.port,
                            start_loop=False, daemon_threads=True)
  _AddPleaseExitMixinToServer(server)
  # pylint: disable=no-member
  server.urlbase = 'http://127.0.0.1:%i' % server.server_port
  app.server = server

  sys.stderr.write('Now running on %s\n' % server.urlbase)

  return server.serve_forever()
