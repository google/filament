# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import time

from telemetry.util import image_util


class InspectorPage():
  """Class that controls a page connected by an inspector_websocket.

  This class provides utility methods for controlling a page connected by an
  inspector_websocket. It does not perform any exception handling. All
  inspector_websocket exceptions must be handled by the caller.
  """
  def __init__(self, inspector_websocket, timeout):
    self._inspector_websocket = inspector_websocket
    self._inspector_websocket.RegisterDomain('Page', self._OnNotification)

    self._navigation_pending = False
    self._navigation_url = ''  # Support for legacy backends.
    self._navigation_frame_id = ''
    self._navigated_frame_ids = None  # Holds frame ids while navigating.
    self._script_to_evaluate_on_commit = None
    # Turn on notifications. We need them to get the Page.frameNavigated event.
    self._EnablePageNotifications(timeout=timeout)

  def _OnNotification(self, msg):
    if msg['method'] == 'Page.frameNavigated':
      url = msg['params']['frame']['url']
      if not self._navigated_frame_ids is None:
        frame_id = msg['params']['frame']['id']
        if self._navigation_frame_id == frame_id:
          self._navigation_frame_id = ''
          self._navigated_frame_ids = None
          self._navigation_pending = False
        else:
          self._navigated_frame_ids.add(frame_id)
      elif self._navigation_url == url:
        # TODO(tonyg): Remove this when Chrome 38 goes stable.
        self._navigation_url = ''
        self._navigation_pending = False
      elif (not url == 'chrome://newtab/' and not url == 'about:blank' and
            not 'parentId' in msg['params']['frame']):
        # Marks the navigation as complete and unblocks the
        # WaitForNavigate call.
        self._navigation_pending = False

  def _SetScriptToEvaluateOnCommit(self, source, timeout):
    existing_source = (self._script_to_evaluate_on_commit and
                       self._script_to_evaluate_on_commit['source'])
    if source == existing_source:
      return
    if existing_source:
      request = {
          'method': 'Page.removeScriptToEvaluateOnLoad',
          'params': {
              'identifier': self._script_to_evaluate_on_commit['id'],
              }
          }
      self._inspector_websocket.SyncRequest(request, timeout)
      self._script_to_evaluate_on_commit = None
    if source:
      request = {
          'method': 'Page.addScriptToEvaluateOnLoad',
          'params': {
              'scriptSource': source,
              }
          }
      res = self._inspector_websocket.SyncRequest(request, timeout)
      self._script_to_evaluate_on_commit = {
          'id': res['result']['identifier'],
          'source': source
          }

  def _EnablePageNotifications(self, timeout=60):
    request = {
        'method': 'Page.enable'
        }
    res = self._inspector_websocket.SyncRequest(request, timeout)
    assert len(res['result']) == 0

  def WaitForNavigate(self, timeout=60):
    """Waits for the navigation to complete.

    The current page is expect to be in a navigation. This function returns
    when the navigation is complete or when the timeout has been exceeded.
    """
    start_time = time.time()
    remaining_time = timeout
    self._navigation_pending = True
    while self._navigation_pending and remaining_time > 0:
      remaining_time = max(timeout - (time.time() - start_time), 0.0)
      self._inspector_websocket.DispatchNotifications(remaining_time)

  def Navigate(self, url, script_to_evaluate_on_commit=None, timeout=60):
    """Navigates to |url|.

    If |script_to_evaluate_on_commit| is given, the script source string will be
    evaluated when the navigation is committed. This is after the context of
    the page exists, but before any script on the page itself has executed.
    """

    self._SetScriptToEvaluateOnCommit(script_to_evaluate_on_commit, timeout)
    request = {
        'method': 'Page.navigate',
        'params': {
            'url': url,
            }
        }
    self._navigated_frame_ids = set()
    res = self._inspector_websocket.SyncRequest(request, timeout)
    if 'frameId' in res['result']:
      # Modern backends are returning frameId from Page.navigate.
      # Use it here to unblock upon precise navigation.
      frame_id = res['result']['frameId']
      if self._navigated_frame_ids and frame_id in self._navigated_frame_ids:
        self._navigated_frame_ids = None
        return
      self._navigation_frame_id = frame_id
    else:
      # TODO(tonyg): Remove this when Chrome 38 goes stable.
      self._navigated_frame_ids = None
      self._navigation_url = url
    self.WaitForNavigate(timeout)

  def CaptureScreenshot(self, timeout=60):
    """Captures a screenshot of the visible web contents.

    Includes scroll bars if the full web contents do not fit into the current
    viewport.

    Returns:
      An image in whatever format telemetry.util.image_util has chosen to use,
      or None if screenshot capture failed.
    """
    request = {
        'method': 'Page.captureScreenshot',
        'params': {
            # TODO(rmistry): when Chrome is running in headless mode, this
            # will need to pass True. Telemetry needs to understand
            # whether the browser is in headless mode, and pass that
            # knowledge down to this method.
            'fromSurface': False
            }
        }
    return self._CaptureScreenshotImpl(request, timeout)

  def CaptureFullScreenshot(self, timeout=60):
    """Captures a screenshot of the full web contents.

    Shouldn't contain any scroll bars.

    Returns:
      An image in whatever format telemetry.util.image_util has chosen to use,
      or None if screenshot capture failed.
    """
    content_width, content_height = self.GetContentDimensions()
    self.SetEmulatedWindowDimensions(content_width, content_height)
    request = {
        'method': 'Page.captureScreenshot',
        'params': {
            # fromSurface must be true to actually capture the full web contents
            # if they do not fit into the viewport.
            'fromSurface': True,
        },
    }
    screenshot = self._CaptureScreenshotImpl(request, timeout)
    self.ClearEmulatedWindowDimensions()
    return screenshot

  def _CaptureScreenshotImpl(self, request, timeout):
    # "Google API are missing..." infobar might cause a viewport resize
    # which invalidates screenshot request. See crbug.com/459820.
    for _ in range(2):
      res = self._inspector_websocket.SyncRequest(request, timeout)
      if res and ('result' in res) and ('data' in res['result']):
        return image_util.FromBase64Png(res['result']['data'])
    return None

  def GetContentDimensions(self, timeout=60):
    """Gets the width/height of the page contents.

    Returns:
      A tuple (width, height) in pixels.
    """
    request = {
        'method': 'Page.getLayoutMetrics',
    }
    res = self._inspector_websocket.SyncRequest(request, timeout)
    dom_rect = res['result']['contentSize']
    return dom_rect['width'], dom_rect['height']

  def SetEmulatedWindowDimensions(self, width, height, timeout=60):
    """Sets the emulated window dimensions for the page.

    Args:
      width: An int specifying the width of the window in pixels.
      height: An int specifying the height of the window in pixels.
    """
    request = {
        'method': 'Emulation.setDeviceMetricsOverride',
        'params': {
            'width': width,
            'height': height,
            'deviceScaleFactor': 0,
            'mobile': False,
        }
    }
    res = self._inspector_websocket.SyncRequest(request, timeout)
    assert 'result' in res

  def ClearEmulatedWindowDimensions(self, timeout=60):
    """Clears the emulated window dimensions, restoring them to real values."""
    request = {
        'method': 'Emulation.clearDeviceMetricsOverride',
    }
    res = self._inspector_websocket.SyncRequest(request, timeout)
    assert 'result' in res

  def CollectGarbage(self, timeout=60):
    request = {
        'method': 'HeapProfiler.collectGarbage'
        }
    res = self._inspector_websocket.SyncRequest(request, timeout)
    assert 'result' in res
