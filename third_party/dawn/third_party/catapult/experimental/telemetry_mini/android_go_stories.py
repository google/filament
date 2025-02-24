#!/usr/bin/env python
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import argparse
import fnmatch
import logging
import os
import sys

import telemetry_mini
import six


BROWSER_FLAGS = [
    '--enable-remote-debugging',
    '--disable-fre',
    '--no-default-browser-check',
    '--no-first-run',
]

TRACE_CONFIG = {
    'excludedCategories': ['*'],
    'includedCategories': ['rails', 'toplevel', 'startup', 'blink.user_timing'],
    'memoryDumpConfig': {'triggers': []}
}

BROWSERS = {
    'android-chrome': telemetry_mini.ChromeApp,
    'android-chromium': telemetry_mini.ChromiumApp,
    'android-system-chrome': telemetry_mini.SystemChromeApp,
}


class TwitterApp(telemetry_mini.AndroidApp):
  PACKAGE_NAME = 'com.twitter.android'


class InstagramApp(telemetry_mini.AndroidApp):
  PACKAGE_NAME = 'com.instagram.android'


class HangoutsApp(telemetry_mini.AndroidApp):
  PACKAGE_NAME = 'com.google.android.talk'


class TwitterFlipkartStory(telemetry_mini.UserStory):
  """Load Chrome Custom Tab from another application.

  The flow of the story is:
  - Start Twitter app to view the @flipkart profile.
  - Tap on a link to open Flipkart in a Chrome Custom Tab.
  - Return to Twitter app.
  """
  NAME = 'twitter_flipkart'
  FLIPKART_TWITTER_LINK = [
      ('package', 'com.twitter.android'),
      ('class', 'android.widget.TextView'),
      ('text', 'flipkart.com')
  ]

  def __init__(self, *args, **kwargs):
    super(TwitterFlipkartStory, self).__init__(*args, **kwargs)
    self.watcher = ProcessWatcher(self.device)
    self.twitter = TwitterApp(self.device)

  def RunPrepareSteps(self):
    self.twitter.ForceStop()

  def RunStorySteps(self):
    # Activity will launch Twitter app on Flipkart profile.
    self.actions.StartActivity('https://twitter.com/flipkart')
    self.watcher.StartWatching(self.twitter)

    # Tapping on Flikpart link on Twitter app will launch Chrome.
    self.actions.TapUiElement(self.FLIPKART_TWITTER_LINK)
    self.watcher.StartWatching(self.browser)
    self.browser.WaitForCurrentPageReady()
    self.actions.SwipeUp(repeat=3)

    # Return to Twitter app.
    self.actions.GoBack()
    self.watcher.AssertAllAlive()

  def RunCleanupSteps(self):
    self.twitter.ForceStop()


class FlipkartInstagramStory(telemetry_mini.UserStory):
  """Interaction between Chrome, PWAs and a WebView-based app.

  The flow of the story is:
  - Launch the Flipkart PWA.
  - Go back home and launch the Instagram app.
  - Use the app switcher to return to Flipkart.
  - Go back home and launch Cricbuzz from a shortcut.
  """
  NAME = 'flipkart_instagram'

  def __init__(self, *args, **kwargs):
    super(FlipkartInstagramStory, self).__init__(*args, **kwargs)
    self.watcher = ProcessWatcher(self.device)
    self.instagram = InstagramApp(self.device)

  def RunPrepareSteps(self):
    self.instagram.ForceStop()
    self.actions.ClearRecentApps()

  def RunStorySteps(self):
    # Tap on home screen shortcut to open Flipkart PWA.
    self.actions.TapHomeScreenShortcut('Flipkart Lite')
    self.watcher.StartWatching(self.browser)
    self.browser.WaitForCurrentPageReady()
    self.actions.SwipeUp(repeat=2)

    # Go back home, then launch Instagram app.
    self.actions.GoHome()
    self.actions.TapHomeScreenShortcut('Instagram')
    self.watcher.StartWatching(self.instagram)
    self.actions.SwipeUp(repeat=5)

    # Go to app switcher and return to Flipkart PWA.
    self.actions.GoAppSwitcher()
    self.actions.TapAppSwitcherTitle('Flipkart Lite')
    self.actions.SwipeDown()

    # Go back home, then open Cricbuzz shortcut.
    self.actions.GoHome()
    self.actions.TapHomeScreenShortcut('Cricbuzz')
    self.browser.WaitForCurrentPageReady()
    self.actions.SwipeUp()
    self.watcher.AssertAllAlive()

  def RunCleanupSteps(self):
    self.instagram.ForceStop()


class HangoutsIndiaTimesStory(telemetry_mini.UserStory):
  """Interaction between Chrome and a non-WebView-based app.

  TODO: Not sure if Hangouts is a non-WebView app. Consider using another app
  if needed.

  The flow of the story is:
  - Launch the Hangouts app.
  - Open a conversation with a link to an IndiaTimes article.
  - Click on the link to launch Chrome.
  - Go back to the conversation.
  """
  NAME = 'hangouts_indiatimes'
  # TODO: Maybe use more specific targets, e.g. check url is in message.
  FIRST_CONVERSATION = [
      ('resource-id', 'com.google.android.talk:id/conversationContent'),
      ('index', '0'),
  ]
  SECOND_MESSAGE = [
      ('resource-id', 'com.google.android.talk:id/message_root'),
      ('index', '1'),
  ]

  def __init__(self, *args, **kwargs):
    super(HangoutsIndiaTimesStory, self).__init__(*args, **kwargs)
    self.watcher = ProcessWatcher(self.device)
    self.hangouts = HangoutsApp(self.device)

  def RunPrepareSteps(self):
    self.hangouts.ForceStop()

  def RunStorySteps(self):
    # Tap on home screen shortcut to open Hangouts app.
    self.actions.TapHomeScreenShortcut('Hangouts')
    self.watcher.StartWatching(self.hangouts)

    # Find conversation with link to IndiaTimes and tap to launch Chrome.
    self.actions.TapUiElement(self.FIRST_CONVERSATION)
    self.actions.TapUiElement(self.SECOND_MESSAGE)
    self.watcher.StartWatching(self.browser)
    self.browser.WaitForCurrentPageReady()
    self.actions.SwipeUp(repeat=4)

    # Go back to Hangouts, then back Home.
    self.actions.GoBack()
    self.actions.Idle(2)
    self.actions.GoHome()
    self.watcher.AssertAllAlive()

  def RunCleanupSteps(self):
    self.hangouts.ForceStop()


STORIES = (
    TwitterFlipkartStory,
    FlipkartInstagramStory,
    HangoutsIndiaTimesStory,
)


class ProcessWatcher(object):
  def __init__(self, device):
    self.device = device
    self._process_pid = {}

  def StartWatching(self, process_name):
    """Register a process or android app to keep track of its PID."""
    if isinstance(process_name, telemetry_mini.AndroidApp):
      process_name = process_name.PACKAGE_NAME

    @telemetry_mini.RetryOn(returns_falsy=True)
    def GetPids():
      # Returns an empty list if the process name is not found.
      return self.device.ProcessStatus()[process_name]

    assert process_name not in self._process_pid
    pids = GetPids()
    assert pids, 'PID for %s not found' % process_name
    assert len(pids) == 1, 'Single PID for %s expected, but found: %s' % (
        process_name, pids)
    logging.info('Started watching %s (PID=%d)', process_name, pids[0])
    self._process_pid[process_name] = pids[0]

  def AssertAllAlive(self):
    """Check that all watched processes remain alive and were not restarted."""
    status = self.device.ProcessStatus()
    all_alive = True
    for process_name, old_pid in sorted(six.iteritems(self._process_pid)):
      new_pids = status[process_name]
      if not new_pids:
        all_alive = False
        logging.error('Process %s died (PID=%d).', process_name, old_pid)
      elif new_pids != [old_pid]:
        all_alive = False
        logging.error(
            'Process %s restarted (PID=%d -> %s).', process_name,
            old_pid, new_pids)
      else:
        logging.info('Process %s still alive (PID=%d)', process_name, old_pid)
    assert all_alive, 'Some watched processes died or got restarted'


def EnsureSingleBrowser(device, browser_name, force_install=False):
  """Ensure a single Chrome browser is installed and available on the device.

  Having more than one Chrome browser available may produce results which are
  confusing or unreliable (e.g. unclear which browser will respond by default
  to intents triggered by other apps).

  This function ensures only the selected browser is available, installing it
  if necessary, and uninstalling/disabling others.
  """
  browser = BROWSERS[browser_name](device)
  available_browsers = set(device.ListPackages('chrome', only_enabled=True))

  # Install or enable if needed.
  if force_install or browser.PACKAGE_NAME not in available_browsers:
    browser.Install()

  # Uninstall disable other browser apps.
  for other_browser in six.itervalues(BROWSERS):
    if (other_browser.PACKAGE_NAME != browser.PACKAGE_NAME and
        other_browser.PACKAGE_NAME in available_browsers):
      other_browser(device).Uninstall()

  # Finally check that only the selected browser is actually available.
  available_browsers = device.ListPackages('chrome', only_enabled=True)
  assert browser.PACKAGE_NAME in available_browsers, (
      'Unable to make %s available' % browser.PACKAGE_NAME)
  available_browsers.remove(browser.PACKAGE_NAME)
  assert not available_browsers, (
      'Other browsers may intefere with the test: %s' % available_browsers)
  return browser


def main():
  browser_names = sorted(BROWSERS)
  default_browser = 'android-chrome'
  parser = argparse.ArgumentParser()
  parser.add_argument('--serial',
                      help='device serial on which to run user stories'
                      ' (defaults to first device found)')
  parser.add_argument('--adb-bin', default='adb', metavar='PATH',
                      help='path to adb binary to use (default: %(default)s)')
  parser.add_argument('--browser', default=default_browser, metavar='NAME',
                      choices=browser_names,
                      help='one of: %s' % ', '.join(
                          '%s (default)' % b if b == default_browser else b
                          for b in browser_names))
  parser.add_argument('--story-filter', metavar='PATTERN', default='*',
                      help='run the matching stories only (allows Unix'
                      ' shell-style wildcards)')
  parser.add_argument('--repeat', metavar='NUM', type=int, default=1,
                      help='repeat the story set a number of times'
                      ' (default: %(default)d)')
  parser.add_argument('--output-dir', metavar='PATH',
                      help='path to directory for placing output trace files'
                      ' (defaults to current directory)')
  parser.add_argument('--force-install', action='store_true',
                      help='install APK even if browser is already available')
  parser.add_argument('--apks-dir', metavar='PATH',
                      help='path where to find APKs to install')
  parser.add_argument('--port', type=int, default=1234,
                      help='port for connection with device'
                      ' (default: %(default)s)')
  parser.add_argument('-v', '--verbose', action='store_true')
  args = parser.parse_args()

  logging.basicConfig()
  if args.verbose:
    logging.getLogger().setLevel(logging.INFO)

  stories = [s for s in STORIES if fnmatch.fnmatch(s.NAME, args.story_filter)]
  if not stories:
    logging.error('No matching stories')
    return 1

  if args.output_dir is None:
    args.output_dir = os.getcwd()
  else:
    args.output_dir = os.path.realpath(args.output_dir)
  if not os.path.isdir(args.output_dir):
    logging.error('Output directory does not exit')
    return 2

  if args.apks_dir is None:
    args.apks_dir = os.path.realpath(os.path.join(
        os.path.dirname(__file__), '..', '..', '..', '..',
        'out', 'Release', 'apks'))
  telemetry_mini.AndroidApp.APKS_DIR = args.apks_dir

  telemetry_mini.AdbMini.ADB_BIN = args.adb_bin
  if args.serial is None:
    device = next(telemetry_mini.AdbMini.GetDevices())
    logging.warning(
        'Connected to first device found: --serial %s', device.serial)
  else:
    device = telemetry_mini.AdbMini(args.serial)

  # Some operations may require a rooted device.
  device.RunCommand('root')
  device.RunCommand('wait-for-device')

  browser = EnsureSingleBrowser(device, args.browser, args.force_install)
  browser.SetBrowserFlags(BROWSER_FLAGS)
  browser.SetTraceConfig(TRACE_CONFIG)
  browser.SetDevToolsLocalPort(args.port)
  telemetry_mini.RunStories(browser, stories, args.repeat, args.output_dir)
  return 0


if __name__ == '__main__':
  sys.exit(main())
