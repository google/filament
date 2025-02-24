import email.utils
import httplib2
import pytest
import re
import tests
import time

dummy_url = "http://127.0.0.1:1"


def test_get_only_if_cached_cache_hit():
    # Test that can do a GET with cache and 'only-if-cached'
    http = httplib2.Http(cache=tests.get_cache_path())
    with tests.server_const_http(add_etag=True) as uri:
        http.request(uri, "GET")
        response, content = http.request(
            uri, "GET", headers={"cache-control": "only-if-cached"}
        )
    assert response.fromcache
    assert response.status == 200


def test_get_only_if_cached_cache_miss():
    # Test that can do a GET with no cache with 'only-if-cached'
    http = httplib2.Http(cache=tests.get_cache_path())
    with tests.server_const_http(request_count=0) as uri:
        response, content = http.request(
            uri, "GET", headers={"cache-control": "only-if-cached"}
        )
    assert not response.fromcache
    assert response.status == 504


def test_get_only_if_cached_no_cache_at_all():
    # Test that can do a GET with no cache with 'only-if-cached'
    # Of course, there might be an intermediary beyond us
    # that responds to the 'only-if-cached', so this
    # test can't really be guaranteed to pass.
    http = httplib2.Http()
    with tests.server_const_http(request_count=0) as uri:
        response, content = http.request(
            uri, "GET", headers={"cache-control": "only-if-cached"}
        )
    assert not response.fromcache
    assert response.status == 504


@pytest.mark.skip(reason="was commented in legacy code")
def test_TODO_vary_no():
    pass
    # when there is no vary, a different Accept header (e.g.) should not
    # impact if the cache is used
    # test that the vary header is not sent
    # uri = urllib.parse.urljoin(base, "vary/no-vary.asis")
    # response, content = http.request(uri, 'GET', headers={'Accept': 'text/plain'})
    # assert response.status == 200
    # assert 'vary' not in response
    #
    # response, content = http.request(uri, 'GET', headers={'Accept': 'text/plain'})
    # assert response.status == 200
    # assert response.fromcache, "Should be from cache"
    #
    # response, content = http.request(uri, 'GET', headers={'Accept': 'text/html'})
    # assert response.status == 200
    # assert response.fromcache, "Should be from cache"


def test_vary_header_is_sent():
    # Verifies RFC 2616 13.6.
    # See https://www.w3.org/Protocols/rfc2616/rfc2616-sec13.html.
    http = httplib2.Http(cache=tests.get_cache_path())
    response = tests.http_response_bytes(
        headers={"vary": "Accept", "cache-control": "max-age=300"}, add_date=True
    )
    with tests.server_const_bytes(response, request_count=3) as uri:
        response, content = http.request(uri, "GET", headers={"accept": "text/plain"})
        assert response.status == 200
        assert "vary" in response

        # get the resource again, from the cache since accept header in this
        # request is the same as the request
        response, content = http.request(uri, "GET", headers={"Accept": "text/plain"})
        assert response.status == 200
        assert response.fromcache, "Should be from cache"

        # get the resource again, not from cache since Accept headers does not match
        response, content = http.request(uri, "GET", headers={"Accept": "text/html"})
        assert response.status == 200
        assert not response.fromcache, "Should not be from cache"

        # get the resource again, without any Accept header, so again no match
        response, content = http.request(uri, "GET")
        assert response.status == 200
        assert not response.fromcache, "Should not be from cache"


def test_vary_header_double():
    http = httplib2.Http(cache=tests.get_cache_path())
    response = tests.http_response_bytes(
        headers={"vary": "Accept, Accept-Language", "cache-control": "max-age=300"},
        add_date=True,
    )
    with tests.server_const_bytes(response, request_count=3) as uri:
        response, content = http.request(
            uri,
            "GET",
            headers={
                "Accept": "text/plain",
                "Accept-Language": "da, en-gb;q=0.8, en;q=0.7",
            },
        )
        assert response.status == 200
        assert "vary" in response

        # we are from cache
        response, content = http.request(
            uri,
            "GET",
            headers={
                "Accept": "text/plain",
                "Accept-Language": "da, en-gb;q=0.8, en;q=0.7",
            },
        )
        assert response.fromcache, "Should be from cache"

        response, content = http.request(uri, "GET", headers={"Accept": "text/plain"})
        assert response.status == 200
        assert not response.fromcache

        # get the resource again, not from cache, varied headers don't match exact
        response, content = http.request(uri, "GET", headers={"Accept-Language": "da"})
        assert response.status == 200
        assert not response.fromcache, "Should not be from cache"


def test_vary_unused_header():
    http = httplib2.Http(cache=tests.get_cache_path())
    response = tests.http_response_bytes(
        headers={"vary": "X-No-Such-Header", "cache-control": "max-age=300"},
        add_date=True,
    )
    with tests.server_const_bytes(response, request_count=1) as uri:
        # A header's value is not considered to vary if it's not used at all.
        response, content = http.request(uri, "GET", headers={"Accept": "text/plain"})
        assert response.status == 200
        assert "vary" in response

        # we are from cache
        response, content = http.request(uri, "GET", headers={"Accept": "text/plain"})
        assert response.fromcache, "Should be from cache"


def test_get_cache_control_no_cache():
    # Test Cache-Control: no-cache on requests
    http = httplib2.Http(cache=tests.get_cache_path())
    with tests.server_const_http(
        add_date=True,
        add_etag=True,
        headers={"cache-control": "max-age=300"},
        request_count=2,
    ) as uri:
        response, _ = http.request(uri, "GET", headers={"accept-encoding": "identity"})
        assert response.status == 200
        assert response["etag"] != ""
        assert not response.fromcache
        response, _ = http.request(uri, "GET", headers={"accept-encoding": "identity"})
        assert response.status == 200
        assert response.fromcache
        response, _ = http.request(
            uri,
            "GET",
            headers={"accept-encoding": "identity", "Cache-Control": "no-cache"},
        )
        assert response.status == 200
        assert not response.fromcache


def test_get_cache_control_pragma_no_cache():
    # Test Pragma: no-cache on requests
    http = httplib2.Http(cache=tests.get_cache_path())
    with tests.server_const_http(
        add_date=True,
        add_etag=True,
        headers={"cache-control": "max-age=300"},
        request_count=2,
    ) as uri:
        response, _ = http.request(uri, "GET", headers={"accept-encoding": "identity"})
        assert response["etag"] != ""
        response, _ = http.request(uri, "GET", headers={"accept-encoding": "identity"})
        assert response.status == 200
        assert response.fromcache
        response, _ = http.request(
            uri, "GET", headers={"accept-encoding": "identity", "Pragma": "no-cache"}
        )
        assert response.status == 200
        assert not response.fromcache


def test_get_cache_control_no_store_request():
    # A no-store request means that the response should not be stored.
    http = httplib2.Http(cache=tests.get_cache_path())
    with tests.server_const_http(
        add_date=True,
        add_etag=True,
        headers={"cache-control": "max-age=300"},
        request_count=2,
    ) as uri:
        response, _ = http.request(uri, "GET", headers={"Cache-Control": "no-store"})
        assert response.status == 200
        assert not response.fromcache
        response, _ = http.request(uri, "GET", headers={"Cache-Control": "no-store"})
        assert response.status == 200
        assert not response.fromcache


def test_get_cache_control_no_store_response():
    # A no-store response means that the response should not be stored.
    http = httplib2.Http(cache=tests.get_cache_path())
    with tests.server_const_http(
        add_date=True,
        add_etag=True,
        headers={"cache-control": "max-age=300, no-store"},
        request_count=2,
    ) as uri:
        response, _ = http.request(uri, "GET")
        assert response.status == 200
        assert not response.fromcache
        response, _ = http.request(uri, "GET")
        assert response.status == 200
        assert not response.fromcache


def test_get_cache_control_no_cache_no_store_request():
    # Test that a no-store, no-cache clears the entry from the cache
    # even if it was cached previously.
    http = httplib2.Http(cache=tests.get_cache_path())
    with tests.server_const_http(
        add_date=True,
        add_etag=True,
        headers={"cache-control": "max-age=300"},
        request_count=3,
    ) as uri:
        response, _ = http.request(uri, "GET")
        response, _ = http.request(uri, "GET")
        assert response.fromcache
        response, _ = http.request(
            uri, "GET", headers={"Cache-Control": "no-store, no-cache"}
        )
        assert response.status == 200
        assert not response.fromcache
        response, _ = http.request(
            uri, "GET", headers={"Cache-Control": "no-store, no-cache"}
        )
        assert response.status == 200
        assert not response.fromcache


def test_update_invalidates_cache():
    # Test that calling PUT or DELETE on a
    # URI that is cache invalidates that cache.
    http = httplib2.Http(cache=tests.get_cache_path())

    def handler(request):
        if request.method in ("PUT", "PATCH", "DELETE"):
            return tests.http_response_bytes(status=405)
        return tests.http_response_bytes(
            add_date=True, add_etag=True, headers={"cache-control": "max-age=300"}
        )

    with tests.server_request(handler, request_count=3) as uri:
        response, _ = http.request(uri, "GET")
        response, _ = http.request(uri, "GET")
        assert response.fromcache
        response, _ = http.request(uri, "DELETE")
        assert response.status == 405
        assert not response.fromcache
        response, _ = http.request(uri, "GET")
        assert not response.fromcache


def handler_conditional_update(request):
    respond = tests.http_response_bytes
    if request.method == "GET":
        if request.headers.get("if-none-match", "") == "12345":
            return respond(status=304)
        return respond(
            add_date=True, headers={"etag": "12345", "cache-control": "max-age=300"}
        )
    elif request.method in ("PUT", "PATCH", "DELETE"):
        if request.headers.get("if-match", "") == "12345":
            return respond(status=200)
        return respond(status=412)
    return respond(status=405)


@pytest.mark.parametrize("method", ("PUT", "PATCH"))
def test_update_uses_cached_etag(method):
    # Test that we natively support http://www.w3.org/1999/04/Editing/
    http = httplib2.Http(cache=tests.get_cache_path())
    with tests.server_request(handler_conditional_update, request_count=3) as uri:
        response, _ = http.request(uri, "GET")
        assert response.status == 200
        assert not response.fromcache
        response, _ = http.request(uri, "GET")
        assert response.status == 200
        assert response.fromcache
        response, _ = http.request(uri, method, body=b"foo")
        assert response.status == 200
        response, _ = http.request(uri, method, body=b"foo")
        assert response.status == 412


def test_update_uses_cached_etag_and_oc_method():
    # Test that we natively support http://www.w3.org/1999/04/Editing/
    http = httplib2.Http(cache=tests.get_cache_path())
    with tests.server_request(handler_conditional_update, request_count=2) as uri:
        response, _ = http.request(uri, "GET")
        assert response.status == 200
        assert not response.fromcache
        response, _ = http.request(uri, "GET")
        assert response.status == 200
        assert response.fromcache
        http.optimistic_concurrency_methods.append("DELETE")
        response, _ = http.request(uri, "DELETE")
        assert response.status == 200


def test_update_uses_cached_etag_overridden():
    # Test that we natively support http://www.w3.org/1999/04/Editing/
    http = httplib2.Http(cache=tests.get_cache_path())
    with tests.server_request(handler_conditional_update, request_count=2) as uri:
        response, content = http.request(uri, "GET")
        assert response.status == 200
        assert not response.fromcache
        response, content = http.request(uri, "GET")
        assert response.status == 200
        assert response.fromcache
        response, content = http.request(
            uri, "PUT", body=b"foo", headers={"if-match": "fred"}
        )
        assert response.status == 412


@pytest.mark.parametrize(
    "data",
    (
        ({}, {}),
        ({"cache-control": " no-cache"}, {"no-cache": 1}),
        (
            {"cache-control": " no-store, max-age = 7200"},
            {"no-store": 1, "max-age": "7200"},
        ),
        ({"cache-control": " , "}, {"": 1}),  # FIXME
        (
            {"cache-control": "Max-age=3600;post-check=1800,pre-check=3600"},
            {"max-age": "3600;post-check=1800", "pre-check": "3600"},
        ),
    ),
    ids=lambda data: str(data[0]),
)
def test_parse_cache_control(data):
    header, expected = data
    assert httplib2._parse_cache_control(header) == expected


def test_normalize_headers():
    # Test that we normalize headers to lowercase
    h = httplib2._normalize_headers({"Cache-Control": "no-cache", "Other": "Stuff"})
    assert "cache-control" in h
    assert "other" in h
    assert h["other"] == "Stuff"


@pytest.mark.parametrize(
    "data",
    (
        (
            {"cache-control": "no-cache"},
            {"cache-control": "max-age=7200"},
            "TRANSPARENT",
        ),
        ({}, {"cache-control": "max-age=fred, min-fresh=barney"}, "STALE"),
        ({}, {"date": "{now}", "expires": "{now+3}"}, "FRESH"),
        (
            {},
            {"date": "{now}", "expires": "{now+3}", "cache-control": "no-cache"},
            "STALE",
        ),
        ({"cache-control": "must-revalidate"}, {}, "STALE"),
        ({}, {"cache-control": "must-revalidate"}, "STALE"),
        ({}, {"date": "{now}", "cache-control": "max-age=0"}, "STALE"),
        ({"cache-control": "only-if-cached"}, {}, "FRESH"),
        ({}, {"date": "{now}", "expires": "0"}, "STALE"),
        ({}, {"data": "{now+3}"}, "STALE"),
        (
            {"cache-control": "max-age=0"},
            {"date": "{now}", "cache-control": "max-age=2"},
            "STALE",
        ),
        (
            {"cache-control": "min-fresh=2"},
            {"date": "{now}", "expires": "{now+2}"},
            "STALE",
        ),
        (
            {"cache-control": "min-fresh=2"},
            {"date": "{now}", "expires": "{now+4}"},
            "FRESH",
        ),
    ),
    ids=lambda data: str(data),
)
def test_entry_disposition(data):
    now = time.time()
    nowre = re.compile(r"{now([\+\-]\d+)?}")

    def render(s):
        m = nowre.match(s)
        if m:
            offset = int(m.expand(r"\1")) if m.group(1) else 0
            s = email.utils.formatdate(now + offset, usegmt=True)
        return s

    request, response, expected = data
    request = {k: render(v) for k, v in request.items()}
    response = {k: render(v) for k, v in response.items()}
    assert httplib2._entry_disposition(response, request) == expected


def test_expiration_model_fresh():
    response_headers = {
        "date": email.utils.formatdate(usegmt=True),
        "cache-control": "max-age=2",
    }
    assert httplib2._entry_disposition(response_headers, {}) == "FRESH"
    # TODO: add current time as _entry_disposition argument to avoid sleep in tests
    time.sleep(3)
    assert httplib2._entry_disposition(response_headers, {}) == "STALE"


def test_expiration_model_date_and_expires():
    now = time.time()
    response_headers = {
        "date": email.utils.formatdate(now, usegmt=True),
        "expires": email.utils.formatdate(now + 2, usegmt=True),
    }
    assert httplib2._entry_disposition(response_headers, {}) == "FRESH"
    time.sleep(3)
    assert httplib2._entry_disposition(response_headers, {}) == "STALE"


# TODO: Repeat all cache tests with memcache. pytest.mark.parametrize
# cache = memcache.Client(['127.0.0.1:11211'], debug=0)
# #cache = memcache.Client(['10.0.0.4:11211'], debug=1)
# http = httplib2.Http(cache)
