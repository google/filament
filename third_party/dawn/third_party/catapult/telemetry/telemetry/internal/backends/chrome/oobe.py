# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging

from telemetry.core import exceptions
from telemetry.internal.browser import web_contents
from telemetry.internal.backends.chrome_inspector.inspector_websocket import \
    WebSocketException

import py_utils


class Oobe(web_contents.WebContents):
  @staticmethod
  def Canonicalize(user, remove_dots=True):
    """Get rid of dots in |user| and add @gmail.com."""
    # Canonicalize.
    user = user.lower()
    if user.find('@') == -1:
      username = user
      domain = 'gmail.com'
    else:
      username, domain = user.split('@')

    # Remove dots for gmail.
    if remove_dots and domain == 'gmail.com':
      username = username.replace('.', '')
    return '%s@%s' % (username, domain)

  def _GaiaWebviewContext(self):
    try:
      webview_contexts = self.GetWebviewContexts()
      for webview in webview_contexts:
        # GAIA webview has base.href accounts.google.com in production, and
        # gaistaging.corp.google.com for QA.
        if webview.EvaluateJavaScript(
            """
            bases = document.getElementsByTagName('base');
            if (bases.length > 0) {
              href = bases[0].href;
              href.indexOf('https://accounts.google.com/') == 0 ||
                  href.indexOf('https://gaiastaging.corp.google.com/') == 0;
            }
            """):
          py_utils.WaitFor(webview.HasReachedQuiescence, 20)
          return webview
    except (exceptions.Error, WebSocketException):
      pass
    return None

  def _ExecuteOobeApi(self, api, *args):
    logging.info('Invoking %s', api)
    self.WaitForJavaScriptCondition(
        "typeof Oobe == 'function' && Oobe.readyForTesting", timeout=120)

    if self.EvaluateJavaScript(
        "typeof {{ @api }} == 'undefined'", api=api):
      raise exceptions.LoginException('%s js api missing' % api)

    # Example values:
    #   |api|:    'doLogin'
    #   |args|:   ['username', 'pass', True]
    #   Executes: 'doLogin("username", "pass", true)'
    self.ExecuteJavaScript('{{ @f }}({{ *args }})', f=api, args=args)

  def EnterpriseWebviewVisible(self, username):
    """We look for a span with the title set to the domain, for example
    <span title="managedchrome.com">."""
    _, domain = username.split('@')
    try:
      webview = self._GaiaWebviewContext()
      return webview and webview.EvaluateJavaScript(
          "document.querySelectorAll('span[title= {{ domain }}]').length;",
          domain=domain)
    except exceptions.DevtoolsTargetCrashException:
      return False

  def NavigateGuestLogin(self):
    """Logs in as guest."""
    self._ExecuteOobeApi('OobeAPI.loginAsGuest')

  def NavigateFakeLogin(self, username, password, gaia_id,
                        enterprise_enroll=False):
    """Fake user login."""
    self._ExecuteOobeApi('Oobe.loginForTesting', username, password, gaia_id,
                         enterprise_enroll)
    if enterprise_enroll:
      # Oobe context recreated after the enrollment (crrev.com/c/2144279).
      py_utils.WaitFor(lambda: not self.IsAlive(), 60)

  def NavigateGaiaLogin(self, username, password,
                        enterprise_enroll=False,
                        for_user_triggered_enrollment=False):
    """Logs in using the GAIA webview. |enterprise_enroll| allows for enterprise
    enrollment. |for_user_triggered_enrollment| should be False for remora
    enrollment."""
    # TODO(achuith): Get rid of this call. crbug.com/804216.
    self._ExecuteOobeApi('OobeAPI.skipToLoginForTesting')
    if for_user_triggered_enrollment:
      self._ExecuteOobeApi('OobeAPI.advanceToScreen', 'enterprise-enrollment')

    url = self.EvaluateJavaScript("window.location.href")
    if url.startswith('chrome://oobe/gaia-signin'):
      self._ExecuteOobeApi('OobeAPI.showGaiaDialog')

    py_utils.WaitFor(self._GaiaWebviewContext, 20)
    self._NavigateWebviewLogin(username, password,
                               wait_for_close=not enterprise_enroll)

  def _UnicornObfuscated(self, text):
    """Converts an email into an obfuscated email.

    So abcdefgh123@gmail.com becomes ****ef****3@gmail.com
    """
    obfuscated = ''
    separator = text.find('@')
    for i in range(separator-1):
      if i in [4, 5]:
        obfuscated += text[i]
      else:
        obfuscated += '*'
    obfuscated += text[separator-1:]
    return obfuscated

  def NavigateUnicornLogin(self, child_user, child_pass,
                           parent_user, parent_pass):
    """Logs into a unicorn account."""

    self._ExecuteOobeApi('OobeAPI.skipToLoginForTesting')
    py_utils.WaitFor(self._GaiaWebviewContext, 20)
    logging.info('Entering child credentials')
    self._NavigateWebviewLogin(child_user, child_pass, False)
    logging.info('Clicking on parent button')
    parent_user = self.Canonicalize(parent_user, remove_dots=False)
    self._ClickGaiaButton(parent_user, self._UnicornObfuscated(parent_user))
    logging.info('Entering parent credentials')
    self._NavigateWebviewEntry('password', parent_pass)
    self._ClickPrimaryActionButton()
    logging.info('Clicking Yes (or I agree)')
    self._ClickGaiaButton('Yes', 'agree')
    py_utils.WaitFor(lambda: not self._GaiaWebviewContext(), 60)
    logging.info('Logged in as unicorn user')

  def _NavigateWebviewLogin(self, username, password, wait_for_close):
    """Logs into the webview-based GAIA screen."""
    self._NavigateWebviewEntry('identifierId', username)
    self._ClickPrimaryActionButton()
    self._NavigateWebviewEntry('password', password)
    self._ClickPrimaryActionButton()
    if wait_for_close:
      py_utils.WaitFor(lambda: not self._GaiaWebviewContext(), 60)

  def _NavigateWebviewEntry(self, field, value):
    """Navigate a username/password GAIA screen."""
    self._WaitForField(field)
    # This code supports both ChromeOS Gaia v1 and v2.
    # In v2 'password' id is assigned to <DIV> element encapsulating
    # unnamed <INPUT>. So this code will select the first <INPUT> element
    # below the given field id in the DOM tree if field id is not attached
    # to <INPUT>.
    self._GaiaWebviewContext().ExecuteJavaScript(
        """
        var field = document.getElementById({{ field }});
        if (field.tagName != 'INPUT')
          field = field.getElementsByTagName('INPUT')[0];

        field.value= {{ value }};""",
        field=field,
        value=value)

  def _WaitForField(self, field):
    """Wait for username/password field to become available."""
    self._GaiaWebviewContext().WaitForJavaScriptCondition(
        "document.getElementById({{ field }}) != null && "
        "!document.getElementById({{ field }}).hidden",
        field=field, timeout=20)

  def _ClickGaiaButton(self, button_text, alt_text):
    """Click the button on the gaia page that matches |button_text|."""
    get_button_js = '''
        (function() {
          var buttons = document.querySelectorAll('[role="button"]');
          if (buttons == null)
            return false;
          for (var i=0; i < buttons.length; ++i) {
            if ((buttons[i].textContent.indexOf('%s') != -1) ||
                (buttons[i].textContent.indexOf('%s') != -1)) {
              buttons[i].click();
              return true;
            }
          }
          return false;
        })();
    ''' % (button_text, alt_text)
    self._GaiaWebviewContext().WaitForJavaScriptCondition(
        get_button_js, timeout=20)

  def _ClickPrimaryActionButton(self):
    """Click the Gaia primary action button on the oobe page"""
    self._ExecuteOobeApi('Oobe.clickGaiaPrimaryButtonForTesting')
