# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry.core import exceptions


class InspectorStorage():
  def __init__(self, inspector_websocket):
    self._websocket = inspector_websocket
    self._websocket.RegisterDomain('Storage', self._OnNotification)
    self._shared_storage_notifications_enabled = False
    self._shared_storage_notifications = []

  def _OnNotification(self, msg):
    if msg['method'] == 'Storage.sharedStorageAccessed':
      self._shared_storage_notifications.append(msg['params'])

    # TODO: track other storage events
    # (https://chromedevtools.github.io/devtools-protocol/tot/Storage/)

  def ClearDataForOrigin(self, url, timeout):
    res = self._websocket.SyncRequest(
        {'method': 'Storage.clearDataForOrigin',
         'params': {
             'origin': url,
             'storageTypes': 'all',
         }}, timeout)
    if 'error' in res:
      raise exceptions.StoryActionError(res['error']['message'])

  def EnableSharedStorageNotifications(self, timeout=60):
    if self._shared_storage_notifications_enabled:
      return
    request = {'method': 'Storage.setSharedStorageTracking',
               'params': {'enable': True}}
    res = self._websocket.SyncRequest(request, timeout)
    if 'error' in res:
      raise exceptions.StoryActionError(res['error']['message'])
    assert len(res['result']) == 0
    self._shared_storage_notifications_enabled = True

  def DisableSharedStorageNotifications(self, timeout=60):
    if not self._shared_storage_notifications_enabled:
      return
    request = {'method': 'Storage.setSharedStorageTracking',
               'params': {'enable': False}}
    res = self._websocket.SyncRequest(request, timeout)
    if 'error' in res:
      raise exceptions.StoryActionError(res['error']['message'])
    assert len(res['result']) == 0
    self._shared_storage_notifications_enabled = False

  @property
  def shared_storage_notifications(self):
    return self._shared_storage_notifications

  def ClearSharedStorageNotifications(self):
    self._shared_storage_notifications = []

  @property
  def shared_storage_notifications_enabled(self):
    return self._shared_storage_notifications_enabled

  def GetSharedStorageMetadata(self, origin, timeout=60):
    request = {'method': 'Storage.getSharedStorageMetadata',
               'params': {'ownerOrigin': origin}}
    res = self._websocket.SyncRequest(request, timeout)
    if 'error' in res:
      if res['error']['message'] == 'Origin not found.':
        # Send a newly created "metadata" dict, since DevTools throws an
        # error if `origin` isn't in the shared storage database yet.
        return {'creationTime': None, 'length': 0, 'remainingBudget': None}
      raise exceptions.StoryActionError(res['error']['message'])
    assert len(res['result']) > 0
    if 'metadata' not in res['result']:
      raise exceptions.StoryActionError("Response missing metadata: "
                                        + res['result'])
    return res['result']['metadata']

  def GetSharedStorageEntries(self, origin, timeout=60):
    request = {'method': 'Storage.getSharedStorageEntries',
               'params': {'ownerOrigin': origin}}
    res = self._websocket.SyncRequest(request, timeout)
    if 'error' in res:
      raise exceptions.StoryActionError(res['error']['message'])
    assert len(res['result']) > 0
    if 'entries' not in res['result']:
      raise exceptions.StoryActionError("Response missing entries: "
                                        + res['result'])
    return res['result']['entries']
