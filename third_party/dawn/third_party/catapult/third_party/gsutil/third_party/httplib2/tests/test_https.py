import socket
import ssl

import pytest
from six.moves import urllib

import httplib2
import tests


def test_get_via_https():
    # Test that we can handle HTTPS
    http = httplib2.Http(ca_certs=tests.CA_CERTS)
    with tests.server_const_http(tls=True) as uri:
        response, _ = http.request(uri, "GET")
        assert response.status == 200


def test_get_301_via_https():
    http = httplib2.Http(ca_certs=tests.CA_CERTS)
    glocation = [""]  # nonlocal kind of trick, maybe redundant

    def handler(request):
        if request.uri == "/final":
            return tests.http_response_bytes(body=b"final")
        return tests.http_response_bytes(status="301 goto", headers={"location": glocation[0]})

    with tests.server_request(handler, request_count=2, tls=True) as uri:
        glocation[0] = urllib.parse.urljoin(uri, "/final")
        response, content = http.request(uri, "GET")
        assert response.status == 200
        assert content == b"final"
        assert response.previous.status == 301
        assert response.previous["location"] == glocation[0]


def test_get_301_via_https_spec_violation_on_location():
    # Test that we follow redirects through HTTPS
    # even if they violate the spec by including
    # a relative Location: header instead of an absolute one.
    http = httplib2.Http(ca_certs=tests.CA_CERTS)

    def handler(request):
        if request.uri == "/final":
            return tests.http_response_bytes(body=b"final")
        return tests.http_response_bytes(status="301 goto", headers={"location": "/final"})

    with tests.server_request(handler, request_count=2, tls=True) as uri:
        response, content = http.request(uri, "GET")
        assert response.status == 200
        assert content == b"final"
        assert response.previous.status == 301


def test_invalid_ca_certs_path():
    http = httplib2.Http(ca_certs="/nosuchfile")
    with tests.server_const_http(request_count=0, tls=True) as uri:
        with tests.assert_raises(IOError):
            http.request(uri, "GET")


def test_not_trusted_ca():
    # Test that we get a SSLHandshakeError if we try to access
    # server using a CA cert file that doesn't contain server's CA.
    http = httplib2.Http(ca_certs=tests.CA_UNUSED_CERTS)
    with tests.server_const_http(tls=True) as uri:
        try:
            http.request(uri, "GET")
            assert False, "expected CERTIFICATE_VERIFY_FAILED"
        except ssl.SSLError as e:
            assert e.reason == "CERTIFICATE_VERIFY_FAILED"
        except httplib2.SSLHandshakeError:  # Python2
            pass


ssl_context_accept_version = hasattr(tests.ssl_context(), "maximum_version") and hasattr(
    tests.ssl_context(), "minimum_version"
)


@pytest.mark.skipif(not ssl_context_accept_version, reason="ssl doesn't support TLS min/max")
@pytest.mark.parametrize("attr", ("maximum_version", "minimum_version"))
@pytest.mark.parametrize("version", (None, "TLSv1_2", ssl.TLSVersion.TLSv1_2) if ssl_context_accept_version else (None,))
def test_set_tls_version(attr, version):
    # We expect failure on Python < 3.7 or OpenSSL < 1.1
    expect_success = hasattr(ssl.SSLContext(), attr)
    kwargs = {"tls_" + attr: version}
    http = httplib2.Http(**kwargs)
    try:
        http.request(tests.DUMMY_HTTPS_URL)
    except RuntimeError:
        assert not expect_success
    except socket.error:
        assert expect_success


@pytest.mark.skipif(
    not hasattr(tests.ssl_context(), "maximum_version"),
    reason="ssl doesn't support TLS min/max",
)
def test_max_tls_version():
    http = httplib2.Http(ca_certs=tests.CA_CERTS, tls_maximum_version="TLSv1_2")
    with tests.server_const_http(tls=True) as uri:
        http.request(uri)
        _, tls_ver, _ = http.connections.popitem()[1].sock.cipher()
        assert "TLSv1.0" <= tls_ver <= "TLSv1.2"


def test_client_cert_verified():
    cert_log = []

    def setup_tls(context, server, skip_errors):
        context.load_verify_locations(cafile=tests.CA_CERTS)
        context.verify_mode = ssl.CERT_REQUIRED
        return context.wrap_socket(server, server_side=True)

    def handler(request):
        cert_log.append(request.client_sock.getpeercert())
        return tests.http_response_bytes()

    http = httplib2.Http(ca_certs=tests.CA_CERTS)
    with tests.server_request(handler, tls=setup_tls) as uri:
        uri_parsed = urllib.parse.urlparse(uri)
        http.add_certificate(tests.CLIENT_PEM, tests.CLIENT_PEM, uri_parsed.netloc)
        http.request(uri)

    assert len(cert_log) == 1
    expect_serial = tests.x509_serial(tests.CLIENT_PEM) if tests.x509 else 16332984194609126127
    assert int(cert_log[0]["serialNumber"], base=16) == expect_serial


def test_client_cert_password_verified():
    cert_log = []

    def setup_tls(context, server, skip_errors):
        context.load_verify_locations(cafile=tests.CA_CERTS)
        context.verify_mode = ssl.CERT_REQUIRED
        return context.wrap_socket(server, server_side=True)

    def handler(request):
        cert_log.append(request.client_sock.getpeercert())
        return tests.http_response_bytes()

    http = httplib2.Http(ca_certs=tests.CA_CERTS)
    with tests.server_request(handler, tls=setup_tls) as uri:
        uri_parsed = urllib.parse.urlparse(uri)
        http.add_certificate(tests.CLIENT_ENCRYPTED_PEM, tests.CLIENT_ENCRYPTED_PEM, uri_parsed.netloc, password="12345")
        http.request(uri)

    assert len(cert_log) == 1
    expect_serial = tests.x509_serial(tests.CLIENT_ENCRYPTED_PEM) if tests.x509 else 16332984194609126128
    assert int(cert_log[0]["serialNumber"], base=16) == expect_serial


@pytest.mark.skipif(
    not hasattr(tests.ssl_context(), "set_servername_callback"),
    reason="SSLContext.set_servername_callback is not available",
)
def test_sni_set_servername_callback():
    sni_log = []

    def setup_tls(context, server, skip_errors):
        context.set_servername_callback(lambda _sock, hostname, _context: sni_log.append(hostname))
        return context.wrap_socket(server, server_side=True)

    http = httplib2.Http(ca_certs=tests.CA_CERTS)
    with tests.server_const_http(tls=setup_tls) as uri:
        uri_parsed = urllib.parse.urlparse(uri)
        http.request(uri)
        assert sni_log == [uri_parsed.hostname]


def test_http_redirect_https():
    http = httplib2.Http(ca_certs=tests.CA_CERTS)
    with tests.server_const_http(tls=True) as uri_https:
        with tests.server_const_http(status=301, headers={"location": uri_https}) as uri_http:
            response, _ = http.request(uri_http, "GET")
            assert response.status == 200
            assert response["content-location"] == uri_https
            assert response.previous.status == 301
            assert response.previous["content-location"] == uri_http


def test_https_redirect_http():
    http = httplib2.Http(ca_certs=tests.CA_CERTS)
    with tests.server_const_http() as uri_http:
        with tests.server_const_http(tls=True, status=301, headers={"location": uri_http}) as uri_https:
            response, _ = http.request(uri_https, "GET")
            assert response.status == 200
            assert response["content-location"] == uri_http
            assert response.previous.status == 301
            assert response.previous["content-location"] == uri_https
