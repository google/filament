'''
Iterative JSON parser.

Main API:

- ``ijson.parse``: iterator returning parsing events with the object tree context,
  see ``ijson.common.parse`` for docs.

- ``ijson.items``: iterator returning Python objects found under a specified prefix,
  see ``ijson.common.items`` for docs.

Top-level ``ijson`` module exposes method from the pure Python backend. There's
also two other backends using the C library yajl in ``ijson.backends`` that have
the same API and are faster under CPython.
'''
from ijson.common import JSONError, IncompleteJSONError, ObjectBuilder
import ijson.backends.python as backend


__version__ = '2.3'


basic_parse = backend.basic_parse
parse = backend.parse
items = backend.items
