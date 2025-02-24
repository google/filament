# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# TODO(aiolos): this should be moved to catapult/base after the repo move.
# It is used by tracing in tvcm/browser_controller.
from __future__ import print_function
from __future__ import absolute_import
import collections
import json
import logging
import os
import re
import subprocess
import sys
import time
import traceback
import six

from telemetry.core import util

NamedPort = collections.namedtuple('NamedPort', ['name', 'port'])


class LocalServerBackend():

  def __init__(self):
    pass

  def StartAndGetNamedPorts(self, args, handler_class=None):
    """Starts the actual server and obtains any sockets on which it
    should listen.

    Args:
      args: Same as what LocalServer.GetBackendStartupArgs() returns.
      handler_class: Class of the handler which is used by sub-classes to
        process resource requests.

    Returns a list of NamedPort on which this backend is listening.
    """
    raise NotImplementedError()

  def ServeForever(self):
    raise NotImplementedError()


class LocalServer():

  def __init__(self, server_backend_class):
    assert LocalServerBackend in server_backend_class.__bases__
    server_module_name = server_backend_class.__module__
    assert server_module_name in sys.modules, \
        'The server class\' module must be findable via sys.modules'
    assert getattr(sys.modules[server_module_name],
                   server_backend_class.__name__), \
        'The server class must getattrable from its __module__ by its __name__'

    self._server_backend_class = server_backend_class
    self._subprocess = None
    self._devnull = None
    self._local_server_controller = None
    self.host_ip = None
    self.port = None

  def Start(self, local_server_controller):
    assert self._subprocess is None
    self._local_server_controller = local_server_controller

    self.host_ip = local_server_controller.host_ip

    server_args = self.GetBackendStartupArgs()
    server_args_as_json = json.dumps(server_args)
    server_module_name = self._server_backend_class.__module__

    self._devnull = open(os.devnull, 'w')
    cmd = [
        sys.executable,
        '-m',
        __name__,
        'run_backend',
        server_module_name,
        self._server_backend_class.__name__,
        server_args_as_json,
    ]

    env = os.environ.copy()
    env['PYTHONPATH'] = os.pathsep.join(sys.path)

    self._subprocess = subprocess.Popen(cmd,
                                        cwd=util.GetTelemetryDir(),
                                        env=env,
                                        stdout=subprocess.PIPE,
                                        stderr=subprocess.PIPE)

    named_ports = self._GetNamedPortsFromBackend()
    http_port = None
    for p in named_ports:
      if p.name == 'http':
        http_port = p.port
    assert http_port and len(named_ports) == 1, (
        'Only http port is supported: %s' % named_ports)
    self.port = http_port

  def _GetNamedPortsFromBackend(self):
    named_ports_json = None
    named_ports_re = re.compile('LocalServerBackend started: (?P<port>.+)')
    # TODO: This will hang if the subprocess doesn't print the correct output.
    while self._subprocess.poll() is None:
      line = str(self._subprocess.stdout.readline().decode('utf-8'))
      print(line)
      m = named_ports_re.match(line)
      if m:
        named_ports_json = m.group('port')
        break

    if not named_ports_json:
      raise Exception('Server process died prematurely ' +
                      'without giving us port pairs.')
    return [NamedPort(**pair) for pair in json.loads(named_ports_json.lower())]

  @property
  def is_running(self):
    return self._subprocess is not None

  def __enter__(self):
    return self

  def __exit__(self, *args):
    self.Close()

  def __del__(self):
    self.Close()

  def Close(self):
    if self._subprocess:
      self._subprocess.kill()
      # kill() does not close the handle to the process. On Windows, a process
      # will live until you delete all handles to that subprocess, so
      # ps_util.ListAllSubprocesses will find this subprocess if
      # we haven't garbage-collected the handle yet. poll() should close the
      # handle once the process dies.
      time.sleep(.01)
      for _ in range(100):
        if self._subprocess.poll() is None:
          time.sleep(.1)
          continue
        break
      else:
        logging.warning('Local server subprocess is still running after we '
                        'attempted to kill it.')
      self._subprocess = None
    if self._devnull:
      self._devnull.close()
      self._devnull = None
    if self._local_server_controller:
      self._local_server_controller.ServerDidClose(self)
      self._local_server_controller = None

  def GetBackendStartupArgs(self):
    """Returns whatever arguments are required to start up the backend"""
    raise NotImplementedError()


class LocalServerController():
  """Manages the list of running servers

  This class manages the running servers, but also provides an isolation layer
  to prevent LocalServer subclasses from accessing the browser backend directly.

  """

  def __init__(self, platform_backend):
    self._platform_backend = platform_backend
    self._local_servers_by_class = {}
    self.host_ip = self._platform_backend.forwarder_factory.host_ip

  def StartServer(self, server):
    assert not server.is_running, 'Server already started'
    assert self._platform_backend.network_controller_backend.is_open
    assert isinstance(server, LocalServer)
    if server.__class__ in self._local_servers_by_class:
      raise Exception(
          'Cannot have two servers of the same class running at once. ' +
          'Locate the existing one and use it, or call Close() on it.')

    server.Start(self)
    self._local_servers_by_class[server.__class__] = server

  def GetRunningServer(self, server_class, default_value):
    return self._local_servers_by_class.get(server_class, default_value)

  @property
  def local_servers(self):
    return list(self._local_servers_by_class.values())

  def Close(self):
    # TODO(crbug.com/953365): This is a terrible infinite loop scenario
    # and we should fix it.
    while len(self._local_servers_by_class):
      server = next(six.itervalues(self._local_servers_by_class))
      try:
        server.Close()
      except Exception: # pylint: disable=broad-except
        traceback.print_exc()

  def GetRemotePort(self, port):
    return self._platform_backend.GetRemotePort(port)

  def ServerDidClose(self, server):
    del self._local_servers_by_class[server.__class__]


def _LocalServerBackendMain(args):
  assert len(args) == 4
  (cmd, server_module_name, server_backend_class_name,
   server_args_as_json) = args[:4]
  assert cmd == 'run_backend'
  server_module = __import__(server_module_name, fromlist=[True])
  server_backend_class = getattr(server_module, server_backend_class_name)
  server = server_backend_class()

  server_args = json.loads(server_args_as_json)

  # Import the handler class if provided.
  handler_module_name = server_args.get('dynamic_request_handler_module_name')
  handler_class_name = server_args.get('dynamic_request_handler_class_name')
  handler_class = None
  if handler_module_name and handler_class_name:
    handler_module = __import__(handler_module_name, fromlist=[True])
    handler_class = getattr(handler_module, handler_class_name, None)
    logging.info('Loading request handler: %s', handler_class_name)

    if handler_class is None:
      raise Exception('Failed to load request handler %s.' % handler_class_name)

  named_ports = server.StartAndGetNamedPorts(server_args, handler_class)
  assert isinstance(named_ports, list)

  # Note: This message is scraped by the parent process'
  # _GetNamedPortsFromBackend(). Do **not** change it.
  # pylint: disable=protected-access
  print('LocalServerBackend started: %s' %
        json.dumps([pair._asdict() for pair in named_ports]))
  sys.stdout.flush()

  return server.ServeForever()


if __name__ == '__main__':
  sys.exit(
      _LocalServerBackendMain(  # pylint: disable=protected-access
          sys.argv[1:]))
