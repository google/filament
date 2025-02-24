import httplib2
import tests


def test_gzip_head():
    # Test that we don't try to decompress a HEAD response
    http = httplib2.Http()
    response = tests.http_response_bytes(
        headers={"content-encoding": "gzip", "content-length": 42}
    )
    with tests.server_const_bytes(response) as uri:
        response, content = http.request(uri, "HEAD")
        assert response.status == 200
        assert int(response["content-length"]) != 0
        assert content == b""


def test_gzip_get():
    # Test that we support gzip compression
    http = httplib2.Http()
    response = tests.http_response_bytes(
        headers={"content-encoding": "gzip"},
        body=tests.gzip_compress(b"properly compressed"),
    )
    with tests.server_const_bytes(response) as uri:
        response, content = http.request(uri, "GET")
        assert response.status == 200
        assert "content-encoding" not in response
        assert "-content-encoding" in response
        assert int(response["content-length"]) == len(b"properly compressed")
        assert content == b"properly compressed"


def test_gzip_post_response():
    http = httplib2.Http()
    response = tests.http_response_bytes(
        headers={"content-encoding": "gzip"},
        body=tests.gzip_compress(b"properly compressed"),
    )
    with tests.server_const_bytes(response) as uri:
        response, content = http.request(uri, "POST", body=b"")
        assert response.status == 200
        assert "content-encoding" not in response
        assert "-content-encoding" in response


def test_gzip_malformed_response():
    http = httplib2.Http()
    # Test that we raise a good exception when the gzip fails
    http.force_exception_to_status_code = False
    response = tests.http_response_bytes(
        headers={"content-encoding": "gzip"}, body=b"obviously not compressed"
    )
    with tests.server_const_bytes(response, request_count=2) as uri:
        with tests.assert_raises(httplib2.FailedToDecompressContent):
            http.request(uri, "GET")

        # Re-run the test with out the exceptions
        http.force_exception_to_status_code = True

        response, content = http.request(uri, "GET")
        assert response.status == 500
        assert response.reason.startswith("Content purported")


def test_deflate_get():
    # Test that we support deflate compression
    http = httplib2.Http()
    response = tests.http_response_bytes(
        headers={"content-encoding": "deflate"},
        body=tests.deflate_compress(b"properly compressed"),
    )
    with tests.server_const_bytes(response) as uri:
        response, content = http.request(uri, "GET")
        assert response.status == 200
        assert "content-encoding" not in response
        assert int(response["content-length"]) == len(b"properly compressed")
        assert content == b"properly compressed"


def test_deflate_malformed_response():
    # Test that we raise a good exception when the deflate fails
    http = httplib2.Http()
    http.force_exception_to_status_code = False
    response = tests.http_response_bytes(
        headers={"content-encoding": "deflate"}, body=b"obviously not compressed"
    )
    with tests.server_const_bytes(response, request_count=2) as uri:
        with tests.assert_raises(httplib2.FailedToDecompressContent):
            http.request(uri, "GET")

        # Re-run the test with out the exceptions
        http.force_exception_to_status_code = True

        response, content = http.request(uri, "GET")
        assert response.status == 500
        assert response.reason.startswith("Content purported")
