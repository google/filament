# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry.internal import app


class AndroidApp(app.App):
  """A running android app instance that can be controlled in a limited way.

  Be sure to clean up after yourself by calling Close() when you are done with
  the app. Or better yet:
    with possible_android_app.Create(options) as android_app:
      ... do all your operations on android_app here
  """
  def __init__(self, app_backend, platform_backend):
    super().__init__(app_backend=app_backend,
                                     platform_backend=platform_backend)
    self._app_backend.Start()

  @property
  def ui(self): # pylint: disable=invalid-name
    """Returns an AppUi object to interact with the app's UI.

    See devil.android.app_ui for the documentation of the API provided.
    """
    return self._app_backend.GetAppUi()

  def Close(self):
    self._app_backend.Close()
