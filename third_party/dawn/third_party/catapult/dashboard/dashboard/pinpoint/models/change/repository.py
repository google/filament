# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging

from google.appengine.ext import ndb


@ndb.transactional(propagation=ndb.TransactionOptions.INDEPENDENT)
def RepositoryUrl(name):
  """Returns the URL of a repository, given its short name.

  If a repository moved locations or has multiple locations, a repository can
  have multiple URLs. The returned URL should be the current canonical one.

  Args:
    name: The short name of the repository.

  Returns:
    A URL string, not including '.git'.
  """
  repository = ndb.Key(Repository, name).get()
  if not repository:
    raise KeyError('Unknown repository name: ' + name)
  return repository.urls[0]


def RepositoryName(url, add_if_missing=False):
  """Returns the short repository name, given its URL.

  By default, the short repository name is the last part of the URL.
  E.g. "https://chromium.googlesource.com/v8/v8": "v8"
  In some cases this is ambiguous, so the names can be manually adjusted.
  E.g. "../chromium/src": "chromium" and "../breakpad/breakpad/src": "breakpad"

  If a repository moved locations or has multiple locations, multiple URLs can
  map to the same name. This should only be done if they are exact mirrors and
  have the same git hashes.
  "https://webrtc.googlesource.com/src": "webrtc"
  "https://webrtc.googlesource.com/src/webrtc": "old_webrtc"
  "https://chromium.googlesource.com/external/webrtc/trunk/webrtc": "old_webrtc"

  Internally, all repositories are stored by short name, which always maps to
  the current canonical URL, so old URLs are automatically "upconverted".

  Args:
    url: The repository URL.
    add_if_missing: If True, also attempts to add the URL to the database with
      the default name mapping. Throws an exception if there's a name collision.

  Returns:
    The short name as a string.

  Raises:
    AssertionError: add_if_missing is True and there's a name collision.
  """
  if url.endswith('.git'):
    url = url[:-4]

  repositories = Repository.query(Repository.urls == url).fetch()
  if repositories:
    return repositories[0].key.id()

  if add_if_missing:
    return _AddRepository(url)

  raise KeyError('Unknown repository URL: %s' % url)


def _AddRepository(url):
  """Add a repository URL to the database with the default name mapping.

  The default short repository name is the last part of the URL.

  Returns:
    The short repository name.

  Raises:
    AssertionError: The default name is already in the database.
  """
  name = url.split('/')[-1]

  repository = ndb.Key(Repository, name).get()
  if repository:
    current_url = repository.urls[0] if len(repository.urls) > 0 else ''
    logging.info('Replacing the repository %s. Old url %s, new url %s',
                    name, current_url, url)
    repository.urls = [url]
    repository.put()
  else:
    Repository(id=name, urls=[url]).put()
  return name


class Repository(ndb.Model):
  _use_memcache = True
  _use_cache = True
  _memcache_timeout = 60 * 60 * 24 * 7  # 7 days worth of caching.
  urls = ndb.StringProperty(repeated=True)
