"""
werkzeug
~~~~~~~~

Werkzeug is the Swiss Army knife of Python web development.

It provides useful classes and functions for any WSGI application to
make the life of a Python web developer much easier. All of the provided
classes are independent from each other so you can mix it with any other
library.

:copyright: 2007 Pallets
:license: BSD-3-Clause
"""
from .serving import run_simple
from .test import Client
from .wrappers import Request
from .wrappers import Response

__version__ = "1.0.1"
