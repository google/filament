# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry.internal.backends import app_backend

from devil.android import app_ui
from devil.android.sdk import intent

import py_utils


class AndroidAppBackend(app_backend.AppBackend):

  def __init__(self, android_platform_backend, start_intent,
               is_app_ready_predicate=None, app_has_webviews=False):
    super().__init__(
        start_intent.package, android_platform_backend)
    self._start_intent = start_intent
    self._is_app_ready_predicate = is_app_ready_predicate
    self._is_running = False
    self._app_ui = None
    assert not app_has_webviews

  @property
  def device(self):
    return self.platform_backend.device

  def GetAppUi(self):
    if self._app_ui is None:
      self._app_ui = app_ui.AppUi(self.device, self._start_intent.package)
    return self._app_ui

  def Start(self):
    """Start an Android app and wait for it to finish launching."""
    def IsAppReady():
      return self._is_app_ready_predicate(self.app)

    # When "is_app_ready_predicate" is provided, we use it to wait for the
    # app to become ready, otherwise "blocking=True" is used as a fall back.
    # TODO(slamm): check if the wait for "ps" check is really needed, or
    # whether the "blocking=True" fall back is sufficient.
    has_ready_predicate = self._is_app_ready_predicate is not None
    self.device.StartActivity(
        self._start_intent,
        blocking=not has_ready_predicate,
        force_stop=True,  # Ensure app was not running.
    )
    if has_ready_predicate:
      py_utils.WaitFor(IsAppReady, timeout=60)

    self._is_running = True

  def Foreground(self):
    self.device.StartActivity(
        intent.Intent(package=self._start_intent.package,
                      activity=self._start_intent.activity,
                      action=None,
                      flags=[intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED]),
        blocking=True)

  def Background(self):
    package = 'org.chromium.push_apps_to_background'
    activity = package + '.PushAppsToBackgroundActivity'
    self.device.StartActivity(
        intent.Intent(
            package=package,
            activity=activity,
            action=None,
            flags=[intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED]),
        blocking=True)

  def Close(self):
    self._is_running = False
    self.platform_backend.KillApplication(self._start_intent.package)

  def IsAppRunning(self):
    return self._is_running
