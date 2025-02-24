# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry.internal.backends.chrome import cros_browser_backend
from telemetry.internal.browser import browser


class CrOSBrowserWithOOBE(browser.Browser):
  """Cros-specific browser."""
  def __init__(self, backend, platform_backend, startup_args):
    assert isinstance(backend, cros_browser_backend.CrOSBrowserBackend)
    super().__init__(
        backend, platform_backend, startup_args)

  @property
  def oobe(self):
    """The login webui (also serves as ui for screenlock and
    out-of-box-experience).
    """
    return self._browser_backend.oobe

  @property
  def oobe_exists(self):
    """True if the login/oobe/screenlock webui exists. This is more lightweight
    than accessing the oobe property.
    """
    return self._browser_backend.oobe_exists

  def BindDevToolsClient(self):
    """If a restart is triggered, wait for the browser to come up, and reconnect
    to devtools.
    """
    self._browser_backend.BindDevToolsClient()

  def WaitForBrowserToComeUp(self):
    """DEPRECATED: Use BindDevToolsClient instead."""
    self.BindDevToolsClient()
