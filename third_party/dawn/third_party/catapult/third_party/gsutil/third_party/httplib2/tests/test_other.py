import httplib2

try:
    from unittest import mock
except ImportError:
    import mock
import pickle
import pytest
import socket
import sys
import tests
import time
from six.moves import urllib


@pytest.mark.skipif(
    sys.version_info <= (3,),
    reason=(
        "TODO: httplib2._convert_byte_str was defined only in python3 code " "version"
    ),
)
def test_convert_byte_str():
    with tests.assert_raises(TypeError):
        httplib2._convert_byte_str(4)
    assert httplib2._convert_byte_str(b"Hello") == "Hello"
    assert httplib2._convert_byte_str("World") == "World"


def test_reflect():
    http = httplib2.Http()
    with tests.server_reflect() as uri:
        response, content = http.request(uri + "?query", "METHOD")
    assert response.status == 200
    host = urllib.parse.urlparse(uri).netloc
    assert content.startswith(
        """\
METHOD /?query HTTP/1.1\r\n\
Host: {host}\r\n""".format(
            host=host
        ).encode()
    ), content


def test_pickle_http():
    http = httplib2.Http(cache=tests.get_cache_path())
    new_http = pickle.loads(pickle.dumps(http))

    assert tuple(sorted(new_http.__dict__)) == tuple(sorted(http.__dict__))
    assert new_http.credentials.credentials == http.credentials.credentials
    assert new_http.certificates.credentials == http.certificates.credentials
    assert new_http.cache.cache == http.cache.cache
    for key in new_http.__dict__:
        if key not in ("cache", "certificates", "credentials"):
            assert getattr(new_http, key) == getattr(http, key)


def test_pickle_http_with_connection():
    http = httplib2.Http()
    http.request("http://random-domain:81/", connection_type=tests.MockHTTPConnection)
    new_http = pickle.loads(pickle.dumps(http))
    assert tuple(http.connections) == ("http:random-domain:81",)
    assert new_http.connections == {}


def test_pickle_custom_request_http():
    http = httplib2.Http()
    http.request = lambda: None
    http.request.dummy_attr = "dummy_value"
    new_http = pickle.loads(pickle.dumps(http))
    assert getattr(new_http.request, "dummy_attr", None) is None


@pytest.mark.xfail(
    sys.version_info >= (3,),
    reason=(
        "FIXME: for unknown reason global timeout test fails in Python3 "
        "with response 200"
    ),
)
def test_timeout_global():
    def handler(request):
        time.sleep(0.5)
        return tests.http_response_bytes()

    try:
        socket.setdefaulttimeout(0.1)
    except Exception:
        pytest.skip("cannot set global socket timeout")
    try:
        http = httplib2.Http()
        http.force_exception_to_status_code = True
        with tests.server_request(handler) as uri:
            response, content = http.request(uri)
            assert response.status == 408
            assert response.reason.startswith("Request Timeout")
    finally:
        socket.setdefaulttimeout(None)


def test_timeout_individual():
    def handler(request):
        time.sleep(0.5)
        return tests.http_response_bytes()

    http = httplib2.Http(timeout=0.1)
    http.force_exception_to_status_code = True

    with tests.server_request(handler) as uri:
        response, content = http.request(uri)
        assert response.status == 408
        assert response.reason.startswith("Request Timeout")


def test_timeout_subsequent():
    class Handler(object):
        number = 0

        @classmethod
        def handle(cls, request):
            # request.number is always 1 because of
            # the new socket connection each time
            cls.number += 1
            if cls.number % 2 != 0:
                time.sleep(0.6)
                return tests.http_response_bytes(status=500)
            return tests.http_response_bytes(status=200)

    http = httplib2.Http(timeout=0.5)
    http.force_exception_to_status_code = True

    with tests.server_request(Handler.handle, request_count=2) as uri:
        response, _ = http.request(uri)
        assert response.status == 408
        assert response.reason.startswith("Request Timeout")

        response, _ = http.request(uri)
        assert response.status == 200


def test_timeout_https():
    c = httplib2.HTTPSConnectionWithTimeout("localhost", 80, timeout=47)
    assert 47 == c.timeout


# @pytest.mark.xfail(
#     sys.version_info >= (3,),
#     reason='[py3] last request should open new connection, but client does not realize socket was closed by server',
# )
def test_connection_close():
    http = httplib2.Http()
    g = []

    def handler(request):
        g.append(request.number)
        return tests.http_response_bytes(proto="HTTP/1.1")

    with tests.server_request(handler, request_count=3) as uri:
        http.request(uri, "GET")  # conn1 req1
        for c in http.connections.values():
            assert c.sock is not None
        http.request(uri, "GET", headers={"connection": "close"})
        time.sleep(0.7)
        http.request(uri, "GET")  # conn2 req1
    assert g == [1, 2, 1]


def test_get_end2end_headers():
    # one end to end header
    response = {"content-type": "application/atom+xml", "te": "deflate"}
    end2end = httplib2._get_end2end_headers(response)
    assert "content-type" in end2end
    assert "te" not in end2end
    assert "connection" not in end2end

    # one end to end header that gets eliminated
    response = {
        "connection": "content-type",
        "content-type": "application/atom+xml",
        "te": "deflate",
    }
    end2end = httplib2._get_end2end_headers(response)
    assert "content-type" not in end2end
    assert "te" not in end2end
    assert "connection" not in end2end

    # Degenerate case of no headers
    response = {}
    end2end = httplib2._get_end2end_headers(response)
    assert len(end2end) == 0

    # Degenerate case of connection referrring to a header not passed in
    response = {"connection": "content-type"}
    end2end = httplib2._get_end2end_headers(response)
    assert len(end2end) == 0


# @pytest.mark.xfail(
#     os.environ.get("TRAVIS_PYTHON_VERSION") in ("2.7", "pypy"),
#     reason="FIXME: fail on Travis py27 and pypy, works elsewhere",
# )
@pytest.mark.parametrize("scheme", ("http", "https"))
def test_ipv6(scheme):
    # Even if IPv6 isn't installed on a machine it should just raise socket.error
    uri = "{scheme}://[::1]:1/".format(scheme=scheme)
    try:
        httplib2.Http(timeout=0.1).request(uri)
    except socket.gaierror:
        assert False, "should get the address family right for IPv6"
    except socket.error:
        pass


@pytest.mark.parametrize(
    "conn_type",
    (httplib2.HTTPConnectionWithTimeout, httplib2.HTTPSConnectionWithTimeout),
)
def test_connection_proxy_info_attribute_error(conn_type):
    # HTTPConnectionWithTimeout did not initialize its .proxy_info attribute
    # https://github.com/httplib2/httplib2/pull/97
    # Thanks to Joseph Ryan https://github.com/germanjoey
    conn = conn_type("no-such-hostname.", 80)
    # TODO: replace mock with dummy local server
    with tests.assert_raises(socket.gaierror):
        with mock.patch("socket.socket.connect", side_effect=socket.gaierror):
            conn.request("GET", "/")


def test_http_443_forced_https():
    http = httplib2.Http()
    http.force_exception_to_status_code = True
    uri = "http://localhost:443/"
    # sorry, using internal structure of Http to check chosen scheme
    with mock.patch("httplib2.Http._request") as m:
        http.request(uri)
        assert len(m.call_args) > 0, "expected Http._request() call"
        conn = m.call_args[0][0]
        assert isinstance(conn, httplib2.HTTPConnectionWithTimeout)


def test_close():
    http = httplib2.Http()
    assert len(http.connections) == 0
    with tests.server_const_http() as uri:
        http.request(uri)
        assert len(http.connections) == 1
        http.close()
        assert len(http.connections) == 0


def test_connect_exception_type():
    # This autoformatting PR actually changed the behavior of error handling:
    # https://github.com/httplib2/httplib2/pull/105/files#diff-c6669c781a2dee1b2d2671cab4e21c66L985
    # potentially changing the type of the error raised by connect()
    # https://github.com/httplib2/httplib2/pull/150
    http = httplib2.Http()
    with mock.patch("httplib2.socket.socket.connect", side_effect=socket.timeout("foo")):
        with tests.assert_raises(socket.timeout):
            http.request(tests.DUMMY_URL)
