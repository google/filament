# Copyright 2012 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
# either express or implied. See the License for the specific
# language governing permissions and limitations under the License.

"""Base and helper classes for Google RESTful APIs."""





from __future__ import absolute_import
import six
__all__ = ['add_sync_methods']

import logging
import os
import random
import time

from . import api_utils

try:
  from google.appengine.api import app_identity
  from google.appengine.api import lib_config
  from google.appengine.ext import ndb
except ImportError:
  from google.appengine.api import app_identity
  from google.appengine.api import lib_config
  from google.appengine.ext import ndb



@ndb.tasklet
def _make_token_async(scopes, service_account_id):
  """Get a fresh authentication token.

  Args:
    scopes: A list of scopes.
    service_account_id: Internal-use only.

  Raises:
    An ndb.Return with a tuple (token, expiration_time) where expiration_time is
    seconds since the epoch.
  """
  if six.PY2:
    rpc = app_identity.create_rpc()
    app_identity.make_get_access_token_call(rpc, scopes, service_account_id)
    token, expires_at = yield rpc
    raise ndb.Return((token, expires_at))
  else:
    # make_get_access_token_call is removed in Python 3.
    raise ndb.Return(app_identity.get_access_token(scopes, service_account_id))


class _ConfigDefaults(object):
  TOKEN_MAKER = _make_token_async

_config = lib_config.register('cloudstorage', _ConfigDefaults.__dict__)


def _make_sync_method(name):
  """Helper to synthesize a synchronous method from an async method name.

  Used by the @add_sync_methods class decorator below.

  Args:
    name: The name of the synchronous method.

  Returns:
    A method (with first argument 'self') that retrieves and calls
    self.<name>, passing its own arguments, expects it to return a
    Future, and then waits for and returns that Future's result.
  """

  def sync_wrapper(self, *args, **kwds):
    method = getattr(self, name)
    future = method(*args, **kwds)
    return future.get_result()

  return sync_wrapper


def add_sync_methods(cls):
  """Class decorator to add synchronous methods corresponding to async methods.

  This modifies the class in place, adding additional methods to it.
  If a synchronous method of a given name already exists it is not
  replaced.

  Args:
    cls: A class.

  Returns:
    The same class, modified in place.
  """
  for name in list(cls.__dict__.keys()):
    if name.endswith('_async'):
      sync_name = name[:-6]
      if not hasattr(cls, sync_name):
        setattr(cls, sync_name, _make_sync_method(name))
  return cls


class _AE_TokenStorage_(ndb.Model):
  """Entity to store app_identity tokens in memcache."""

  token = ndb.StringProperty()
  expires = ndb.FloatProperty()


class _RestApi(object):
  """Base class for REST-based API wrapper classes.

  This class manages authentication tokens and request retries.  All
  APIs are available as synchronous and async methods; synchronous
  methods are synthesized from async ones by the add_sync_methods()
  function in this module.

  WARNING: Do NOT directly use this api. It's an implementation detail
  and is subject to change at any release.
  """

  def __init__(self, scopes, service_account_id=None, token_maker=None,
               retry_params=None):
    """Constructor.

    Args:
      scopes: A scope or a list of scopes.
      service_account_id: Internal use only.
      token_maker: An asynchronous function of the form
        (scopes, service_account_id) -> (token, expires).
      retry_params: An instance of api_utils.RetryParams. If None, the
        default for current thread will be used.
    """

    if isinstance(scopes, six.string_types):
      scopes = [scopes]
    self.scopes = scopes
    self.service_account_id = service_account_id
    self.make_token_async = token_maker or _config.TOKEN_MAKER
    if not retry_params:
      retry_params = api_utils._get_default_retry_params()
    self.retry_params = retry_params
    self.user_agent = {'User-Agent': retry_params._user_agent}
    self.expiration_headroom = random.randint(60, 240)

  def __getstate__(self):
    """Store state as part of serialization/pickling."""
    return {'scopes': self.scopes,
            'id': self.service_account_id,
            'a_maker': (None if self.make_token_async == _make_token_async
                        else self.make_token_async),
            'retry_params': self.retry_params,
            'expiration_headroom': self.expiration_headroom}

  def __setstate__(self, state):
    """Restore state as part of deserialization/unpickling."""
    self.__init__(state['scopes'],
                  service_account_id=state['id'],
                  token_maker=state['a_maker'],
                  retry_params=state['retry_params'])
    self.expiration_headroom = state['expiration_headroom']

  @ndb.tasklet
  def do_request_async(self, url, method='GET', headers=None, payload=None,
                       deadline=None, callback=None):
    """Issue one HTTP request.

    It performs async retries using tasklets.

    Args:
      url: the url to fetch.
      method: the method in which to fetch.
      headers: the http headers.
      payload: the data to submit in the fetch.
      deadline: the deadline in which to make the call.
      callback: the call to make once completed.

    Yields:
      The async fetch of the url.
    """
    retry_wrapper = api_utils._RetryWrapper(
        self.retry_params,
        retriable_exceptions=api_utils._RETRIABLE_EXCEPTIONS,
        should_retry=api_utils._should_retry)
    resp = yield retry_wrapper.run(
        self.urlfetch_async,
        url=url,
        method=method,
        headers=headers,
        payload=payload,
        deadline=deadline,
        callback=callback,
        follow_redirects=False)
    raise ndb.Return((resp.status_code, resp.headers, resp.content))

  @ndb.tasklet
  def get_token_async(self, refresh=False):
    """Get an authentication token.

    The token is cached in memcache, keyed by the scopes argument.
    Uses a random token expiration headroom value generated in the constructor
    to eliminate a burst of GET_ACCESS_TOKEN API requests.

    Args:
      refresh: If True, ignore a cached token; default False.

    Yields:
      An authentication token. This token is guaranteed to be non-expired.
    """
    key = '%s,%s' % (self.service_account_id, ','.join(self.scopes))
    ts = yield _AE_TokenStorage_.get_by_id_async(
        key,
        use_cache=True,
        use_memcache=self.retry_params.memcache_access_token,
        use_datastore=self.retry_params.save_access_token)
    if refresh or ts is None or ts.expires < (
        time.time() + self.expiration_headroom):
      token, expires_at = yield self.make_token_async(
          self.scopes, self.service_account_id)
      timeout = int(expires_at - time.time())
      ts = _AE_TokenStorage_(id=key, token=token, expires=expires_at)
      if timeout > 0:
        yield ts.put_async(memcache_timeout=timeout,
                           use_datastore=self.retry_params.save_access_token,
                           use_cache=True,
                           use_memcache=self.retry_params.memcache_access_token)
    raise ndb.Return(ts.token)

  @ndb.tasklet
  def urlfetch_async(self, url, method='GET', headers=None,
                     payload=None, deadline=None, callback=None,
                     follow_redirects=False):
    """Make an async urlfetch() call.

    This is an async wrapper around urlfetch(). It adds an authentication
    header.

    Args:
      url: the url to fetch.
      method: the method in which to fetch.
      headers: the http headers.
      payload: the data to submit in the fetch.
      deadline: the deadline in which to make the call.
      callback: the call to make once completed.
      follow_redirects: whether or not to follow redirects.

    Yields:
      This returns a Future despite not being decorated with @ndb.tasklet!
    """
    headers = {} if headers is None else dict(headers)
    headers.update(self.user_agent)
    try:
      self.token = yield self.get_token_async()
    except app_identity.InternalError as e:
      if os.environ.get('DATACENTER', '').endswith('sandman'):
        self.token = None
        logging.warning('Could not fetch an authentication token in sandman '
                     'based Appengine devel setup; proceeding without one.')
      else:
        raise e
    if self.token:
      headers['authorization'] = 'OAuth ' + self.token

    deadline = deadline or self.retry_params.urlfetch_timeout

    ctx = ndb.get_context()
    resp = yield ctx.urlfetch(
        url, payload=payload, method=method,
        headers=headers, follow_redirects=follow_redirects,
        deadline=deadline, callback=callback)
    raise ndb.Return(resp)


_RestApi = add_sync_methods(_RestApi)
