# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from telemetry.core import exceptions

# Keep this in sync with kTargetClosedMessage in
# content/browser/devtools/devtools_session.cc
# The hardcoding of the message is unfortunate, but we need it to distinguish
# real evaluation errors (syntax error) from this recoverable DevTool error.
TARGET_CLOSED_MESSAGE = "Inspected target navigated or closed"

class InspectorRuntime():
  def __init__(self, inspector_websocket):
    self._inspector_websocket = inspector_websocket
    self._inspector_websocket.RegisterDomain('Runtime', self._OnNotification)
    self._contexts_enabled = False
    self._all_context_ids = None

  def _OnNotification(self, msg):
    if (self._contexts_enabled and
        msg['method'] == 'Runtime.executionContextCreated'):
      self._all_context_ids.add(msg['params']['context']['id'])

  def Execute(self, expr, context_id, timeout, user_gesture=False):
    self.Evaluate(expr + '; 0;', context_id, timeout, user_gesture)

  def Evaluate(self, expr, context_id, timeout, user_gesture=False,
               promise=False):
    """Evaluates a javascript expression and returns the result.

    |context_id| can refer to an iframe. The main page has context_id=1, the
    first iframe context_id=2, etc.

    Raises:
      exceptions.EvaluateException
      exceptions.WebSocketDisconnected
      inspector_websocket.WebSocketException
      socket.error
    """
    request = {
        'method': 'Runtime.evaluate',
        'params': {
            'expression': expr,
            'returnByValue': True,
            'userGesture': user_gesture,
            'awaitPromise': promise,
        }
    }
    if context_id is not None:
      self.EnableAllContexts()
      request['params']['contextId'] = context_id
    res = self._inspector_websocket.SyncRequest(request, timeout)
    if 'error' in res:
      msg = res['error']['message']
      if msg == TARGET_CLOSED_MESSAGE:
        raise exceptions.DevtoolsTargetClosedException(msg)
      raise exceptions.EvaluateException(msg)

    if 'exceptionDetails' in res['result']:
      details = res['result']['exceptionDetails']
      raise exceptions.EvaluateException(
          text=details['text'],
          class_name=details.get('exception', {}).get('className'),
          description=details.get('exception', {}).get('description'))

    if res['result']['result']['type'] == 'undefined':
      return None
    return res['result']['result']['value']

  def EnableAllContexts(self):
    """Allow access to iframes.

    Raises:
      exceptions.WebSocketDisconnected
      inspector_websocket.WebSocketException
      socket.error
    """
    if not self._contexts_enabled:
      self._contexts_enabled = True
      self._all_context_ids = set()
      self._inspector_websocket.SyncRequest({'method': 'Runtime.enable'},
                                            timeout=30)
      # Disable capturing of stack traces for the inspector to minimize the
      # performance impact of the inspector on the page (crbug/1280831).
      self._inspector_websocket.SyncRequest({
          'method': 'Runtime.setMaxCallStackSizeToCapture',
          'params': {
              'size': 0,
          }
      }, timeout=30)
    return self._all_context_ids

  def CrashRendererProcess(self, context_id, timeout):
    request = {
        'method': 'Page.crash',
    }
    if context_id is not None:
      self.EnableAllContexts()
      request['params']['contextId'] = context_id
    res = self._inspector_websocket.SyncRequest(request, timeout)
    if 'error' in res:
      raise exceptions.EvaluateException(res['error']['message'])
    return res

  def CrashGpuProcess(self, timeout):
    res = self._inspector_websocket.SyncRequest(
        {'method': 'Browser.crashGpuProcess'}, timeout)
    if 'error' in res:
      raise exceptions.EvaluateException(res['error']['message'])
    return res

  def RunInspectorCommand(self, command, timeout):
    """Runs an inspector command.

    Raises:
      exceptions.WebSocketDisconnected
      inspector_websocket.WebSocketException
      socket.error
    """
    res = self._inspector_websocket.SyncRequest(command, timeout)
    return res
