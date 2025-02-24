# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import logging
import os
import threading
import time

logger = logging.getLogger(__name__)


class Denylist(object):
  def __init__(self, path):
    self._denylist_lock = threading.RLock()
    self._path = path

  def Read(self):
    """Reads the denylist from the denylist file.

    Returns:
      A dict containing bad devices.
    """
    with self._denylist_lock:
      denylist = dict()
      if not os.path.exists(self._path):
        return denylist

      try:
        with open(self._path, 'r') as f:
          denylist = json.load(f)
      except (IOError, ValueError) as e:
        logger.warning('Unable to read denylist: %s', str(e))
        os.remove(self._path)

      if not isinstance(denylist, dict):
        logger.warning('Ignoring %s: %s (a dict was expected instead)',
                       self._path, denylist)
        denylist = dict()

      return denylist

  def Write(self, denylist):
    """Writes the provided denylist to the denylist file.

    Args:
      denylist: list of bad devices to write to the denylist file.
    """
    with self._denylist_lock:
      with open(self._path, 'w') as f:
        json.dump(denylist, f)

  def Extend(self, devices, reason='unknown'):
    """Adds devices to denylist file.

    Args:
      devices: list of bad devices to be added to the denylist file.
      reason: string specifying the reason for denylist (eg: 'unauthorized')
    """
    timestamp = time.time()
    event_info = {
        'timestamp': timestamp,
        'reason': reason,
    }
    device_dicts = {device: event_info for device in devices}
    logger.info('Adding %s to denylist %s for reason: %s', ','.join(devices),
                self._path, reason)
    with self._denylist_lock:
      denylist = self.Read()
      denylist.update(device_dicts)
      self.Write(denylist)

  def Reset(self):
    """Erases the denylist file if it exists."""
    logger.info('Resetting denylist %s', self._path)
    with self._denylist_lock:
      if os.path.exists(self._path):
        os.remove(self._path)
