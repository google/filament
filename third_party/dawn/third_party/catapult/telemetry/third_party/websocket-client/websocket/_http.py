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
import errno
import os
import socket
import sys

import six

from ._exceptions import *
from ._logging import *
from ._socket import*
from ._ssl_compat import *
from ._url import *

if six.PY3:
    from base64 import encodebytes as base64encode
else:
    from base64 import encodestring as base64encode

__all__ = ["proxy_info", "connect", "read_headers"]


class proxy_info(object):

    def __init__(self, **options):
        self.host = options.get("http_proxy_host", None)
        if self.host:
            self.port = options.get("http_proxy_port", 0)
            self.auth = options.get("http_proxy_auth", None)
            self.no_proxy = options.get("http_no_proxy", None)
        else:
            self.port = 0
            self.auth = None
            self.no_proxy = None


def connect(url, options, proxy, socket):
    hostname, port, resource, is_secure = parse_url(url)

    if socket:
        return socket, (hostname, port, resource)

    addrinfo_list, need_tunnel, auth = _get_addrinfo_list(
        hostname, port, is_secure, proxy)
    if not addrinfo_list:
        raise WebSocketException(
            "Host not found.: " + hostname + ":" + str(port))

    sock = None
    try:
        sock = _open_socket(addrinfo_list, options.sockopt, options.timeout)
        if need_tunnel:
            sock = _tunnel(sock, hostname, port, auth)

        if is_secure:
            if HAVE_SSL:
                sock = _ssl_socket(sock, options.sslopt, hostname)
            else:
                raise WebSocketException("SSL not available.")

        return sock, (hostname, port, resource)
    except:
        if sock:
            sock.close()
        raise


def _get_addrinfo_list(hostname, port, is_secure, proxy):
    phost, pport, pauth = get_proxy_info(
        hostname, is_secure, proxy.host, proxy.port, proxy.auth, proxy.no_proxy)
    if not phost:
        addrinfo_list = socket.getaddrinfo(
            hostname, port, 0, 0, socket.SOL_TCP)
        return addrinfo_list, False, None
    else:
        pport = pport and pport or 80
        addrinfo_list = socket.getaddrinfo(phost, pport, 0, 0, socket.SOL_TCP)
        return addrinfo_list, True, pauth


def _open_socket(addrinfo_list, sockopt, timeout):
    err = None
    for addrinfo in addrinfo_list:
        family = addrinfo[0]
        sock = socket.socket(family)
        sock.settimeout(timeout)
        for opts in DEFAULT_SOCKET_OPTION:
            sock.setsockopt(*opts)
        for opts in sockopt:
            sock.setsockopt(*opts)

        address = addrinfo[4]
        try:
            sock.connect(address)
        except socket.error as error:
            error.remote_ip = str(address[0])
            if error.errno in (errno.ECONNREFUSED, ):
                err = error
                continue
            else:
                raise
        else:
            break
    else:
        raise err

    return sock


def _can_use_sni():
    return six.PY2 and sys.version_info >= (2, 7, 9) or sys.version_info >= (3, 2)


def _wrap_sni_socket(sock, sslopt, hostname, check_hostname):
    context = ssl.SSLContext(sslopt.get('ssl_version', ssl.PROTOCOL_SSLv23))

    if sslopt.get('cert_reqs', ssl.CERT_NONE) != ssl.CERT_NONE:
        context.load_verify_locations(cafile=sslopt.get('ca_certs', None))
    if sslopt.get('certfile', None):
        context.load_cert_chain(
            sslopt['certfile'],
            sslopt.get('keyfile', None),
            sslopt.get('password', None),
        )
    # see
    # https://github.com/liris/websocket-client/commit/b96a2e8fa765753e82eea531adb19716b52ca3ca#commitcomment-10803153
    context.verify_mode = sslopt['cert_reqs']
    if HAVE_CONTEXT_CHECK_HOSTNAME:
        context.check_hostname = check_hostname
    if 'ciphers' in sslopt:
        context.set_ciphers(sslopt['ciphers'])
    if 'cert_chain' in sslopt:
        certfile, keyfile, password = sslopt['cert_chain']
        context.load_cert_chain(certfile, keyfile, password)

    return context.wrap_socket(
        sock,
        do_handshake_on_connect=sslopt.get('do_handshake_on_connect', True),
        suppress_ragged_eofs=sslopt.get('suppress_ragged_eofs', True),
        server_hostname=hostname,
    )


def _ssl_socket(sock, user_sslopt, hostname):
    sslopt = dict(cert_reqs=ssl.CERT_REQUIRED)
    sslopt.update(user_sslopt)

    if os.environ.get('WEBSOCKET_CLIENT_CA_BUNDLE'):
        certPath = os.environ.get('WEBSOCKET_CLIENT_CA_BUNDLE')
    else:
        certPath = os.path.join(
            os.path.dirname(__file__), "cacert.pem")
    if os.path.isfile(certPath) and user_sslopt.get('ca_certs', None) is None:
        sslopt['ca_certs'] = certPath
    check_hostname = sslopt["cert_reqs"] != ssl.CERT_NONE and sslopt.pop(
        'check_hostname', True)

    if _can_use_sni():
        sock = _wrap_sni_socket(sock, sslopt, hostname, check_hostname)
    else:
        sslopt.pop('check_hostname', True)
        sock = ssl.wrap_socket(sock, **sslopt)

    if not HAVE_CONTEXT_CHECK_HOSTNAME and check_hostname:
        match_hostname(sock.getpeercert(), hostname)

    return sock


def _tunnel(sock, host, port, auth):
    debug("Connecting proxy...")
    connect_header = "CONNECT %s:%d HTTP/1.0\r\n" % (host, port)
    # TODO: support digest auth.
    if auth and auth[0]:
        auth_str = auth[0]
        if auth[1]:
            auth_str += ":" + auth[1]
        encoded_str = base64encode(auth_str.encode()).strip().decode()
        connect_header += "Proxy-Authorization: Basic %s\r\n" % encoded_str
    connect_header += "\r\n"
    dump("request header", connect_header)

    send(sock, connect_header)

    try:
        status, resp_headers = read_headers(sock)
    except Exception as e:
        raise WebSocketProxyException(str(e))

    if status != 200:
        raise WebSocketProxyException(
            "failed CONNECT via proxy status: %r" % status)

    return sock


def read_headers(sock):
    status = None
    headers = {}
    trace("--- response header ---")

    while True:
        line = recv_line(sock)
        line = line.decode('utf-8').strip()
        if not line:
            break
        trace(line)
        if not status:

            status_info = line.split(" ", 2)
            status = int(status_info[1])
        else:
            kv = line.split(":", 1)
            if len(kv) == 2:
                key, value = kv
                headers[key.lower()] = value.strip()
            else:
                raise WebSocketException("Invalid header")

    trace("-----------------------")

    return status, headers
