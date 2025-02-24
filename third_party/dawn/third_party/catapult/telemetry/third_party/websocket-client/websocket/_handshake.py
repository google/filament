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
    Boston, MA  02110-1335  USA

"""
from __future__ import absolute_import
import hashlib
import hmac
import os

import six

from ._exceptions import *
from ._http import *
from ._logging import *
from ._socket import *
from six.moves import map

if six.PY3:
    from base64 import encodebytes as base64encode
else:
    from base64 import encodestring as base64encode

__all__ = ["handshake_response", "handshake"]

if hasattr(hmac, "compare_digest"):
    compare_digest = hmac.compare_digest
else:
    def compare_digest(s1, s2):
        return s1 == s2

# websocket supported version.
VERSION = 13


class handshake_response(object):

    def __init__(self, status, headers, subprotocol):
        self.status = status
        self.headers = headers
        self.subprotocol = subprotocol


def handshake(sock, hostname, port, resource, **options):
    headers, key = _get_handshake_headers(resource, hostname, port, options)

    header_str = "\r\n".join(headers)
    send(sock, header_str)
    dump("request header", header_str)

    status, resp = _get_resp_headers(sock)
    success, subproto = _validate(resp, key, options.get("subprotocols"))
    if not success:
        raise WebSocketException("Invalid WebSocket Header")

    return handshake_response(status, resp, subproto)


def _get_handshake_headers(resource, host, port, options):
    headers = [
        "GET %s HTTP/1.1" % resource,
        "Upgrade: websocket",
        "Connection: Upgrade"
    ]
    if port == 80 or port == 443:
        hostport = host
    else:
        hostport = "%s:%d" % (host, port)

    if "host" in options and options["host"]:
        headers.append("Host: %s" % options["host"])
    else:
        headers.append("Host: %s" % hostport)

    key = _create_sec_websocket_key()
    headers.append("Sec-WebSocket-Key: %s" % key)
    headers.append("Sec-WebSocket-Version: %s" % VERSION)

    subprotocols = options.get("subprotocols")
    if subprotocols:
        headers.append("Sec-WebSocket-Protocol: %s" % ",".join(subprotocols))

    if "header" in options:
        header = options["header"]
        if isinstance(header, dict):
            header = list(map(": ".join, list(header.items())))
        headers.extend(header)

    cookie = options.get("cookie", None)

    if cookie:
        headers.append("Cookie: %s" % cookie)

    headers.append("")
    headers.append("")

    return headers, key


def _get_resp_headers(sock, success_status=101):
    status, resp_headers = read_headers(sock)
    if status != success_status:
        raise WebSocketBadStatusException("Handshake status %d", status)
    return status, resp_headers

_HEADERS_TO_CHECK = {
    "upgrade": "websocket",
    "connection": "upgrade",
}


def _validate(headers, key, subprotocols):
    subproto = None
    for k, v in _HEADERS_TO_CHECK.items():
        r = headers.get(k, None)
        if not r:
            return False, None
        r = r.lower()
        if v != r:
            return False, None

    if subprotocols:
        subproto = headers.get("sec-websocket-protocol", None).lower()
        if not subproto or subproto not in [s.lower() for s in subprotocols]:
            error("Invalid subprotocol: " + str(subprotocols))
            return False, None

    result = headers.get("sec-websocket-accept", None)
    if not result:
        return False, None
    result = result.lower()

    if isinstance(result, six.text_type):
        result = result.encode('utf-8')

    value = (key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").encode('utf-8')
    hashed = base64encode(hashlib.sha1(value).digest()).strip().lower()
    success = compare_digest(hashed, result)

    if success:
        return True, subproto
    else:
        return False, None


def _create_sec_websocket_key():
    randomness = os.urandom(16)
    return base64encode(randomness).decode('utf-8').strip()
