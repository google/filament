#
# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
from time import time

class Progress(object):
  def __init__(self, title, total=0):
    self._title = title
    self._total = total
    self._done = 0
    self._lastp = -1
    self._start = time()
    self._show = False
    self._width = 0

  def update(self, inc=1, extra=''):
    self._done += inc

    if not self._show:
      if 0.5 <= time() - self._start:
        self._show = True
      else:
        return

    text = None

    if self._total <= 0:
      text = '%s: %3d' % (self._title, self._done)
    else:
      p = (100 * self._done) / self._total

      if self._lastp != p:
        self._lastp = p
        text = '%s: %3d%% (%2d/%2d)' % (self._title, p,
                                        self._done, self._total)

    if text:
      text += ' ' + extra
      text = text[:self.terminal_width()]  # Avoid wrapping
      spaces = max(self._width - len(text), 0)
      sys.stdout.write('%s%*s\r' % (text, spaces, ''))
      sys.stdout.flush()
      self._width = len(text)

  def end(self):
    if not self._show:
      return

    if self._total <= 0:
      text = '%s: %d, done.' % (
        self._title,
        self._done)
    else:
      p = (100 * self._done) / self._total
      text = '%s: %3d%% (%d/%d), done.' % (
        self._title,
        p,
        self._done,
        self._total)

    spaces = max(self._width - len(text), 0)
    sys.stdout.write('%s%*s\n' % (text, spaces, ''))
    sys.stdout.flush()

  def terminal_width(self):
    """Returns sys.maxsize if the width cannot be determined."""
    try:
      if not sys.stdout.isatty():
        return sys.maxsize
      if sys.platform == 'win32':
        import ctypes
        class CONSOLE_SCREEN_BUFFER_INFO(ctypes.Structure):
          _fields_ = [
            ('dwSize', ctypes.wintypes._COORD),
            ('dwCursorPosition', ctypes.wintypes._COORD),
            ('wAttributes', ctypes.c_ushort),
            ('srWindow', ctypes.wintypes._SMALL_RECT),
            ('dwMaximumWindowSize', ctypes.wintypes._COORD)
          ]
        # Get our own instance of the kernel lib since python
        # libraries and python ports are known to manipulate
        # ctypes.windll.kernel32. See
        # https://github.com/pallets/click/issues/126
        kernel32 = ctypes.WinDLL('kernel32')
        handle = kernel32.GetStdHandle(-12)  # -12 == stderr
        console_screen_buffer_info = CONSOLE_SCREEN_BUFFER_INFO()
        if kernel32.GetConsoleScreenBufferInfo(
            ctypes.c_ulong(handle),
            ctypes.byref(console_screen_buffer_info)):
          # Note that we return 1 less than the width since writing into
          # the rightmost column automatically performs a line feed.
          right = console_screen_buffer_info.srWindow.Right
          left = console_screen_buffer_info.srWindow.Left
          return right - left
        return sys.maxsize
      else:
        import fcntl
        import struct
        import termios
        packed = fcntl.ioctl(sys.stderr.fileno(), termios.TIOCGWINSZ, '\0' * 8)
        _, columns, _, _ = struct.unpack('HHHH', packed)
        return columns
    except Exception:  # pylint: disable=broad-except
      return sys.maxsize
