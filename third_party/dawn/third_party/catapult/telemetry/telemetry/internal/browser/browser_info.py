# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

_CHECK_WEBGL_SUPPORTED_SCRIPT = """
(function () {
  var c = document.createElement('canvas');
  var gl = c.getContext('webgl', { failIfMajorPerformanceCaveat: true });
  if (gl == null) {
    gl = c.getContext('experimental-webgl',
        { failIfMajorPerformanceCaveat: true });
    if (gl == null) {
      return false;
    }
  }
  return true;
})();
"""

class BrowserInfo():
  """A wrapper around browser object that allows looking up infos of the
  browser.
  """
  def __init__(self, browser):
    self._browser = browser

  def HasWebGLSupport(self):
    result = False
    # If no tab is opened, open one and close it after evaluate
    # _CHECK_WEBGL_SUPPORTED_SCRIPT
    if len(self._browser.tabs) == 0 and self._browser.supports_tab_control:
      self._browser.tabs.New()
      tab = self._browser.tabs[0]
      result = tab.EvaluateJavaScript(_CHECK_WEBGL_SUPPORTED_SCRIPT)
      tab.Close()
    elif len(self._browser.tabs) > 0:
      tab = self._browser.tabs[0]
      result = tab.EvaluateJavaScript(_CHECK_WEBGL_SUPPORTED_SCRIPT)
    return result

  @property
  def browser_type(self):
    return self._browser.browser_type

  @property
  def browser(self):
    return self._browser
