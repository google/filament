# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from google.appengine.ext import ndb

_MEMCACHE_TIMEOUT = 60 * 60 * 24 * 30


def Get(id_string):
  """Retrieve cached commit or patch details from the Datastore.

  Args:
    id_string: A string that uniquely identifies a Commit or Patch.

  Returns:
    A dict with the fields {'url', 'author', created', 'subject', 'message'}.
  """
  entity = ndb.Key(Commit, id_string).get(use_datastore=False)
  if not entity:
    raise KeyError('Commit or Patch not found in the Datastore:\n' +
                   str(id_string))

  return {
      'url': entity.url,
      'author': entity.author,
      'created': entity.created,
      'subject': entity.subject,
      'message': entity.message,
  }


def Put(id_string,
        url,
        author,
        created,
        subject,
        message,
        memcache_timeout=_MEMCACHE_TIMEOUT):
  """Add commit or patch details to the Datastore cache.

  Args:
    id_string: A string that uniquely identifies a Commit or Patch.
    url: The URL of the Commit or Patch.
    author: The author of the Commit or Patch.
    created: A datetime. When the Commit was committed or the Patch was created.
    subject: The title/subject line of the Commit or Patch.
    message: The Commit message.
  """
  if not memcache_timeout:
    memcache_timeout = _MEMCACHE_TIMEOUT

  Commit(
      url=url,
      author=author,
      created=created,
      subject=subject,
      message=message,
      id=id_string).put(
          use_datastore=False, memcache_timeout=memcache_timeout)


class Commit(ndb.Model):
  # Never write/read from Datastore.
  _use_datastore = False

  # Rely on this model being cached only in memory or memcache.
  _use_memcache = True
  _use_cache = True

  # Cache the data in Memcache for up-to 30 days
  _memcache_timeout = _MEMCACHE_TIMEOUT

  url = ndb.StringProperty(indexed=False, required=True)
  author = ndb.StringProperty(indexed=False, required=True)
  created = ndb.DateTimeProperty(indexed=False, required=True)
  subject = ndb.StringProperty(indexed=False, required=True)
  message = ndb.TextProperty(required=True)
