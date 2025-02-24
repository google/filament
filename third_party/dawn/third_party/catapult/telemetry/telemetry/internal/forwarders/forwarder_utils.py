# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""A helper for common operations in ssh forwarding"""
from __future__ import absolute_import
import re

import py_utils

def ReadRemotePort(filename):
  """Fetches the port created remotely.

  When we specify the remote port '0' in ssh remote port forwarding,
  the remote ssh server should return the port it binds to in stderr.
  e.g. 'Allocated port 42360 for remote forward to localhost:12345', the port
  42360 is the port created remotely and the traffic to the port will be
  relayed to localhost port 12345.

  Args:
    filename: the file that stderr is redirected to.

  Returns:
    The port created on the remote device.
  """
  def TryReadingPort(f):
    line = f.readline()
    tokens = re.search(r'port (\d+) for remote forward to', line)
    return int(tokens.group(1)) if tokens else None

  with open(filename, 'r') as f:
    return py_utils.WaitFor(lambda: TryReadingPort(f), timeout=60)

def GetForwardingArgs(local_port, remote_port, host_ip, port_forward):
  """Prepare the forwarding arguments to execute for devices that connect with
  the host via ssh.

  Args:
    local_port: Port on the host.
    remote_port: Port on the remote device.
    host_ip: ip of the host.
    port_forward: Direction of the connection. True if forwarding from the host.

  Returns:
    List of strings, the command arguments for handling port forwarding.
  """
  if port_forward:
    arg_format = '-R{remote_port}:{host_ip}:{local_port}'
  else:
    arg_format = '-L{local_port}:{host_ip}:{remote_port}'
  return [
      arg_format.format(host_ip=host_ip,
                        local_port=local_port,
                        remote_port=remote_port or 0)
  ]
