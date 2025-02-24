"""
websocket - WebSocket client library for Python

Copyright (C) 2010 Hiroki Ohtani(liris)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1335  USA

"""
from __future__ import absolute_import
import socket

import six

from ._exceptions import *
from ._ssl_compat import *
from ._utils import *

DEFAULT_SOCKET_OPTION = [(socket.SOL_TCP, socket.TCP_NODELAY, 1)]
if hasattr(socket, "SO_KEEPALIVE"):
    DEFAULT_SOCKET_OPTION.append((socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1))
if hasattr(socket, "TCP_KEEPIDLE"):
    DEFAULT_SOCKET_OPTION.append((socket.SOL_TCP, socket.TCP_KEEPIDLE, 30))
if hasattr(socket, "TCP_KEEPINTVL"):
    DEFAULT_SOCKET_OPTION.append((socket.SOL_TCP, socket.TCP_KEEPINTVL, 10))
if hasattr(socket, "TCP_KEEPCNT"):
    DEFAULT_SOCKET_OPTION.append((socket.SOL_TCP, socket.TCP_KEEPCNT, 3))

_default_timeout = None

__all__ = ["DEFAULT_SOCKET_OPTION", "sock_opt", "setdefaulttimeout", "getdefaulttimeout",
           "recv", "recv_line", "send"]


class sock_opt(object):

    def __init__(self, sockopt, sslopt):
        if sockopt is None:
            sockopt = []
        if sslopt is None:
            sslopt = {}
        self.sockopt = sockopt
        self.sslopt = sslopt
        self.timeout = None


def setdefaulttimeout(timeout):
    """
    Set the global timeout setting to connect.

    timeout: default socket timeout time. This value is second.
    """
    global _default_timeout
    _default_timeout = timeout


def getdefaulttimeout():
    """
    Return the global timeout setting(second) to connect.
    """
    return _default_timeout


def recv(sock, bufsize):
    if not sock:
        raise WebSocketConnectionClosedException("socket is already closed.")

    try:
        bytes_ = sock.recv(bufsize)
    except socket.timeout as e:
        message = extract_err_message(e)
        raise WebSocketTimeoutException(message)
    except SSLError as e:
        message = extract_err_message(e)
        if message == "The read operation timed out":
            raise WebSocketTimeoutException(message)
        else:
            raise

    if not bytes_:
        raise WebSocketConnectionClosedException(
            "Connection is already closed.")

    return bytes_


def recv_line(sock):
    line = []
    while True:
        c = recv(sock, 1)
        line.append(c)
        if c == six.b("\n"):
            break
    return six.b("").join(line)


def send(sock, data):
    if isinstance(data, six.text_type):
        data = data.encode('utf-8')

    if not sock:
        raise WebSocketConnectionClosedException("socket is already closed.")

    try:
        return sock.send(data)
    except socket.timeout as e:
        message = extract_err_message(e)
        raise WebSocketTimeoutException(message)
    except Exception as e:
        message = extract_err_message(e)
        if isinstance(message, str) and "timed out" in message:
            raise WebSocketTimeoutException(message)
        else:
            raise
