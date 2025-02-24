import httplib2
import pytest


def test_from_std66():
    cases = (
        ("http://example.com", ("http", "example.com", "", None, None)),
        ("https://example.com", ("https", "example.com", "", None, None)),
        ("https://example.com:8080", ("https", "example.com:8080", "", None, None)),
        ("http://example.com/", ("http", "example.com", "/", None, None)),
        ("http://example.com/path", ("http", "example.com", "/path", None, None)),
        (
            "http://example.com/path?a=1&b=2",
            ("http", "example.com", "/path", "a=1&b=2", None),
        ),
        (
            "http://example.com/path?a=1&b=2#fred",
            ("http", "example.com", "/path", "a=1&b=2", "fred"),
        ),
        (
            "http://example.com/path?a=1&b=2#fred",
            ("http", "example.com", "/path", "a=1&b=2", "fred"),
        ),
    )
    for a, b in cases:
        assert httplib2.parse_uri(a) == b


def test_norm():
    cases = (
        ("http://example.org", "http://example.org/"),
        ("http://EXAMple.org", "http://example.org/"),
        ("http://EXAMple.org?=b", "http://example.org/?=b"),
        ("http://EXAMple.org/mypath?a=b", "http://example.org/mypath?a=b"),
        ("http://localhost:80", "http://localhost:80/"),
    )
    for a, b in cases:
        assert httplib2.urlnorm(a)[-1] == b

    assert httplib2.urlnorm("http://localhost:80/") == httplib2.urlnorm(
        "HTTP://LOCALHOST:80"
    )

    try:
        httplib2.urlnorm("/")
        assert False, "Non-absolute URIs should raise an exception"
    except httplib2.RelativeURIError:
        pass


@pytest.mark.parametrize(
    "data",
    (
        ("", ",d41d8cd98f00b204e9800998ecf8427e"),
        (
            "http://example.org/fred/?a=b",
            "example.orgfreda=b,58489f63a7a83c3b7794a6a398ee8b1f",
        ),
        (
            "http://example.org/fred?/a=b",
            "example.orgfreda=b,8c5946d56fec453071f43329ff0be46b",
        ),
        (
            "http://www.example.org/fred?/a=b",
            "www.example.orgfreda=b,499c44b8d844a011b67ea2c015116968",
        ),
        (
            "https://www.example.org/fred?/a=b",
            "www.example.orgfreda=b,692e843a333484ce0095b070497ab45d",
        ),
        (
            httplib2.urlnorm("http://WWW")[-1],
            httplib2.safename(httplib2.urlnorm("http://www")[-1]),
        ),
        (
            u"http://\u2304.org/fred/?a=b",
            ".orgfreda=b,ecaf0f97756c0716de76f593bd60a35e",
        ),
        (
            "normal-resource-name.js",
            "normal-resource-name.js,8ff7c46fd6e61bf4e91a0a1606954a54",
        ),
        (
            "foo://dom/path/brath/carapath",
            "dompathbrathcarapath,83db942781ed975c7a5b7c24039f8ca3",
        ),
        ("with/slash", "withslash,17cc656656bb8ce2411bd41ead56d176"),
        (
            "thisistoomuch" * 42,
            ("thisistoomuch" * 6) + "thisistoomuc,c4553439dd179422c6acf6a8ac093eb6",
        ),
        (u"\u043f\u0440", ",9f18c0db74a9734e9d18461e16345083"),
        (u"\u043f\u0440".encode("utf-8"), ",9f18c0db74a9734e9d18461e16345083"),
        (
            b"column\tvalues/unstr.zip",
            "columnvaluesunstr.zip,b9740dcd0553e11b526450ceb8f76683",
        ),
    ),
    ids=str,
)
def test_safename(data):
    result = httplib2.safename(data[0])
    assert result == data[1]


def test_safename2():
    assert httplib2.safename("http://www") != httplib2.safename("https://www")

    # Test the max length limits
    uri = "http://" + ("w" * 200) + ".org"
    uri2 = "http://" + ("w" * 201) + ".org"
    assert httplib2.safename(uri) != httplib2.safename(uri2)
    # Max length should be 90 + 1 (',') + 32 = 123
    assert len(httplib2.safename(uri2)) == 123
    assert len(httplib2.safename(uri)) == 123
