# -*- coding: utf-8 -*-
"""
    webapp2_extras.local_app
    ~~~~~~~~~~~~~~~~~~~~~~~~

    This module is deprecated. The functionality is now available
    directly in webapp2.

    Previously it implemented a WSGIApplication adapted for threaded
    environments.

    :copyright: 2011 by tipfy.org.
    :license: Apache Sotware License, see LICENSE for details.
"""
import warnings

import webapp2

warnings.warn(DeprecationWarning(
    'webapp2_extras.local_app is deprecated. webapp2.WSGIApplication is now '
    'thread-safe by default when webapp2_extras.local is available.'),
    stacklevel=1)

WSGIApplication = webapp2.WSGIApplication
