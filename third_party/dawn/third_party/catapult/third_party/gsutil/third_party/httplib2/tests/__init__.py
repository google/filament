from __future__ import print_function

import base64
import contextlib
import copy
import email.utils
import functools
import gzip
import hashlib
import os
import random
import re
import shutil
import socket
import ssl
import struct
import sys
import threading
import time
import traceback
import zlib

import httplib2
import pytest
import six
from six.moves import http_client, queue, urllib


_missing = object()

x509 = None
if sys.version_info >= (3, 6):
    from cryptography import x509

DUMMY_URL = "http://127.0.0.1:1"
DUMMY_HTTPS_URL = "https://127.0.0.1:2"

tls_dir = os.path.join(os.path.dirname(__file__), "tls")
CA_CERTS = os.path.join(tls_dir, "ca.pem")
CA_UNUSED_CERTS = os.path.join(tls_dir, "ca_unused.pem")
CLIENT_PEM = os.path.join(tls_dir, "client.pem")
CLIENT_ENCRYPTED_PEM = os.path.join(tls_dir, "client_encrypted.pem")
SERVER_PEM = os.path.join(tls_dir, "server.pem")
SERVER_CHAIN = os.path.join(tls_dir, "server_chain.pem")
skip_benchmark = pytest.mark.skipif(
    os.getenv("httplib2_test_bench", "") != "1",
    reason="benchmark disabled by default, set env httplib2_test_bench=1",
)


@contextlib.contextmanager
def assert_raises(exc_type):
    def _name(t):
        return getattr(t, "__name__", None) or str(t)

    if not isinstance(exc_type, tuple):
        exc_type = (exc_type,)
    names = ", ".join(map(_name, exc_type))

    try:
        yield
    except exc_type:
        pass
    else:
        assert False, "Expected exception(s) {0}".format(names)


class BufferedReader(object):
    """io.BufferedReader with \r\n support"""

    def __init__(self, sock):
        self._buf = b""
        self._end = False
        self._newline = b"\r\n"
        self._sock = sock
        if isinstance(sock, bytes):
            self._sock = None
            self._buf = sock

    def _fill(self, target=1, more=None, untilend=False):
        if more:
            target = len(self._buf) + more
        while untilend or (len(self._buf) < target):
            # crutch to enable HttpRequest.from_bytes
            if self._sock is None:
                chunk = b""
            else:
                chunk = self._sock.recv(8 << 10)
            # print("!!! recv", chunk)
            if not chunk:
                self._end = True
                if untilend:
                    return
                else:
                    raise EOFError
            self._buf += chunk

    def peek(self, size):
        self._fill(target=size)
        return self._buf[:size]

    def read(self, size):
        self._fill(target=size)
        chunk, self._buf = self._buf[:size], self._buf[size:]
        return chunk

    def readall(self):
        self._fill(untilend=True)
        chunk, self._buf = self._buf, b""
        return chunk

    def readline(self):
        while True:
            i = self._buf.find(self._newline)
            if i >= 0:
                break
            self._fill(more=1)
        inext = i + len(self._newline)
        line, self._buf = self._buf[:inext], self._buf[inext:]
        return line


def parse_http_message(kind, buf):
    if buf._end:
        return None
    try:
        start_line = buf.readline()
    except EOFError:
        return None
    msg = kind()
    msg.raw = start_line
    if kind is HttpRequest:
        assert re.match(br".+ HTTP/\d\.\d\r\n$", start_line), "Start line does not look like HTTP request: " + repr(
            start_line
        )
        msg.method, msg.uri, msg.proto = start_line.rstrip().decode().split(" ", 2)
        assert msg.proto.startswith("HTTP/"), repr(start_line)
    elif kind is HttpResponse:
        assert re.match(
            br"^HTTP/\d\.\d \d+ .+\r\n$", start_line
        ), "Start line does not look like HTTP response: " + repr(start_line)
        msg.proto, msg.status, msg.reason = start_line.rstrip().decode().split(" ", 2)
        msg.status = int(msg.status)
        assert msg.proto.startswith("HTTP/"), repr(start_line)
    else:
        raise Exception("Use HttpRequest or HttpResponse .from_{bytes,buffered}")
    msg.version = msg.proto[5:]

    while True:
        line = buf.readline()
        msg.raw += line
        line = line.rstrip()
        if not line:
            break
        t = line.decode().split(":", 1)
        msg.headers[t[0].lower()] = t[1].lstrip()

    content_length_string = msg.headers.get("content-length", "")
    if content_length_string.isdigit():
        content_length = int(content_length_string)
        msg.body = msg.body_raw = buf.read(content_length)
    elif msg.headers.get("transfer-encoding") == "chunked":
        raise NotImplemented
    elif msg.version == "1.0":
        msg.body = msg.body_raw = buf.readall()
    else:
        msg.body = msg.body_raw = b""

    msg.raw += msg.body_raw
    return msg


class HttpMessage(object):
    def __init__(self):
        self.headers = {}

    @classmethod
    def from_bytes(cls, bs):
        buf = BufferedReader(bs)
        return parse_http_message(cls, buf)

    @classmethod
    def from_buffered(cls, buf):
        return parse_http_message(cls, buf)

    def __repr__(self):
        return "{} {}".format(self.__class__, repr(vars(self)))


class HttpRequest(HttpMessage):
    pass


class HttpResponse(HttpMessage):
    pass


class MockResponse(six.BytesIO):
    def __init__(self, body, **kwargs):
        six.BytesIO.__init__(self, body)
        self.headers = kwargs

    def items(self):
        return self.headers.items()

    def iteritems(self):
        return six.iteritems(self.headers)


class MockHTTPConnection(object):
    """This class is just a mock of httplib.HTTPConnection used for testing"""

    def __init__(
        self,
        host,
        port=None,
        key_file=None,
        cert_file=None,
        strict=None,
        timeout=None,
        proxy_info=None,
    ):
        self.host = host
        self.port = port
        self.timeout = timeout
        self.log = ""
        self.sock = None

    def set_debuglevel(self, level):
        pass

    def connect(self):
        "Connect to a host on a given port."
        pass

    def close(self):
        pass

    def request(self, method, request_uri, body, headers):
        pass

    def getresponse(self):
        return MockResponse(b"the body", status="200")


class MockHTTPBadStatusConnection(object):
    """Mock of httplib.HTTPConnection that raises BadStatusLine."""

    num_calls = 0

    def __init__(
        self,
        host,
        port=None,
        key_file=None,
        cert_file=None,
        strict=None,
        timeout=None,
        proxy_info=None,
    ):
        self.host = host
        self.port = port
        self.timeout = timeout
        self.log = ""
        self.sock = None
        MockHTTPBadStatusConnection.num_calls = 0

    def set_debuglevel(self, level):
        pass

    def connect(self):
        pass

    def close(self):
        pass

    def request(self, method, request_uri, body, headers):
        pass

    def getresponse(self):
        MockHTTPBadStatusConnection.num_calls += 1
        raise http_client.BadStatusLine("")


@contextlib.contextmanager
def server_socket(fun, request_count=1, timeout=5, scheme="", tls=None):
    """Base socket server for tests.
    Likely you want to use server_request or other higher level helpers.
    All arguments except fun can be passed to other server_* helpers.

    :param fun: fun(client_sock, tick) called after successful accept().
    :param request_count: test succeeds after exactly this number of requests, triggered by tick(request)
    :param timeout: seconds.
    :param scheme: affects yielded value
        "" - build normal http/https URI.
        string - build normal URI using supplied scheme.
        None - yield (addr, port) tuple.
    :param tls:
        None (default) - plain HTTP.
        True - HTTPS with reasonable defaults. Likely you want httplib2.Http(ca_certs=tests.CA_CERTS)
        string - path to custom server cert+key PEM file.
        callable - function(context, listener, skip_errors) -> ssl_wrapped_listener
    """
    gresult = [None]
    gcounter = [0]
    tls_skip_errors = [
        "TLSV1_ALERT_UNKNOWN_CA",
    ]
    assert scheme in ("", "http", "https"), 'tests.server_socket: invalid scheme="{}"'.format(scheme)

    def tick(request):
        gcounter[0] += 1
        keep = True
        keep &= gcounter[0] < request_count
        if request is not None:
            keep &= request.headers.get("connection", "").lower() != "close"
        return keep

    def server_socket_thread(srv):
        try:
            while gcounter[0] < request_count:
                try:
                    client, _ = srv.accept()
                except ssl.SSLError as e:
                    if e.reason in tls_skip_errors:
                        return
                    raise

                try:
                    client.settimeout(timeout)
                    fun(client, tick)
                finally:
                    try:
                        client.shutdown(socket.SHUT_RDWR)
                    except (IOError, socket.error):
                        pass
                    # FIXME: client.close() introduces connection reset by peer
                    # at least in other/connection_close test
                    # should not be a problem since socket would close upon garbage collection
            if gcounter[0] > request_count:
                gresult[0] = Exception("Request count expected={0} actual={1}".format(request_count, gcounter[0]))
        except Exception as e:
            # traceback.print_exc caused IOError: concurrent operation on sys.stderr.close() under setup.py test
            print(traceback.format_exc(), file=sys.stderr)
            gresult[0] = e

    bind_hostname = "localhost"
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind((bind_hostname, 0))
    try:
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    except socket.error as ex:
        print("non critical error on SO_REUSEADDR", ex)
    server.listen(10)
    server.settimeout(timeout)
    server_port = server.getsockname()[1]
    if tls is True:
        tls = SERVER_CHAIN
    if tls:
        # TODO: Drop py3.3 support and replace this with ssl.create_default_context
        ssl_context = ssl.SSLContext(getattr(ssl, "PROTOCOL_TLS", None) or getattr(ssl, "PROTOCOL_SSLv23"))
        # TODO: leave this when py3.3 support is dropped
        # ssl_context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH, cafile=CA_CERTS)

        if callable(tls):
            ssl_context.load_cert_chain(SERVER_CHAIN)
            server = tls(ssl_context, server, tls_skip_errors)
        else:
            ssl_context.load_cert_chain(tls)
            server = ssl_context.wrap_socket(server, server_side=True)
    if scheme == "":
        scheme = "https" if tls else "http"

    t = threading.Thread(target=server_socket_thread, args=(server,))
    t.daemon = True
    t.start()
    if scheme is None:
        yield (bind_hostname, server_port)
    else:
        yield u"{scheme}://{host}:{port}/".format(scheme=scheme, host=bind_hostname, port=server_port)
    server.close()
    t.join()
    if gresult[0] is not None:
        raise gresult[0]


def server_yield(fun, **kwargs):
    q = queue.Queue(1)
    g = fun(q.get)

    def server_yield_socket_handler(sock, tick):
        buf = BufferedReader(sock)
        i = 0
        while True:
            request = HttpRequest.from_buffered(buf)
            if request is None:
                break
            i += 1
            request.client_sock = sock
            request.number = i
            q.put(request)
            response = six.next(g)
            sock.sendall(response)
            request.client_sock = None
            if not tick(request):
                break

    return server_socket(server_yield_socket_handler, **kwargs)


def server_request(request_handler, **kwargs):
    def server_request_socket_handler(sock, tick):
        buf = BufferedReader(sock)
        i = 0
        while True:
            request = HttpRequest.from_buffered(buf)
            if request is None:
                break
            # print("--- debug request\n" + request.raw.decode("ascii", "replace"))
            i += 1
            request.client_sock = sock
            request.number = i
            response = request_handler(request=request)
            # print("--- debug response\n" + response.decode("ascii", "replace"))
            sock.sendall(response)
            request.client_sock = None
            if not tick(request):
                break

    return server_socket(server_request_socket_handler, **kwargs)


def server_const_bytes(response_content, **kwargs):
    return server_request(lambda request: response_content, **kwargs)


_http_kwargs = (
    "proto",
    "status",
    "headers",
    "body",
    "add_content_length",
    "add_date",
    "add_etag",
    "undefined_body_length",
)


def http_response_bytes(
    proto="HTTP/1.1",
    status="200 OK",
    headers=None,
    body=b"",
    add_content_length=True,
    add_date=False,
    add_etag=False,
    undefined_body_length=False,
    **kwargs
):
    if undefined_body_length:
        add_content_length = False
    if headers is None:
        headers = {}
    if add_content_length:
        headers.setdefault("content-length", str(len(body)))
    if add_date:
        headers.setdefault("date", email.utils.formatdate())
    if add_etag:
        headers.setdefault("etag", '"{0}"'.format(hashlib.md5(body).hexdigest()))
    header_string = "".join("{0}: {1}\r\n".format(k, v) for k, v in headers.items())
    if not undefined_body_length and proto != "HTTP/1.0" and "content-length" not in headers:
        raise Exception("httplib2.tests.http_response_bytes: client could not figure response body length")
    if str(status).isdigit():
        status = "{} {}".format(status, http_client.responses[status])
    response = (
        "{proto} {status}\r\n{headers}\r\n".format(proto=proto, status=status, headers=header_string).encode() + body
    )
    return response


def make_http_reflect(**kwargs):
    assert "body" not in kwargs, "make_http_reflect will overwrite response " "body"

    def fun(request):
        kw = copy.deepcopy(kwargs)
        kw["body"] = request.raw
        response = http_response_bytes(**kw)
        return response

    return fun


def server_route(routes, **kwargs):
    response_404 = http_response_bytes(status="404 Not Found")
    response_wildcard = routes.get("")

    def handler(request):
        target = routes.get(request.uri, response_wildcard) or response_404
        if callable(target):
            response = target(request=request)
        else:
            response = target
        return response

    return server_request(handler, **kwargs)


def server_const_http(**kwargs):
    response_kwargs = {k: kwargs.pop(k) for k in dict(kwargs) if k in _http_kwargs}
    response = http_response_bytes(**response_kwargs)
    return server_const_bytes(response, **kwargs)


def server_list_http(responses, **kwargs):
    i = iter(responses)

    def handler(request):
        return next(i)

    kwargs.setdefault("request_count", len(responses))
    return server_request(handler, **kwargs)


def server_reflect(**kwargs):
    response_kwargs = {k: kwargs.pop(k) for k in dict(kwargs) if k in _http_kwargs}
    http_handler = make_http_reflect(**response_kwargs)
    return server_request(http_handler, **kwargs)


def store_request_response(out):
    def wrapper(fun):
        @functools.wraps(fun)
        def wrapped(request, *a, **kw):
            response_bytes = fun(request, *a, **kw)
            if out is not None:
                response = HttpResponse.from_bytes(response_bytes)
                out.append((request, response))
            return response_bytes

        return wrapped

    return wrapper


def http_reflect_with_auth(allow_scheme, allow_credentials, out_renew_nonce=None, out_requests=None):
    """allow_scheme - 'basic', 'digest', etc allow_credentials - sequence of ('name', 'password') out_renew_nonce - None | [function]

        Way to return nonce renew function to caller.
        Kind of `out` parameter in some programming languages.
        Allows to keep same signature for all handler builder functions.
    out_requests - None | []
        If set to list, every parsed request will be appended here.
    """
    glastnc = [None]
    gnextnonce = [None]
    gserver_nonce = [gen_digest_nonce(salt=b"n")]
    realm = "httplib2 test"
    server_opaque = gen_digest_nonce(salt=b"o")

    def renew_nonce():
        if gnextnonce[0]:
            assert False, "previous nextnonce was not used, probably bug in test code"
        gnextnonce[0] = gen_digest_nonce()
        return gserver_nonce[0], gnextnonce[0]

    if out_renew_nonce:
        out_renew_nonce[0] = renew_nonce

    def deny(**kwargs):
        nonce_stale = kwargs.pop("nonce_stale", False)
        if nonce_stale:
            kwargs.setdefault("body", b"nonce stale")
        if allow_scheme == "basic":
            authenticate = 'basic realm="{realm}"'.format(realm=realm)
        elif allow_scheme == "digest":
            authenticate = (
                'digest realm="{realm}", qop="auth"'
                + ', nonce="{nonce}", opaque="{opaque}"'
                + (", stale=true" if nonce_stale else "")
            ).format(realm=realm, nonce=gserver_nonce[0], opaque=server_opaque)
        elif allow_scheme == "wsse":
            authenticate = 'wsse realm="{realm}", profile="UsernameToken"'.format(realm=realm)
        else:
            raise Exception("unknown allow_scheme={0}".format(allow_scheme))
        deny_headers = {"www-authenticate": authenticate}
        kwargs.setdefault("status", 401)
        # supplied headers may overwrite generated ones
        deny_headers.update(kwargs.get("headers", {}))
        kwargs["headers"] = deny_headers
        kwargs.setdefault("body", b"HTTP authorization required")
        return http_response_bytes(**kwargs)

    @store_request_response(out_requests)
    def http_reflect_with_auth_handler(request):
        auth_header = request.headers.get("authorization", "")
        if not auth_header:
            return deny()
        try:
            auth_parsed = httplib2.auth._parse_www_authenticate(request.headers, "authorization")
            print("debug: auth_parsed", auth_parsed)
        except httplib2.error.MalformedHeader:
            print("debug: auth header error")
            return http_response_bytes(status=400, body=b"authorization header syntax error")
        scheme = auth_header.split(" ", 1)[0].lower()
        print("debug: first auth scheme='{}'".format(scheme))
        if scheme != allow_scheme:
            return deny(body=b"must use different auth scheme")
        auth_info = auth_parsed[scheme]
        if scheme == "basic":
            decoded = base64.b64decode(auth_info["token"]).decode()
            username, password = decoded.split(":", 1)
            if (username, password) in allow_credentials:
                return make_http_reflect()(request)
            else:
                return deny(body=b"supplied credentials are not allowed")
        elif scheme == "digest":
            server_nonce_old = gserver_nonce[0]
            nextnonce = gnextnonce[0]
            if nextnonce:
                # server decided to change nonce, in this case, guided by caller test code
                gserver_nonce[0] = nextnonce
                gnextnonce[0] = None
            server_nonce_current = gserver_nonce[0]
            client_cnonce = auth_info.get("cnonce", "")
            client_nc = auth_info.get("nc", "")
            client_nonce = auth_info.get("nonce", "")
            client_opaque = auth_info.get("opaque", "")
            client_qop = auth_info.get("qop", "auth").strip('"')

            # TODO: auth_info.get('algorithm', 'md5')
            hasher = hashlib.md5

            # TODO: client_qop auth-int
            ha2 = hasher(":".join((request.method, request.uri)).encode()).hexdigest()

            if client_nonce != server_nonce_current:
                if client_nonce == server_nonce_old:
                    return deny(nonce_stale=True)
                return deny(body=b"invalid nonce")
            if not client_nc:
                return deny(body=b"auth-info nc missing")
            if client_opaque != server_opaque:
                return deny(
                    body="auth-info opaque mismatch expected={} actual={}".format(server_opaque, client_opaque).encode()
                )
            for allow_username, allow_password in allow_credentials:
                ha1 = hasher(":".join((allow_username, realm, allow_password)).encode()).hexdigest()
                allow_response = hasher(
                    ":".join((ha1, client_nonce, client_nc, client_cnonce, client_qop, ha2)).encode()
                ).hexdigest()
                rspauth_ha2 = hasher(":{}".format(request.uri).encode()).hexdigest()
                rspauth = hasher(
                    ":".join(
                        (
                            ha1,
                            client_nonce,
                            client_nc,
                            client_cnonce,
                            client_qop,
                            rspauth_ha2,
                        )
                    ).encode()
                ).hexdigest()
                if auth_info.get("response", "") == allow_response:
                    # TODO: fix or remove doubtful comment
                    # do we need to save nc only on success?
                    glastnc[0] = client_nc
                    allow_headers = {
                        "authentication-info": ", ".join(
                            filter(
                                None,
                                (
                                    'nextnonce="{}"'.format(nextnonce) if nextnonce else "",
                                    "qop={}".format(client_qop),
                                    'rspauth="{}"'.format(rspauth),
                                    'cnonce="{}"'.format(client_cnonce),
                                    "nc={}".format(client_nc),
                                ),
                            )
                        ).strip()
                    }
                    return make_http_reflect(headers=allow_headers)(request)
            return deny(body=b"supplied credentials are not allowed")
        elif scheme == "wsse":
            x_wsse = request.headers.get("x-wsse", "")
            if x_wsse.count(",") != 3:
                return http_response_bytes(status=400, body=b"x-wsse header syntax error")
            wsse_params = httplib2.auth._parse_www_authenticate(request.headers, "x-wsse").get("usernametoken", {})
            print("debug: wsse_params", wsse_params)
            client_username = wsse_params.get("username", "")
            client_nonce = wsse_params.get("nonce", "")
            client_created = wsse_params.get("created", "")
            client_digest = wsse_params.get("passworddigest", "")
            allow_password = None
            for allow_username, allow_password in allow_credentials:
                if client_username == allow_username:
                    break
            else:
                return deny(body=b"unknown username")

            digest = hashlib.sha1("".join((client_nonce, client_created, allow_password)).encode("utf-8")).digest()
            digest_b64 = base64.b64encode(digest).decode()
            print("debug: check client={} == real={}".format(client_digest, digest_b64))
            if client_digest == digest_b64:
                return make_http_reflect()(request)

            return deny(body=b"supplied credentials are not allowed")
        else:
            return http_response_bytes(
                status=400,
                body="unknown authorization scheme={0}".format(scheme).encode(),
            )

    return http_reflect_with_auth_handler


def get_cache_path():
    default = "./_httplib2_test_cache"
    path = os.environ.get("httplib2_test_cache_path") or default
    if os.path.exists(path):
        shutil.rmtree(path)
    return path


def gen_digest_nonce(salt=b""):
    t = struct.pack(">Q", int(time.time() * 1e9))
    return base64.b64encode(t + b":" + hashlib.sha1(t + salt).digest()).decode()


def gen_password():
    length = random.randint(8, 64)
    return "".join(six.unichr(random.randint(0, 127)) for _ in range(length))


def gzip_compress(bs):
    # gzipobj = zlib.compressobj(9, zlib.DEFLATED, zlib.MAX_WBITS | 16)
    # result = gzipobj.compress(text) + gzipobj.flush()
    buf = six.BytesIO()
    gf = gzip.GzipFile(fileobj=buf, mode="wb", compresslevel=6)
    gf.write(bs)
    gf.close()
    return buf.getvalue()


def gzip_decompress(bs):
    return zlib.decompress(bs, zlib.MAX_WBITS | 16)


def deflate_compress(bs):
    do = zlib.compressobj(9, zlib.DEFLATED, -zlib.MAX_WBITS)
    return do.compress(bs) + do.flush()


def deflate_decompress(bs):
    return zlib.decompress(bs, -zlib.MAX_WBITS)


def ssl_context(protocol=None):
    """Workaround for old SSLContext() required protocol argument."""
    if sys.version_info < (3, 5, 3):
        return ssl.SSLContext(ssl.PROTOCOL_SSLv23)
    return ssl.SSLContext()


def memoize(fun):
    # functools.cache 3.9+
    f = getattr(functools, "cache", None)
    if f:
        return f(fun)

    # functools.lru_cache 3.2+
    f = getattr(functools, "lru_cache", None)
    if f:
        return f()(fun)

    # < 3.2
    return fun


@memoize
def x509_serial(path):
    if x509 is None:
        raise Exception("x509_serial requires package cryptography installed")
    with open(path, "rb") as f:
        pem = f.read()
    cert = x509.load_pem_x509_certificate(pem)
    return cert.serial_number


def rebuild_uri(old, scheme=_missing, netloc=_missing, host=_missing, port=_missing, path=_missing):
    if "//" not in old:
        old = "//" + old
    u = urllib.parse.urlsplit(old)
    if scheme is _missing:
        scheme = u.scheme
    if netloc is _missing:
        netloc = u.netloc
    if host is _missing:
        host = u.hostname if not u.netloc.startswith("[") else "[{}]".format(u.hostname)
    if port is _missing:
        port = u.port
    netloc = host if port is None else "{}:{}".format(host, port)
    if path is _missing:
        path = u.path
    new = (scheme, netloc, path) + u[3:]
    return urllib.parse.urlunsplit(new)
