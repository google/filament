# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry.core import android_action_runner
from telemetry.core import platform
from telemetry.internal.app import android_app
from telemetry.internal.backends import android_app_backend


class AndroidPlatform(platform.Platform):

  def __init__(self, platform_backend):
    super().__init__(platform_backend)
    self._android_action_runner = android_action_runner.AndroidActionRunner(
        platform_backend)

  def Initialize(self):
    self._platform_backend.Initialize()

  @property
  def android_action_runner(self):
    return self._android_action_runner

  @property
  def system_ui(self):
    """Returns an AppUi object to interact with Android's system UI.

    See devil.android.app_ui for the documentation of the API provided.
    """
    return self._platform_backend.GetSystemUi()

  def GetSharedPrefs(self, package, filename, use_encrypted_path=False):
    """Retrieves a Devil SharedPrefs instance from the backend.

    See devil.android.sdk.shared_prefs for the documentation of the returned
    object.

    Args:
      package: A string containing the package of the app that the SharedPrefs
          instance will be for.
      filename: A string containing the specific settings file of the app that
          the SharedPrefs instance will be for.
      use_encrypted_path: Whether to use the newer device-encrypted path
          (/data/user_de/) instead of the older unencrypted path (/data/data/).

    Returns:
      A reference to a SharedPrefs object for the given package and filename
      on whatever device this platform object refers to.
    """
    return self._platform_backend.GetSharedPrefs(
        package, filename, use_encrypted_path=use_encrypted_path)

  def IsLowEnd(self):
    return self._platform_backend.IsLowEnd()

  def IsAosp(self):
    return self._platform_backend.IsAosp()

  def LaunchAndroidApplication(self,
                               start_intent,
                               is_app_ready_predicate=None,
                               app_has_webviews=False):
    """Launches an Android application given the intent.

    Args:
      start_intent: The intent to use to start the app.
      is_app_ready_predicate: A predicate function to determine
          whether the app is ready. This is a function that takes an
          AndroidApp instance and return a boolean. When it is not passed in,
          the app is ready when the intent to launch it is completed.
      app_has_webviews: A boolean indicating whether the app is expected to
          contain any WebViews. If True, the app will be launched with
          appropriate webview flags, and the GetWebViews method of the returned
          object may be used to access them.

    Returns:
      A reference to the android_app launched.
    """
    self._platform_backend.DismissCrashDialogIfNeeded()
    app_backend = android_app_backend.AndroidAppBackend(
        self._platform_backend, start_intent, is_app_ready_predicate,
        app_has_webviews)
    return android_app.AndroidApp(app_backend, self._platform_backend)

  def StartAndroidService(self, start_intent):
    """Starts an Android service specified by |start_intent|.

    Args:
      start_intent: The intent to use to start the service
    """
    self._platform_backend.device.StartService(start_intent)

  def RemoveSystemPackages(self, packages):
    """Removes the given packages if installed as system apps.

    Args:
      packages: A list of package names to remove.
    """
    self._platform_backend.RemoveSystemPackages(packages)
