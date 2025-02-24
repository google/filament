# -*- coding: utf-8 -*-
"""
    webapp2_extras.sessions_ndb
    ===========================

    Extended sessions stored in datastore using the ndb library.

    App Engine-specific modules were moved to webapp2_extras.appengine.
    This module is here for compatibility purposes.

    :copyright: 2011 by tipfy.org.
    :license: Apache Sotware License, see LICENSE for details.
"""
import warnings

warnings.warn(DeprecationWarning(
    'webapp2_extras.sessions_ndb is deprecated. '
    'App Engine-specific modules were moved to webapp2_extras.appengine.'),
    stacklevel=1)
from webapp2_extras.appengine.sessions_ndb import *
