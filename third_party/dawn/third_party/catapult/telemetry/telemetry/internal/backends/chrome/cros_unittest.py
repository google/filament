# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import os
import tempfile

from telemetry.core import exceptions
from telemetry import decorators
from telemetry.internal.backends.chrome import cros_test_case
from telemetry.internal.backends.chrome import oobe

import py_utils


class CrOSCryptohomeTest(cros_test_case.CrOSTestCase):
  @decorators.Enabled('chromeos')
  def testCryptohome(self):
    """Verifies cryptohome mount status for regular and guest user and when
    logged out"""
    with self._CreateBrowser() as b:
      self.assertLess(0, len(b.tabs))
      self.assertGreater(3, len(b.tabs))
      self.assertTrue(b.tabs[0].url)
      self.assertTrue(self._IsCryptohomeMounted())
      self.assertTrue(
          self._cri.IsCryptohomeMounted(self._username, self._is_guest))

      ephemeral_fs = self._cri.FilesystemMountedAt(
          self._cri.EphemeralCryptohomePath(self._username))

      # TODO(achuith): Remove dependency on /home/chronos/user.
      chronos_fs = self._cri.FilesystemMountedAt('/home/chronos/user')
      self.assertTrue(chronos_fs or ephemeral_fs)
      if self._is_guest:
        self.assertTrue(
            (ephemeral_fs and ephemeral_fs.startswith('/dev/loop')) or
            (chronos_fs == 'guestfs'))
      elif chronos_fs:
        crypto_fs = self._cri.FilesystemMountedAt(
            self._cri.CryptohomePath(self._username))
        self.assertEqual(crypto_fs, chronos_fs)

    self.assertFalse(self._IsCryptohomeMounted())
    self.assertFalse(
        self._cri.IsCryptohomeMounted(self._username, self._is_guest))
    self.assertEqual(self._cri.FilesystemMountedAt('/home/chronos/user'),
                     '/dev/mapper/encstateful')


class CrOSLoginTest(cros_test_case.CrOSTestCase):
  def _GetCredentialsIter(self, credentials_file=None):
    """Read username and password from credentials.txt. Each line is of the
    format username:password. For unicorn accounts, the file may contain
    multiple lines of credentials."""
    if not credentials_file:
      credentials_file = os.path.join(os.path.dirname(__file__),
                                      'credentials.txt')
    if os.path.exists(credentials_file):
      with open(credentials_file) as f:
        for credentials in f:
          username, password = credentials.strip().split(':')
          yield username, password

  @decorators.Enabled('chromeos')
  def testCanonicalize(self):
    """Test Oobe.Canonicalize function."""
    self.assertEqual(oobe.Oobe.Canonicalize('User.1'), 'user1@gmail.com')
    self.assertEqual(oobe.Oobe.Canonicalize('User.2', remove_dots=False),
                     'user.2@gmail.com')
    self.assertEqual(oobe.Oobe.Canonicalize('User.3@chromium.org'),
                     'user.3@chromium.org')

  @decorators.Enabled('chromeos')
  def testGetCredentials(self):
    fd, cred_file = tempfile.mkstemp()
    try:
      os.write(fd, 'user1:pass1\nuser2:pass2\nuser3:pass3'.encode())
      os.close(fd)
      cred_iter = self._GetCredentialsIter(cred_file)
      for i in range(1, 4):
        username, password = next(cred_iter)
        self.assertEqual(username, 'user%d' % i)
        self.assertEqual(password, 'pass%d' % i)
    finally:
      os.unlink(cred_file)

  @decorators.Enabled('chromeos')
  def testLoginStatus(self):
    """Tests autotestPrivate.loginStatus"""
    if self._is_guest:
      return
    with self._CreateBrowser(autotest_ext=True) as b:
      login_status = self._GetLoginStatus(b)
      self.assertEqual(type(login_status), dict)

      self.assertEqual(not self._is_guest, login_status['isRegularUser'])
      self.assertEqual(self._is_guest, login_status['isGuest'])
      self.assertEqual(login_status['email'], self._username)
      self.assertFalse(login_status['isScreenLocked'])

  @decorators.Enabled('chromeos')
  def testLogout(self):
    """Tests autotestPrivate.logout"""
    if self._is_guest:
      return
    with self._CreateBrowser(autotest_ext=True) as b:
      extension = self._GetAutotestExtension(b)
      try:
        extension.ExecuteJavaScript('chrome.autotestPrivate.logout();')
      except exceptions.Error:
        pass
      py_utils.WaitFor(lambda: not self._IsCryptohomeMounted(), 20)

  @decorators.Disabled('all')
  def testGaiaLogin(self):
    """Tests gaia login. Use credentials in credentials.txt if it exists,
    otherwise use autotest.catapult."""
    if self._is_guest:
      return
    try:
      username, password = next(self._GetCredentialsIter())
    except StopIteration:
      username = 'autotest.catapult'
      password = 'autotest'
    with self._CreateBrowser(gaia_login=True,
                             username=oobe.Oobe.Canonicalize(username),
                             password=password):
      self.assertTrue(py_utils.WaitFor(self._IsCryptohomeMounted, 10))

  @decorators.Enabled('chromeos')
  def testUnicornLogin(self):
    """Tests unicorn account login."""
    if self._is_guest:
      return

    try:
      creds_iter = self._GetCredentialsIter()
      child_user, child_pass = next(creds_iter)
      parent_user, parent_pass = next(creds_iter)
    except StopIteration:
      return
    with self._CreateBrowser(auto_login=False) as browser:
      browser.oobe.NavigateUnicornLogin(child_user, child_pass,
                                        parent_user, parent_pass)
      self.assertTrue(py_utils.WaitFor(self._IsCryptohomeMounted, 10))


class CrOSScreenLockerTest(cros_test_case.CrOSTestCase):
  def _IsScreenLocked(self, browser):
    return self._GetLoginStatus(browser)['isScreenLocked']

  def _LockScreen(self, browser):
    self.assertFalse(self._IsScreenLocked(browser))

    extension = self._GetAutotestExtension(browser)
    self.assertTrue(extension.EvaluateJavaScript(
        "typeof chrome.autotestPrivate.lockScreen == 'function'"))
    logging.info('Locking screen')
    extension.ExecuteJavaScript('chrome.autotestPrivate.lockScreen();')

    logging.info('Waiting for the lock screen')
    def ScreenLocked():
      return (browser.oobe_exists and
              browser.oobe.EvaluateJavaScript("typeof Oobe == 'function'") and
              browser.oobe.EvaluateJavaScript(
                  "typeof Oobe.authenticateForTesting == 'function'"))
    py_utils.WaitFor(ScreenLocked, 10)
    self.assertTrue(self._IsScreenLocked(browser))

  def _AttemptUnlockBadPassword(self, browser):
    logging.info('Trying a bad password')
    def ErrorBubbleVisible():
      return not browser.oobe.EvaluateJavaScript(
          "document.getElementById('bubble').hidden")

    self.assertFalse(ErrorBubbleVisible())
    browser.oobe.ExecuteJavaScript(
        "Oobe.authenticateForTesting({{ username }}, 'bad');",
        username=self._username)
    py_utils.WaitFor(ErrorBubbleVisible, 10)
    self.assertTrue(self._IsScreenLocked(browser))

  def _UnlockScreen(self, browser):
    logging.info('Unlocking')
    browser.oobe.ExecuteJavaScript(
        'Oobe.authenticateForTesting({{ username }}, {{ password }});',
        username=self._username, password=self._password)
    py_utils.WaitFor(lambda: not browser.oobe_exists, 10)
    self.assertFalse(self._IsScreenLocked(browser))

  @decorators.Disabled('all')
  def testScreenLock(self):
    """Tests autotestPrivate.screenLock"""
    if self._is_guest:
      return
    with self._CreateBrowser(autotest_ext=True) as browser:
      self._LockScreen(browser)
      self._AttemptUnlockBadPassword(browser)
      self._UnlockScreen(browser)
