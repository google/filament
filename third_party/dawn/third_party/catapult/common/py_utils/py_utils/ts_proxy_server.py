# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Start and stop tsproxy."""

from __future__ import absolute_import
import locale
import logging
import os
import re
import signal
import subprocess
import sys
import time
import six

try:
  import fcntl
except ImportError:
  fcntl = None

import py_utils
from py_utils import retry_util
from py_utils import atexit_with_log

_TSPROXY_PATH = os.path.join(
    py_utils.GetCatapultDir(), 'third_party', 'tsproxy', 'tsproxy.py')

class TsProxyServerError(Exception):
  """Catch-all exception for tsProxy Server."""

def ParseTsProxyPortFromOutput(output_line):
  port_re = re.compile(
      r'Started Socks5 proxy server on '
      r'(?P<host>[^:]*):'
      r'(?P<port>\d+)')
  m = port_re.match(output_line)
  if m:
    return int(m.group('port'))
  return None


class TsProxyServer():
  """Start and stop tsproxy.

  TsProxy provides basic latency, download and upload traffic shaping. This
  class provides a programming API to the tsproxy script in
  catapult/third_party/tsproxy/tsproxy.py

  This class can be used as a context manager.
  """

  def __init__(self, host_ip=None, http_port=None, https_port=None):
    """
    Initialize TsProxyServer.

    Args:

      host_ip: A string of the host ip address.
      http_port: A decimal of the port used for http traffic.
      https_port: a decimal of the port used for https traffic.

    """
    self._proc = None
    self._port = None
    self._is_running = False
    self._host_ip = host_ip
    assert bool(http_port) == bool(https_port)
    self._http_port = http_port
    self._https_port = https_port
    self._non_blocking = False
    self._rtt = None
    self._inbkps = None
    self._outkbps = None

  @property
  def port(self):
    return self._port

  @retry_util.RetryOnException(TsProxyServerError, retries=3)
  def StartServer(self, timeout=10, retries=None):
    """Start TsProxy server and verify that it started."""
    del retries # Handled by decorator.
    cmd_line = [sys.executable, _TSPROXY_PATH]
    # Use port 0 so tsproxy picks a random available port.
    cmd_line.extend(['--port=0'])
    if self._host_ip:
      cmd_line.append('--desthost=%s' % self._host_ip)
    if self._http_port:
      cmd_line.append(
          '--mapports=443:%s,*:%s' % (self._https_port, self._http_port))
      logging.info('Tsproxy commandline: %s', cmd_line)
    # In python3 subprocess handles encoding/decoding; this warns if it won't
    # be UTF-8.
    if locale.getpreferredencoding() != 'UTF-8':
      logging.warning('Decoding will use %s instead of UTF-8',
                      locale.getpreferredencoding())
    # In python3 universal_newlines forces subprocess to encode/decode,
    # allowing per-line buffering.
    self._proc = subprocess.Popen(
        cmd_line, stdout=subprocess.PIPE, stdin=subprocess.PIPE,
        stderr=subprocess.PIPE, bufsize=1, universal_newlines=True)
    self._non_blocking = False
    if fcntl:
      logging.info('fcntl is supported, trying to set '
                   'non blocking I/O for the ts_proxy process')
      fd = self._proc.stdout.fileno()
      fl = fcntl.fcntl(fd, fcntl.F_GETFL)
      fcntl.fcntl(fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)
      self._non_blocking = True

    atexit_with_log.Register(self.StopServer)
    try:
      py_utils.WaitFor(self._IsStarted, timeout)
      logging.info('TsProxy port: %s', self._port)
      self._is_running = True
    except py_utils.TimeoutException:
      err = self.StopServer()
      if err:
        logging.error('Error stopping WPR server:\n%s', err)
      six.raise_from(TsProxyServerError(
          'Error starting tsproxy: timed out after %s seconds' % timeout), None)

  def _IsStarted(self):
    assert not self._is_running
    assert self._proc
    if self._proc.poll() is not None:
      return False
    self._proc.stdout.flush()
    output_line = self._ReadLineTsProxyStdout(timeout=5)
    logging.debug('TsProxy output: %s', output_line)
    self._port = ParseTsProxyPortFromOutput(output_line)
    return self._port is not None

  def _ReadLineTsProxyStdout(self, timeout):
    def ReadSingleLine():
      try:
        return self._proc.stdout.readline().strip()
      except IOError:
        # Add a sleep to avoid trying to read self._proc.stdout too often.
        if self._non_blocking:
          time.sleep(0.5)
        return None
    return py_utils.WaitFor(ReadSingleLine, timeout)

  @retry_util.RetryOnException(TsProxyServerError, retries=3)
  def _IssueCommand(self, command_string, timeout, retries=None):
    del retries  # handled by the decorator
    logging.info('Issuing command to ts_proxy_server: %s', command_string)
    command_output = []
    self._proc.stdin.write(('%s\n' % command_string))
    def CommandStatusIsRead():
      self._proc.stdin.flush()
      self._proc.stdout.flush()
      command_output.append(self._ReadLineTsProxyStdout(timeout))
      return command_output[-1] == 'OK' or command_output[-1] == 'ERROR'

    py_utils.WaitFor(CommandStatusIsRead, timeout)

    success = 'OK' in command_output
    logging.log(logging.DEBUG if success else logging.ERROR,
                'TsProxy output:\n%s', '\n'.join(command_output))
    if not success:
      six.raise_from(TsProxyServerError('Failed to execute command: %s',
                                        command_string), None)

  def UpdateOutboundPorts(self, http_port, https_port, timeout=5):
    assert http_port and https_port
    assert http_port != https_port
    assert isinstance(http_port, int) and isinstance(https_port, int)
    assert 1 <= http_port <= 65535
    assert 1 <= https_port <= 65535
    self._IssueCommand('set mapports 443:%i,*:%i' % (https_port, http_port),
                       timeout)

  def UpdateTrafficSettings(
      self, round_trip_latency_ms=None,
      download_bandwidth_kbps=None, upload_bandwidth_kbps=None, timeout=20):
    """Update traffic settings of the proxy server.

    Notes that this method only updates the specified parameter.
    """
    # Memorize the traffic settings & only execute the command if the traffic
    # settings are different.
    if round_trip_latency_ms is not None and self._rtt != round_trip_latency_ms:
      self._IssueCommand('set rtt %s' % round_trip_latency_ms, timeout)
      self._rtt = round_trip_latency_ms

    if (download_bandwidth_kbps is not None and
        self._inbkps != download_bandwidth_kbps):
      self._IssueCommand('set inkbps %s' % download_bandwidth_kbps, timeout)
      self._inbkps = download_bandwidth_kbps

    if (upload_bandwidth_kbps is not None and
        self._outkbps != upload_bandwidth_kbps):
      self._IssueCommand('set outkbps %s' % upload_bandwidth_kbps, timeout)
      self._outkbps = upload_bandwidth_kbps

  def StopServer(self):
    """Stop TsProxy Server."""
    if not self._is_running:
      logging.debug('Attempting to stop TsProxy server that is not running.')
      return None
    if not self._proc:
      return None
    try:
      self._IssueCommand('exit', timeout=10)
      py_utils.WaitFor(lambda: self._proc.poll() is not None, 10)
    except py_utils.TimeoutException:
      # signal.SIGINT is not supported on Windows.
      if not sys.platform.startswith('win'):
        try:
          # Use a SIGNINT so that it can do graceful cleanup
          self._proc.send_signal(signal.SIGINT)
        except ValueError:
          logging.warning('Unable to stop ts_proxy_server gracefully.\n')
      self._proc.terminate()
    _, err = self._proc.communicate()

    self._proc = None
    self._port = None
    self._is_running = False
    self._rtt = None
    self._inbkps = None
    self._outkbps = None
    return err

  def __enter__(self):
    """Add support for with-statement."""
    self.StartServer()
    return self

  def __exit__(self, unused_exc_type, unused_exc_val, unused_exc_tb):
    """Add support for with-statement."""
    self.StopServer()
