# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Database model for page state data.

This is used for storing front-end configuration.
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from google.appengine.ext import ndb


class PageState(ndb.Model):
  """An entity with a single blob value where id is a hash value."""

  value = ndb.BlobProperty(indexed=False)

  value_v2 = ndb.BlobProperty(indexed=False)
