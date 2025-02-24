# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""A wrapper for stored_object that separates internal and external."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from google.appengine.ext import ndb

from dashboard.common import datastore_hooks
from dashboard.common import stored_object


@ndb.synctasklet
def Get(key):
  """Gets either the external or internal copy of an object."""
  result = yield GetAsync(key)
  raise ndb.Return(result)


@ndb.tasklet
def GetAsync(key):
  namespaced_key = NamespaceKey(key)
  result = yield stored_object.GetAsync(namespaced_key)
  raise ndb.Return(result)


@ndb.synctasklet
def GetExternal(key):
  """Gets the external copy of a stored object."""
  result = yield GetExternalAsync(key)
  raise ndb.Return(result)


@ndb.tasklet
def GetExternalAsync(key):
  namespaced_key = NamespaceKey(key, datastore_hooks.EXTERNAL)
  result = yield stored_object.GetAsync(namespaced_key)
  raise ndb.Return(result)


@ndb.synctasklet
def Set(key, value):
  """Sets the the value of a stored object, either external or internal."""
  yield SetAsync(key, value)


@ndb.tasklet
def SetAsync(key, value):
  namespaced_key = NamespaceKey(key)
  yield stored_object.SetAsync(namespaced_key, value)


@ndb.synctasklet
def SetExternal(key, value):
  """Sets the external copy of a stored object."""
  yield SetExternalAsync(key, value)


@ndb.tasklet
def SetExternalAsync(key, value):
  namespaced_key = NamespaceKey(key, datastore_hooks.EXTERNAL)
  yield stored_object.SetAsync(namespaced_key, value)


@ndb.synctasklet
def Delete(key):
  """Deletes both the internal and external copy of a stored object."""
  yield DeleteAsync(key)


@ndb.tasklet
def DeleteAsync(key):
  internal_key = NamespaceKey(key, namespace=datastore_hooks.INTERNAL)
  external_key = NamespaceKey(key, namespace=datastore_hooks.EXTERNAL)
  yield (stored_object.DeleteAsync(internal_key),
         stored_object.DeleteAsync(external_key))


def NamespaceKey(key, namespace=None):
  """Prepends a namespace string to a key string."""
  if not namespace:
    namespace = datastore_hooks.GetNamespace()
  return '%s__%s' % (namespace, key)
