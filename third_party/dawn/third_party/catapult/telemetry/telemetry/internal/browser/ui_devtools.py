# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

MOUSE_EVENT_TYPE_MOUSE_PRESSED = 'mousePressed'
MOUSE_EVENT_TYPE_MOUSE_DRAGGED = 'mouseDragged'
MOUSE_EVENT_TYPE_MOUSE_RELEASED = 'mouseReleased'
MOUSE_EVENT_TYPE_MOUSE_MOVED = 'mouseMoved'
MOUSE_EVENT_TYPE_MOUSE_ENTERED = 'mouseEntered'
MOUSE_EVENT_TYPE_MOUSE_EXITED = 'mouseExited'
MOUSE_EVENT_TYPE_MOUSE_WHEEL = 'mouseWheel'

MOUSE_EVENT_BUTTON_NONE = 'none'
MOUSE_EVENT_BUTTON_LEFT = 'left'
MOUSE_EVENT_BUTTON_RIGHT = 'right'
MOUSE_EVENT_BUTTON_MIDDLE = 'middle'
MOUSE_EVENT_BUTTON_BACK = 'back'
MOUSE_EVENT_BUTTON_FORWARD = 'forward'

MOUSE_EVENT_WHEEL_DIRECTION_NONE = 'none'
MOUSE_EVENT_WHEEL_DIRECTION_UP = 'up'
MOUSE_EVENT_WHEEL_DIRECTION_DOWN = 'down'
MOUSE_EVENT_WHEEL_DIRECTION_LEFT = 'left'
MOUSE_EVENT_WHEEL_DIRECTION_RIGHT = 'right'

KEY_EVENT_TYPE_KEY_PRESSED = 'keyPressed'
KEY_EVENT_TYPE_KEY_RELEASED = 'keyReleased'


class UIDevTools():
  """This class is mainly used to interact with native UI

  For more info see the Desktop UI benchmark documentation in the link below
  https://chromium.googlesource.com/chromium/src/+/master/docs/speed/benchmark/harnesses/desktop_ui.md
  """
  def __init__(self, ui_devtools_backend):
    self._ui_devtools_backend = ui_devtools_backend

  def Close(self):
    self._ui_devtools_backend.Close()

  def QueryNodes(self, query):
    return self._ui_devtools_backend.QueryNodes(query)

  # pylint: disable=redefined-builtin
  def DispatchMouseEvent(self,
                         node_id,
                         type=MOUSE_EVENT_TYPE_MOUSE_PRESSED,
                         x=0,
                         y=0,
                         button=MOUSE_EVENT_BUTTON_LEFT,
                         wheel_direction=MOUSE_EVENT_WHEEL_DIRECTION_NONE):
    return self._ui_devtools_backend.DispatchMouseEvent(node_id,
                                                        type,
                                                        x,
                                                        y,
                                                        button,
                                                        wheel_direction)

  # pylint: disable=redefined-builtin
  def DispatchKeyEvent(self,
                       node_id,
                       type=KEY_EVENT_TYPE_KEY_PRESSED,
                       key_code=0,
                       code=0,
                       flags=0,
                       key=0,
                       is_char=False):
    return self._ui_devtools_backend.DispatchKeyEvent(node_id,
                                                      type,
                                                      key_code,
                                                      code,
                                                      flags,
                                                      key,
                                                      is_char)
