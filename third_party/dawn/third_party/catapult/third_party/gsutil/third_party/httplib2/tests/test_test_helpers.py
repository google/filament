import tests
import pytest
from six.moves import urllib


@pytest.mark.parametrize("path", ("_", "/"), ids=lambda x: "path=" + x)
@pytest.mark.parametrize("port", ("_", None, 81), ids=lambda x: "port={}".format(x))
@pytest.mark.parametrize("host", ("_", "1.1.1.1", "[fe80::1]", "fqdn."), ids=lambda x: "host=" + x)
@pytest.mark.parametrize("scheme", ("_", "http", "https", "random"), ids=lambda x: "scheme=" + x)
@pytest.mark.parametrize(
    "base", ("127.0.0.1:8001", "//[::1]/path1", "https://foo.bar:8443/p?query",), ids=lambda x: "base=" + x,
)
def test_rebuild_uri(base, scheme, host, port, path):
    ubase = urllib.parse.urlsplit("//" + base if "//" not in base else base)
    kwargs_prepare = {"scheme": scheme, "host": host, "port": port, "path": path}
    kwargs = {k: v for k, v in kwargs_prepare.items() if v != "_"}
    result = tests.rebuild_uri(base, **kwargs)
    u2 = urllib.parse.urlsplit(result)
    describe = "rebuild_uri({}, {}) = {}".format(
        base, ", ".join("{}={}".format(k, v) for k, v in kwargs.items()), result
    )
    expect = lambda x, y: x if x != "_" else None or y
    assert u2.scheme == expect(scheme, ubase.scheme), describe
    assert u2.hostname == expect(host, ubase.hostname).strip("[]"), describe
    assert u2.port == expect(port, ubase.port), describe
    assert u2.path == expect(path, ubase.path), describe
