# -*- coding: utf-8 -*-
"""
    webapp2_extras.json
    ===================

    JSON helpers for webapp2.

    :copyright: 2011 by tipfy.org.
    :license: Apache Sotware License, see LICENSE for details.
"""
from __future__ import absolute_import

import base64
import urllib

try:
    # Preference for installed library with updated fixes.
    # Also available in Google App Engine SDK >= 1.4.2.
    import simplejson as json
except ImportError: # pragma: no cover
    try:
        # Standard library module in Python >= 2.6.
        import json
    except ImportError: # pragma: no cover
        raise RuntimeError(
            'A JSON parser is required, e.g., simplejson at '
            'http://pypi.python.org/pypi/simplejson/')

assert hasattr(json, 'loads') and hasattr(json, 'dumps'), \
    'Expected a JSON module with the functions loads() and dumps().'


def encode(value, *args, **kwargs):
    """Serializes a value to JSON.

    This comes from `Tornado`_.

    :param value:
        A value to be serialized.
    :param args:
        Extra arguments to be passed to `json.dumps()`.
    :param kwargs:
        Extra keyword arguments to be passed to `json.dumps()`.
    :returns:
        The serialized value.
    """
    # By default encode using a compact format.
    kwargs.setdefault('separators', (',', ':'))
    # JSON permits but does not require forward slashes to be escaped.
    # This is useful when json data is emitted in a <script> tag
    # in HTML, as it prevents </script> tags from prematurely terminating
    # the javascript.  Some json libraries do this escaping by default,
    # although python's standard library does not, so we do it here.
    # See: http://goo.gl/WsXwv
    return json.dumps(value, *args, **kwargs).replace("</", "<\\/")


def decode(value, *args, **kwargs):
    """Deserializes a value from JSON.

    This comes from `Tornado`_.

    :param value:
        A value to be deserialized.
    :param args:
        Extra arguments to be passed to `json.loads()`.
    :param kwargs:
        Extra keyword arguments to be passed to `json.loads()`.
    :returns:
        The deserialized value.
    """
    if isinstance(value, str):
        value = value.decode('utf-8')

    assert isinstance(value, unicode)
    return json.loads(value, *args, **kwargs)


def b64encode(value, *args, **kwargs):
    """Serializes a value to JSON and encodes it using base64.

    Parameters and return value are the same from :func:`encode`.
    """
    return base64.b64encode(encode(value, *args, **kwargs))


def b64decode(value, *args, **kwargs):
    """Decodes a value using base64 and deserializes it from JSON.

    Parameters and return value are the same from :func:`decode`.
    """
    return decode(base64.b64decode(value), *args, **kwargs)


def quote(value, *args, **kwargs):
    """Serializes a value to JSON and encodes it using urllib.quote.

    Parameters and return value are the same from :func:`encode`.
    """
    return urllib.quote(encode(value, *args, **kwargs))


def unquote(value, *args, **kwargs):
    """Decodes a value using urllib.unquote and deserializes it from JSON.

    Parameters and return value are the same from :func:`decode`.
    """
    return decode(urllib.unquote(value), *args, **kwargs)
