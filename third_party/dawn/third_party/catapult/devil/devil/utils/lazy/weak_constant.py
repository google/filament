# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import threading

from devil.utils import reraiser_thread
from devil.utils import timeout_retry


class WeakConstant(object):
  """A thread-safe, lazily initialized object.

  This does not support modification after initialization. The intended
  constant nature of the object is not enforced, though, hence the "weak".
  """

  def __init__(self, initializer):
    self._initialized = threading.Event()
    self._initializer = initializer
    self._lock = threading.Lock()
    self._val = None

  def read(self):
    """Get the object, creating it if necessary."""
    if self._initialized.is_set():
      return self._val
    with self._lock:
      if not self._initialized.is_set():
        # We initialize the value on a separate thread to protect
        # from holding self._lock indefinitely in the event that
        # self._initializer hangs.
        initializer_thread = reraiser_thread.ReraiserThread(self._initializer)
        initializer_thread.start()
        timeout_retry.WaitFor(
            lambda: initializer_thread.join(1) or not initializer_thread.
            is_alive(),
            wait_period=0)
        self._val = initializer_thread.GetReturnValue()
        self._initialized.set()

    return self._val
