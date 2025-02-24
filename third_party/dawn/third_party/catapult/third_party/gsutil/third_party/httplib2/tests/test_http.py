from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import email.utils
import errno
import httplib2

try:
    from unittest import mock
except ImportError:
    import mock
import os
import sys
import pytest
from six.moves import http_client, urllib
import socket
import tests


def _raise_connection_refused_exception(*args, **kwargs):
    raise socket.error(errno.ECONNREFUSED, "Connection refused.")


def test_connection_type():
    http = httplib2.Http()
    http.force_exception_to_status_code = False
    response, content = http.request(tests.DUMMY_URL, connection_type=tests.MockHTTPConnection)
    assert response["content-location"] == tests.DUMMY_URL
    assert content == b"the body"


def test_bad_status_line_retry():
    http = httplib2.Http()
    old_retries = httplib2.RETRIES
    httplib2.RETRIES = 1
    http.force_exception_to_status_code = False
    try:
        response, content = http.request(tests.DUMMY_URL, connection_type=tests.MockHTTPBadStatusConnection)
    except http_client.BadStatusLine:
        assert tests.MockHTTPBadStatusConnection.num_calls == 2
    httplib2.RETRIES = old_retries


def test_unknown_server():
    http = httplib2.Http()
    http.force_exception_to_status_code = False
    with tests.assert_raises(httplib2.ServerNotFoundError):
        with mock.patch("socket.socket.connect", side_effect=socket.gaierror):
            http.request("http://no-such-hostname./")

    # Now test with exceptions turned off
    http.force_exception_to_status_code = True
    response, content = http.request("http://no-such-hostname./")
    assert response["content-type"] == "text/plain"
    assert content.startswith(b"Unable to find")
    assert response.status == 400


# @pytest.mark.skipif(
#     not os.environ.get("httplib2_test_still_run_skipped") and sys.version_info.major == 2,
#     reason="Fails on Travis py27/pypy, works elsewhere. See https://travis-ci.org/httplib2/httplib2/jobs/408769880.",
# )
@mock.patch("socket.socket.connect", spec=True)
def test_connection_refused_raises_exception(mock_socket_connect):
    mock_socket_connect.side_effect = _raise_connection_refused_exception
    http = httplib2.Http()
    http.force_exception_to_status_code = False
    with tests.assert_raises(socket.error):
        http.request(tests.DUMMY_URL)


# @pytest.mark.skipif(
#     not os.environ.get("httplib2_test_still_run_skipped") and sys.version_info.major == 2,
#     reason="Fails on Travis py27/pypy, works elsewhere. See https://travis-ci.org/httplib2/httplib2/jobs/408769880.",
# )
@mock.patch("socket.socket.connect", spec=True)
def test_connection_refused_returns_response(mock_socket_connect):
    mock_socket_connect.side_effect = _raise_connection_refused_exception
    http = httplib2.Http()
    http.force_exception_to_status_code = True
    response, content = http.request(tests.DUMMY_URL)
    content = content.lower()
    assert response["content-type"] == "text/plain"
    assert b"connection refused" in content or b"actively refused" in content or b"socket is not connected" in content
    assert response.status == 400


def test_get_iri():
    http = httplib2.Http()
    query = u"?a=\N{CYRILLIC CAPITAL LETTER DJE}"
    with tests.server_reflect() as uri:
        response, content = http.request(uri + query, "GET")
        assert response.status == 200
        reflected = tests.HttpRequest.from_bytes(content)
        assert reflected.uri == "/?a=%D0%82"


def test_get_is_default_method():
    # Test that GET is the default method
    http = httplib2.Http()
    with tests.server_reflect() as uri:
        response, content = http.request(uri)
        assert response.status == 200
        reflected = tests.HttpRequest.from_bytes(content)
        assert reflected.method == "GET"


def test_different_methods():
    # Test that all methods can be used
    http = httplib2.Http()
    methods = ["GET", "PUT", "DELETE", "POST", "unknown"]
    with tests.server_reflect(request_count=len(methods)) as uri:
        for method in methods:
            response, content = http.request(uri, method, body=b" ")
            assert response.status == 200
            reflected = tests.HttpRequest.from_bytes(content)
            assert reflected.method == method


def test_head_read():
    # Test that we don't try to read the response of a HEAD request
    # since httplib blocks response.read() for HEAD requests.
    http = httplib2.Http()
    respond_with = b"HTTP/1.0 200 OK\r\ncontent-length: " b"14\r\n\r\nnon-empty-body"
    with tests.server_const_bytes(respond_with) as uri:
        response, content = http.request(uri, "HEAD")
    assert response.status == 200
    assert content == b""


def test_get_no_cache():
    # Test that can do a GET w/o the cache turned on.
    http = httplib2.Http()
    with tests.server_const_http() as uri:
        response, content = http.request(uri, "GET")
    assert response.status == 200
    assert response.previous is None


def test_user_agent():
    # Test that we provide a default user-agent
    http = httplib2.Http()
    with tests.server_reflect() as uri:
        response, content = http.request(uri, "GET")
        assert response.status == 200
        reflected = tests.HttpRequest.from_bytes(content)
        assert reflected.headers.get("user-agent", "").startswith("Python-httplib2/")


def test_user_agent_non_default():
    # Test that the default user-agent can be over-ridden
    http = httplib2.Http()
    with tests.server_reflect() as uri:
        response, content = http.request(uri, "GET", headers={"User-Agent": "fred/1.0"})
        assert response.status == 200
        reflected = tests.HttpRequest.from_bytes(content)
        assert reflected.headers.get("user-agent") == "fred/1.0"


def test_get_300_with_location():
    # Test the we automatically follow 300 redirects if a Location: header is provided
    http = httplib2.Http()
    final_content = b"This is the final destination.\n"
    routes = {
        "/final": tests.http_response_bytes(body=final_content),
        "": tests.http_response_bytes(status="300 Multiple Choices", headers={"location": "/final"}),
    }
    with tests.server_route(routes, request_count=2) as uri:
        response, content = http.request(uri, "GET")
    assert response.status == 200
    assert content == final_content
    assert response.previous.status == 300
    assert not response.previous.fromcache

    # Confirm that the intermediate 300 is not cached
    with tests.server_route(routes, request_count=2) as uri:
        response, content = http.request(uri, "GET")
    assert response.status == 200
    assert content == final_content
    assert response.previous.status == 300
    assert not response.previous.fromcache


def test_get_300_with_location_noredirect():
    # Test the we automatically follow 300 redirects if a Location: header is provided
    http = httplib2.Http()
    http.follow_redirects = False
    response = tests.http_response_bytes(
        status="300 Multiple Choices",
        headers={"location": "/final"},
        body=b"redirect body",
    )
    with tests.server_const_bytes(response) as uri:
        response, content = http.request(uri, "GET")
    assert response.status == 300


def test_get_300_without_location():
    # Not giving a Location: header in a 300 response is acceptable
    # In which case we just return the 300 response
    http = httplib2.Http()
    with tests.server_const_http(status="300 Multiple Choices", body=b"redirect body") as uri:
        response, content = http.request(uri, "GET")
    assert response.status == 300
    assert response.previous is None
    assert content == b"redirect body"


def test_get_301():
    # Test that we automatically follow 301 redirects
    # and that we cache the 301 response
    http = httplib2.Http(cache=tests.get_cache_path())
    destination = ""
    routes = {
        "/final": tests.http_response_bytes(body=b"This is the final destination.\n"),
        "": tests.http_response_bytes(
            status="301 Now where did I leave that URL",
            headers={"location": "/final"},
            body=b"redirect body",
        ),
    }
    with tests.server_route(routes, request_count=3) as uri:
        destination = urllib.parse.urljoin(uri, "/final")
        response1, content1 = http.request(uri, "GET")
        response2, content2 = http.request(uri, "GET")
    assert response1.status == 200
    assert "content-location" in response2
    assert response1["content-location"] == destination
    assert content1 == b"This is the final destination.\n"
    assert response1.previous.status == 301
    assert not response1.previous.fromcache

    assert response2.status == 200
    assert response2["content-location"] == destination
    assert content2 == b"This is the final destination.\n"
    assert response2.previous.status == 301
    assert response2.previous.fromcache


@pytest.mark.skipif(
    not os.environ.get("httplib2_test_still_run_skipped") and sys.version_info.major == 2,
    reason="FIXME: timeout on CI py27 and pypy, works elsewhere",
)
def test_head_301():
    # Test that we automatically follow 301 redirects
    http = httplib2.Http()
    destination = ""
    routes = {
        "/final": tests.http_response_bytes(body=b"This is the final destination.\n"),
        "": tests.http_response_bytes(
            status="301 Now where did I leave that URL",
            headers={"location": "/final"},
            body=b"redirect body",
        ),
    }
    with tests.server_route(routes, request_count=2) as uri:
        destination = urllib.parse.urljoin(uri, "/final")
        response, content = http.request(uri, "HEAD")
    assert response.status == 200
    assert response["content-location"] == destination
    assert response.previous.status == 301
    assert not response.previous.fromcache


@pytest.mark.xfail(reason=("FIXME: 301 cache works only with follow_redirects, should work regardless"))
def test_get_301_no_redirect():
    # Test that we cache the 301 response
    http = httplib2.Http(cache=tests.get_cache_path(), timeout=0.5)
    http.follow_redirects = False
    response = tests.http_response_bytes(
        status="301 Now where did I leave that URL",
        headers={"location": "/final", "cache-control": "max-age=300"},
        body=b"redirect body",
        add_date=True,
    )
    with tests.server_const_bytes(response) as uri:
        response, _ = http.request(uri, "GET")
        assert response.status == 301
        assert not response.fromcache
        response, _ = http.request(uri, "GET")
        assert response.status == 301
        assert response.fromcache


def test_get_302():
    # Test that we automatically follow 302 redirects
    # and that we DO NOT cache the 302 response
    http = httplib2.Http(cache=tests.get_cache_path())
    second_url, final_url = "", ""
    routes = {
        "/final": tests.http_response_bytes(body=b"This is the final destination.\n"),
        "/second": tests.http_response_bytes(
            status="302 Found", headers={"location": "/final"}, body=b"second redirect"
        ),
        "": tests.http_response_bytes(status="302 Found", headers={"location": "/second"}, body=b"redirect body"),
    }
    with tests.server_route(routes, request_count=7) as uri:
        second_url = urllib.parse.urljoin(uri, "/second")
        final_url = urllib.parse.urljoin(uri, "/final")
        response1, content1 = http.request(second_url, "GET")
        response2, content2 = http.request(second_url, "GET")
        response3, content3 = http.request(uri, "GET")
    assert response1.status == 200
    assert response1["content-location"] == final_url
    assert content1 == b"This is the final destination.\n"
    assert response1.previous.status == 302
    assert not response1.previous.fromcache

    assert response2.status == 200
    # FIXME:
    # assert response2.fromcache
    assert response2["content-location"] == final_url
    assert content2 == b"This is the final destination.\n"
    assert response2.previous.status == 302
    assert not response2.previous.fromcache
    assert response2.previous["content-location"] == second_url

    assert response3.status == 200
    # FIXME:
    # assert response3.fromcache
    assert content3 == b"This is the final destination.\n"
    assert response3.previous.status == 302
    assert not response3.previous.fromcache


def test_get_302_redirection_limit():
    # Test that we can set a lower redirection limit
    # and that we raise an exception when we exceed
    # that limit.
    http = httplib2.Http()
    http.force_exception_to_status_code = False
    routes = {
        "/second": tests.http_response_bytes(
            status="302 Found", headers={"location": "/final"}, body=b"second redirect"
        ),
        "": tests.http_response_bytes(status="302 Found", headers={"location": "/second"}, body=b"redirect body"),
    }
    with tests.server_route(routes, request_count=4) as uri:
        try:
            http.request(uri, "GET", redirections=1)
            assert False, "This should not happen"
        except httplib2.RedirectLimit:
            pass
        except Exception:
            assert False, "Threw wrong kind of exception "

        # Re-run the test with out the exceptions
        http.force_exception_to_status_code = True
        response, content = http.request(uri, "GET", redirections=1)

    assert response.status == 500
    assert response.reason.startswith("Redirected more")
    assert response["status"] == "302"
    assert content == b"second redirect"
    assert response.previous is not None


def test_get_302_no_location():
    # Test that we throw an exception when we get
    # a 302 with no Location: header.
    http = httplib2.Http()
    http.force_exception_to_status_code = False
    with tests.server_const_http(status="302 Found", request_count=2) as uri:
        try:
            http.request(uri, "GET")
            assert False, "Should never reach here"
        except httplib2.RedirectMissingLocation:
            pass
        except Exception:
            assert False, "Threw wrong kind of exception "

        # Re-run the test with out the exceptions
        http.force_exception_to_status_code = True
        response, content = http.request(uri, "GET")

    assert response.status == 500
    assert response.reason.startswith("Redirected but")
    assert "302" == response["status"]
    assert content == b""


@pytest.mark.skipif(
    not os.environ.get("httplib2_test_still_run_skipped") and sys.version_info.major == 2,
    reason="FIXME: timeout on CI py27 and pypy, works elsewhere",
)
def test_303():
    # Do a follow-up GET on a Location: header
    # returned from a POST that gave a 303.
    http = httplib2.Http()
    routes = {
        "/final": tests.make_http_reflect(),
        "": tests.make_http_reflect(status="303 See Other", headers={"location": "/final"}),
    }
    with tests.server_route(routes, request_count=2) as uri:
        response, content = http.request(uri, "POST", " ")
    assert response.status == 200
    reflected = tests.HttpRequest.from_bytes(content)
    assert reflected.uri == "/final"
    assert response.previous.status == 303

    # Skip follow-up GET
    http = httplib2.Http()
    http.follow_redirects = False
    with tests.server_route(routes, request_count=1) as uri:
        response, content = http.request(uri, "POST", " ")
    assert response.status == 303

    # All methods can be used
    http = httplib2.Http()
    cases = "DELETE GET HEAD POST PUT EVEN_NEW_ONES".split(" ")
    with tests.server_route(routes, request_count=len(cases) * 2) as uri:
        for method in cases:
            response, content = http.request(uri, method, body=b"q q")
            assert response.status == 200
            reflected = tests.HttpRequest.from_bytes(content)
            assert reflected.method == "GET"


def test_etag_used():
    # Test that we use ETags properly to validate our cache
    cache_path = tests.get_cache_path()
    http = httplib2.Http(cache=cache_path)
    response_kwargs = dict(
        add_date=True,
        add_etag=True,
        body=b"something",
        headers={"cache-control": "public,max-age=300"},
    )

    def handler(request):
        if request.headers.get("range"):
            return tests.http_response_bytes(status=206, **response_kwargs)
        return tests.http_response_bytes(**response_kwargs)

    with tests.server_request(handler, request_count=2) as uri:
        response, _ = http.request(uri, "GET", headers={"accept-encoding": "identity"})
        assert response["etag"] == '"437b930db84b8079c2dd804a71936b5f"'

        http.request(uri, "GET", headers={"accept-encoding": "identity"})
        response, _ = http.request(
            uri,
            "GET",
            headers={"accept-encoding": "identity", "cache-control": "must-revalidate"},
        )
        assert response.status == 200
        assert response.fromcache

        # TODO: API to read cache item, at least internal to tests
        cache_file_name = os.path.join(cache_path, httplib2.safename(httplib2.urlnorm(uri)[-1]))
        with open(cache_file_name, "r") as f:
            status_line = f.readline()
        assert status_line.startswith("status:")

        response, content = http.request(uri, "HEAD", headers={"accept-encoding": "identity"})
        assert response.status == 200
        assert response.fromcache

        response, content = http.request(uri, "GET", headers={"accept-encoding": "identity", "range": "bytes=0-0"})
        assert response.status == 206
        assert not response.fromcache


def test_etag_ignore():
    # Test that we can forcibly ignore ETags
    http = httplib2.Http(cache=tests.get_cache_path())
    response_kwargs = dict(add_date=True, add_etag=True)
    with tests.server_reflect(request_count=3, **response_kwargs) as uri:
        response, content = http.request(uri, "GET", headers={"accept-encoding": "identity"})
        assert response.status == 200
        assert response["etag"] != ""

        response, content = http.request(
            uri,
            "GET",
            headers={"accept-encoding": "identity", "cache-control": "max-age=0"},
        )
        reflected = tests.HttpRequest.from_bytes(content)
        assert reflected.headers.get("if-none-match")

        http.ignore_etag = True
        response, content = http.request(
            uri,
            "GET",
            headers={"accept-encoding": "identity", "cache-control": "max-age=0"},
        )
        assert not response.fromcache
        reflected = tests.HttpRequest.from_bytes(content)
        assert not reflected.headers.get("if-none-match")


def test_etag_override():
    # Test that we can forcibly ignore ETags
    http = httplib2.Http(cache=tests.get_cache_path())
    response_kwargs = dict(add_date=True, add_etag=True)
    with tests.server_reflect(request_count=3, **response_kwargs) as uri:
        response, _ = http.request(uri, "GET", headers={"accept-encoding": "identity"})
        assert response.status == 200
        assert response["etag"] != ""

        response, content = http.request(
            uri,
            "GET",
            headers={"accept-encoding": "identity", "cache-control": "max-age=0"},
        )
        assert response.status == 200
        reflected = tests.HttpRequest.from_bytes(content)
        assert reflected.headers.get("if-none-match")
        assert reflected.headers.get("if-none-match") != "fred"

        response, content = http.request(
            uri,
            "GET",
            headers={
                "accept-encoding": "identity",
                "cache-control": "max-age=0",
                "if-none-match": "fred",
            },
        )
        assert response.status == 200
        reflected = tests.HttpRequest.from_bytes(content)
        assert reflected.headers.get("if-none-match") == "fred"


@pytest.mark.skip(reason="was commented in legacy code")
def test_get_304_end_to_end():
    pass
    # Test that end to end headers get overwritten in the cache
    # uri = urllib.parse.urljoin(base, "304/end2end.cgi")
    # response, content = http.request(uri, 'GET')
    # assertNotEqual(response['etag'], "")
    # old_date = response['date']
    # time.sleep(2)

    # response, content = http.request(uri, 'GET', headers = {'Cache-Control': 'max-age=0'})
    # # The response should be from the cache, but the Date: header should be updated.
    # new_date = response['date']
    # assert new_date != old_date
    # assert response.status == 200
    # assert response.fromcache == True


def test_get_304_last_modified():
    # Test that we can still handle a 304
    # by only using the last-modified cache validator.
    http = httplib2.Http(cache=tests.get_cache_path())
    date = email.utils.formatdate()

    def handler(read):
        read()
        yield tests.http_response_bytes(status=200, body=b"something", headers={"date": date, "last-modified": date})

        request2 = read()
        assert request2.headers["if-modified-since"] == date
        yield tests.http_response_bytes(status=304)

    with tests.server_yield(handler, request_count=2) as uri:
        response, content = http.request(uri, "GET")
        assert response.get("last-modified") == date

        response, content = http.request(uri, "GET")
        assert response.status == 200
        assert response.fromcache


def test_get_307():
    # Test that we do follow 307 redirects but
    # do not cache the 307
    http = httplib2.Http(cache=tests.get_cache_path(), timeout=1)
    r307 = tests.http_response_bytes(status=307, headers={"location": "/final"})
    r200 = tests.http_response_bytes(
        status=200,
        add_date=True,
        body=b"final content\n",
        headers={"cache-control": "max-age=300"},
    )

    with tests.server_list_http([r307, r200, r307]) as uri:
        response, content = http.request(uri, "GET")
        assert response.previous.status == 307
        assert not response.previous.fromcache
        assert response.status == 200
        assert not response.fromcache
        assert content == b"final content\n"

        response, content = http.request(uri, "GET")
        assert response.previous.status == 307
        assert not response.previous.fromcache
        assert response.status == 200
        assert response.fromcache
        assert content == b"final content\n"


def test_post_307():
    # 307: follow with same method
    http = httplib2.Http(cache=tests.get_cache_path(), timeout=1)
    http.follow_all_redirects = True
    r307 = tests.http_response_bytes(status=307, headers={"location": "/final"})
    r200 = tests.http_response_bytes(status=200, body=b"final content\n")

    with tests.server_list_http([r307, r200, r307, r200]) as uri:
        response, content = http.request(uri, "POST")
        assert response.previous.status == 307
        assert not response.previous.fromcache
        assert response.status == 200
        assert not response.fromcache
        assert content == b"final content\n"

        response, content = http.request(uri, "POST")
        assert response.previous.status == 307
        assert not response.previous.fromcache
        assert response.status == 200
        assert not response.fromcache
        assert content == b"final content\n"


def test_change_308():
    # 308: follow with same method, cache redirect
    http = httplib2.Http(cache=tests.get_cache_path(), timeout=1)
    routes = {
        "/final": tests.make_http_reflect(),
        "": tests.http_response_bytes(
            status="308 Permanent Redirect",
            add_date=True,
            headers={"cache-control": "max-age=300", "location": "/final"},
        ),
    }

    with tests.server_route(routes, request_count=3) as uri:
        response, content = http.request(uri, "CHANGE", body=b"hello308")
        assert response.previous.status == 308
        assert not response.previous.fromcache
        assert response.status == 200
        assert not response.fromcache
        assert content.startswith(b"CHANGE /final HTTP")

        response, content = http.request(uri, "CHANGE")
        assert response.previous.status == 308
        assert response.previous.fromcache
        assert response.status == 200
        assert not response.fromcache
        assert content.startswith(b"CHANGE /final HTTP")


def test_get_410():
    # Test that we pass 410's through
    http = httplib2.Http()
    with tests.server_const_http(status=410) as uri:
        response, content = http.request(uri, "GET")
        assert response.status == 410


def test_get_duplicate_headers():
    # Test that duplicate headers get concatenated via ','
    http = httplib2.Http()
    response = b"""HTTP/1.0 200 OK\r\n\
Link: link1\r\n\
Content-Length: 7\r\n\
Link: link2\r\n\r\n\
content"""
    with tests.server_const_bytes(response) as uri:
        response, content = http.request(uri, "GET")
        assert response.status == 200
        assert content == b"content"
        assert response["link"], "link1, link2"


def test_custom_redirect_codes():
    http = httplib2.Http()
    http.redirect_codes = set([300])
    with tests.server_const_http(status=301, request_count=1) as uri:
        response, content = http.request(uri, "GET")
        assert response.status == 301
        assert response.previous is None


def test_cwe93_inject_crlf():
    # https://cwe.mitre.org/data/definitions/93.html
    # GET /?q= HTTP/1.1      <- injected "HTTP/1.1" from attacker
    # injected: attack
    # ignore-http: HTTP/1.1  <- nominal "HTTP/1.1" from library
    # Host: localhost:57285
    http = httplib2.Http()
    with tests.server_reflect() as uri:
        danger_url = urllib.parse.urljoin(uri, "?q= HTTP/1.1\r\ninjected: attack\r\nignore-http:")
        response, content = http.request(danger_url, "GET")
        assert response.status == 200
        req = tests.HttpRequest.from_bytes(content)
        assert req.headers.get("injected") is None


def test_inject_space():
    # Injecting space into request line is precursor to CWE-93 and possibly other injections
    http = httplib2.Http()
    with tests.server_reflect() as uri:
        # "\r\nignore-http:" suffix is nuance for current server implementation
        # please only pay attention to space after "?q="
        danger_url = urllib.parse.urljoin(uri, "?q= HTTP/1.1\r\nignore-http:")
        response, content = http.request(danger_url, "GET")
        assert response.status == 200
        req = tests.HttpRequest.from_bytes(content)
        expect = (
            # new behavior after bpo-43882 fix
            # https://github.com/httplib2/httplib2/issues/193
            "/?q=%20HTTP/1.1ignore-http:",
            # old behavior
            "/?q=%20HTTP/1.1%0D%0Aignore-http:",
        )
        assert req.uri in expect
