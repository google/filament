# -*- coding: utf-8 -*-
"""
    webapp2_extras.users
    ====================

    Helpers for google.appengine.api.users.

    App Engine-specific modules were moved to webapp2_extras.appengine.
    This module is here for compatibility purposes.

    :copyright: 2011 tipfy.org.
    :license: Apache Sotware License, see LICENSE for details.
"""
import warnings

warnings.warn(DeprecationWarning(
    'webapp2_extras.users is deprecated. '
    'App Engine-specific modules were moved to webapp2_extras.appengine.'),
    stacklevel=1)
from webapp2_extras.appengine.users import *
