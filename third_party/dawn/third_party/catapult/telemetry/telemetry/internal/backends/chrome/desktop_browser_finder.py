# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Finds desktop browsers that can be started and controlled by telemetry."""

from __future__ import absolute_import
import logging
import os
import shutil
import sys
import tempfile
import six

import dependency_manager  # pylint: disable=import-error

from py_utils import file_util
from telemetry.core import exceptions
from telemetry.core import platform as platform_module
from telemetry.internal.backends.chrome import chrome_startup_args
from telemetry.internal.backends.chrome import desktop_browser_backend
from telemetry.internal.browser import browser
from telemetry.internal.browser import possible_browser
from telemetry.internal.platform import desktop_device
from telemetry.internal.util import binary_manager
from telemetry.internal.util import local_first_binary_manager
# This is a workaround for https://goo.gl/1tGNgd
from telemetry.internal.util import path as path_module

_BROWSER_STARTUP_TRIES = 3


class PossibleDesktopBrowser(possible_browser.PossibleBrowser):
  """A desktop browser that can be controlled."""

  def __init__(self, browser_type, finder_options, executable, flash_path,
               is_content_shell, browser_directory, is_local_build=False):
    """
    Args:
      browser_type: A string representing what type of browser this is, e.g.
          'stable' or 'debug'.
      finder_options: A browser_options.BrowserFinderOptions instance containing
          parsed arguments.
      executable: A string containing a path to the browser executable to use.
      flash_path: A string containing a path to the version of Flash to use. Can
          be None if Flash is not going to be used.
      is_content_shell: A boolean denoting if this browser is a content shell
          instead of a full browser.
      browser_directory: A string containing a path to the directory where
          the browser is installed. This is typically the directory containing
          |executable|, but not guaranteed.
      is_local_build: Whether the browser was built locally (as opposed to
          being downloaded).
    """
    del finder_options
    target_os = sys.platform.lower()
    super().__init__(
        browser_type, target_os, not is_content_shell)
    assert browser_type in FindAllBrowserTypes(), (
        'Please add %s to desktop_browser_finder.FindAllBrowserTypes' %
        browser_type)
    self._local_executable = executable
    self._flash_path = flash_path
    self._is_content_shell = is_content_shell
    self._browser_directory = browser_directory
    self._profile_directory = None
    self._download_directory = None
    self._extra_browser_args = set()
    self.is_local_build = is_local_build
    # If the browser was locally built, then the chosen browser directory
    # should be the same as the build directory. If the browser wasn't
    # locally built, then passing in a non-None build directory is fine
    # since a build directory without any useful debug artifacts is
    # equivalent to no build directory at all.
    self._build_dir = self._browser_directory

  def __repr__(self):
    return 'PossibleDesktopBrowser(type=%s, executable=%s, flash=%s)' % (
        self.browser_type, self._local_executable, self._flash_path)

  @property
  def browser_directory(self):
    return self._browser_directory

  @property
  def profile_directory(self):
    return self._profile_directory

  @property
  def last_modification_time(self):
    if os.path.exists(self._local_executable):
      return os.path.getmtime(self._local_executable)
    return -1

  @property
  def extra_browser_args(self):
    return list(self._extra_browser_args)

  def AddExtraBrowserArg(self, arg):
    self._extra_browser_args.add(arg)

  def _InitPlatformIfNeeded(self):
    if self._platform:
      return

    self._platform = platform_module.GetHostPlatform()

    # pylint: disable=protected-access
    self._platform_backend = self._platform._platform_backend

  def _GetPathsForOsPageCacheFlushing(self):
    return [self.profile_directory, self.browser_directory]

  def SetUpEnvironment(self, browser_options):
    super().SetUpEnvironment(browser_options)
    if self._browser_options.dont_override_profile:
      return

    # If given, this directory's contents will be used to seed the profile.
    source_profile = self._browser_options.profile_dir
    if source_profile and self._is_content_shell:
      raise RuntimeError('Profiles cannot be used with content shell')

    if not self._browser_options.profile_type == 'exact':
      self._profile_directory = tempfile.mkdtemp()
    else:
      self._profile_directory = source_profile

    self._download_directory = tempfile.mkdtemp()
    if not self._browser_options.profile_type == 'exact' and source_profile:
      logging.info('Seeding profile directory from: %s', source_profile)
      # copytree requires the directory to not exist, so just delete the empty
      # directory and re-create it.
      os.rmdir(self._profile_directory)
      shutil.copytree(source_profile, self._profile_directory)

      # When using an existing profile directory, we need to make sure to
      # delete the file containing the active DevTools port number.
      devtools_file_path = os.path.join(
          self._profile_directory,
          desktop_browser_backend.DEVTOOLS_ACTIVE_PORT_FILE)
      if os.path.isfile(devtools_file_path):
        os.remove(devtools_file_path)

    # Copy data into the profile if it hasn't already been added via
    # |source_profile|.
    for source, dest in self._browser_options.profile_files_to_copy:
      full_dest_path = os.path.join(self._profile_directory, dest)
      if not os.path.exists(full_dest_path):
        file_util.CopyFileWithIntermediateDirectories(source, full_dest_path)

  def _TearDownEnvironment(self):
    if self._profile_directory and os.path.exists(self._profile_directory):
      if not self._browser_options.profile_type == 'exact':
        # Remove the profile directory, which was hosted on a temp dir.
        shutil.rmtree(self._profile_directory, ignore_errors=True)
      self._profile_directory = None
    if self._download_directory and os.path.exists(self._download_directory):
      # Remove the download directory, which was hosted on a temp dir.
      shutil.rmtree(self._download_directory, ignore_errors=True)
      self._download_directory = None

  def Create(self):
    # Init the LocalFirstBinaryManager if this is the first time we're creating
    # a browser.
    if local_first_binary_manager.LocalFirstBinaryManager.NeedsInit():
      local_first_binary_manager.LocalFirstBinaryManager.Init(
          self._build_dir, self._local_executable,
          self.platform.GetOSName(), self.platform.GetArchName())

    if self._flash_path and not os.path.exists(self._flash_path):
      logging.warning(
          'Could not find Flash at %s. Continuing without Flash.\n'
          'To run with Flash, check it out via http://go/read-src-internal',
          self._flash_path)
      self._flash_path = None

    self._InitPlatformIfNeeded()

    for x in range(0, _BROWSER_STARTUP_TRIES):
      try:
        # Note: we need to regenerate the browser startup arguments for each
        # browser startup attempt since the state of the startup arguments
        # may not be guaranteed the same each time
        # For example, see: crbug.com/865895#c17
        startup_args = self.GetBrowserStartupArgs(self._browser_options)
        browser_backend = desktop_browser_backend.DesktopBrowserBackend(
            self._platform_backend, self._browser_options,
            self._browser_directory, self._profile_directory,
            self._local_executable, self._flash_path, self._is_content_shell,
            build_dir=self._build_dir)
        new_browser = browser.Browser(
            browser_backend, self._platform_backend, startup_args)
        browser_backend.SetDownloadBehavior(
            'allow', self._download_directory, 30)
        return new_browser
      except Exception: # pylint: disable=broad-except
        retry = x < _BROWSER_STARTUP_TRIES - 1
        retry_message = 'retrying' if retry else 'giving up'
        logging.warning('Browser creation failed (attempt %d of %d), %s.',
                        (x + 1), _BROWSER_STARTUP_TRIES, retry_message)
        if retry:
          # Reset the environment to prevent leftovers in the profile
          # directory from influencing the next try.
          # CleanUpEnvironment sets browser_options to None,
          # so we must save them.
          saved_browser_options = self._browser_options
          self.CleanUpEnvironment()
          self.SetUpEnvironment(saved_browser_options)
        else:
          logging.warning('This might be because of software compositing being'
                          ' disabled. Please try again with'
                          ' --allow-software-compositing flag.')
          raise
    # Should never be hit, but Pylint can't see that we will retry until we
    # eventually re-raise the above exception.
    raise RuntimeError()

  def GetBrowserStartupArgs(self, browser_options):
    startup_args = chrome_startup_args.GetFromBrowserOptions(browser_options)
    startup_args.extend(chrome_startup_args.GetReplayArgs(
        self._platform_backend.network_controller_backend))

    # Setting port=0 allows the browser to choose a suitable port.
    startup_args.append('--remote-debugging-port=0')
    startup_args.append('--enable-crash-reporter-for-testing')
    startup_args.append('--disable-component-update')

    if not self._is_content_shell:
      window_sizes = [arg for arg in browser_options.extra_browser_args
                      if arg.startswith('--window-size=')]
      if len(window_sizes) == 0:
        startup_args.append('--window-size=1280,1024')
      if self._flash_path:
        startup_args.append('--ppapi-flash-path=%s' % self._flash_path)
        # Also specify the version of Flash as a large version, so that it is
        # not overridden by the bundled or component-updated version of Flash.
        startup_args.append('--ppapi-flash-version=99.9.999.999')

    if self.profile_directory is not None:
      startup_args.append('--user-data-dir=%s' % self.profile_directory)

    trace_config_file = (self._platform_backend.tracing_controller_backend
                         .GetChromeTraceConfigFile())
    if trace_config_file:
      startup_args.append('--trace-config-file=%s' % trace_config_file)
    trace_config = (self._platform_backend.tracing_controller_backend.
                    GetChromeTraceConfig())
    if trace_config:
      if (trace_config.chrome_trace_config.trace_format is None
          or trace_config.chrome_trace_config.trace_format != 'proto'):
        startup_args.append('--trace-startup-format=json')

    if sys.platform.startswith('linux'):
      # All linux tests should use the --password-store=basic
      # flag, which stops Chrome from reaching the system's Keyring or
      # KWallet. These are stateful system libraries, which can hurt tests
      # by reducing isolation, reducing speed and introducing flakiness due
      # to their own bugs.
      # See crbug.com/991424.
      startup_args.append('--password-store=basic')

    startup_args.extend(
        [a for a in self.extra_browser_args if a not in startup_args])

    return startup_args

  def SupportsOptions(self, browser_options):
    if ((len(browser_options.extensions_to_load) != 0)
        and self._is_content_shell):
      return False
    return True

  def UpdateExecutableIfNeeded(self):
    pass

  def GetTypExpectationsTags(self):
    '''Override parent function to add release/debug tags

    This function overrides PossibleBrowser's GetTypExpectationTags
    member function. It adds the debug tag if the  debug browser like
    debug_x64 is being used for tests, and the release tag if a release
    browser like release_x64 is being used.
    '''
    tags = super().GetTypExpectationsTags()
    if 'debug' in self.browser_type.lower().split('_'):
      tags.append('debug')
    if 'release' in self.browser_type.lower().split('_'):
      tags.append('release')
    return tags


def SelectDefaultBrowser(possible_browsers):
  local_builds_by_date = [
      b for b in sorted(possible_browsers,
                        key=lambda b: b.last_modification_time)
      if b.is_local_build]
  if local_builds_by_date:
    return local_builds_by_date[-1]
  return None

def CanFindAvailableBrowsers():
  return not platform_module.GetHostPlatform().GetOSName() == 'chromeos'

def FindAllBrowserTypes():
  return [
      'exact',
      'reference',
      'release',
      'release_x64',
      'debug',
      'debug_x64',
      'default',
      'stable',
      'beta',
      'dev',
      'canary',
      'content-shell-debug',
      'content-shell-debug_x64',
      'content-shell-release',
      'content-shell-release_x64',
      'content-shell-default',
      'system']

def FindAllAvailableBrowsers(finder_options, device):
  """Finds all the desktop browsers available on this machine."""
  if not isinstance(device, desktop_device.DesktopDevice):
    return []

  browsers = []

  if not CanFindAvailableBrowsers():
    return []

  has_x11_display = True
  if sys.platform.startswith('linux') and os.getenv('DISPLAY') is None:
    has_x11_display = False

  os_name = platform_module.GetHostPlatform().GetOSName()
  arch_name = platform_module.GetHostPlatform().GetArchName()
  try:
    flash_path = binary_manager.LocalPath('flash', os_name, arch_name)
  except dependency_manager.NoPathFoundError:
    flash_path = None

  chromium_app_names = []
  if sys.platform == 'darwin':
    chromium_app_names.append('Chromium.app/Contents/MacOS/Chromium')
    chromium_app_names.append('Google Chrome.app/Contents/MacOS/Google Chrome')
    chromium_app_names.append('Google Chrome for Testing.app/' +
                              'Contents/MacOS/Google Chrome for Testing')
    content_shell_app_name = 'Content Shell.app/Contents/MacOS/Content Shell'
  elif sys.platform.startswith('linux'):
    chromium_app_names.append('chrome')
    content_shell_app_name = 'content_shell'
  elif sys.platform.startswith('win'):
    chromium_app_names.append('chrome.exe')
    content_shell_app_name = 'content_shell.exe'
  else:
    raise Exception('Platform not recognized')

  # Add the explicit browser executable if given and we can handle it.
  if finder_options.browser_executable:
    is_content_shell = finder_options.browser_executable.endswith(
        content_shell_app_name)

    # It is okay if the executable name doesn't match any of known chrome
    # browser executables, since it may be of a different browser.
    normalized_executable = os.path.expanduser(
        finder_options.browser_executable)
    if path_module.IsExecutable(normalized_executable):
      browser_directory = os.path.dirname(finder_options.browser_executable)
      browsers.append(PossibleDesktopBrowser(
          'exact', finder_options, normalized_executable, flash_path,
          is_content_shell,
          browser_directory))
    else:
      raise exceptions.PathMissingError(
          '%s specified by --browser-executable does not exist or is not '
          'executable' %
          normalized_executable)

  def AddIfFound(browser_type, build_path, app_name, content_shell):
    app = os.path.join(build_path, app_name)
    if path_module.IsExecutable(app):
      browsers.append(PossibleDesktopBrowser(
          browser_type, finder_options, app, flash_path,
          content_shell, build_path, is_local_build=True))
      return True
    return False

  # Add local builds
  if finder_options.chromium_output_dir:
    for chromium_app_name in chromium_app_names:
      AddIfFound(finder_options.browser_type,
                 finder_options.chromium_output_dir, chromium_app_name, False)
  else:
    for build_path in path_module.GetBuildDirectories(
        finder_options.chrome_root):
      # TODO(agrieve): Extract browser_type from args.gn's is_debug.
      browser_type = os.path.basename(build_path.rstrip(os.sep)).lower()
      for chromium_app_name in chromium_app_names:
        AddIfFound(browser_type, build_path, chromium_app_name, False)
      AddIfFound('content-shell-' + browser_type, build_path,
                 content_shell_app_name, True)

  reference_build = None
  if finder_options.browser_type == 'reference':
    # Reference builds are only available in a Chromium checkout. We should not
    # raise an error just because they don't exist.
    os_name = platform_module.GetHostPlatform().GetOSName()
    arch_name = platform_module.GetHostPlatform().GetArchName()
    reference_build = binary_manager.FetchPath(
        'chrome_stable', os_name, arch_name)

  # Mac-specific options.
  if sys.platform == 'darwin':
    mac_canary_root = '/Applications/Google Chrome Canary.app/'
    mac_canary = mac_canary_root + 'Contents/MacOS/Google Chrome Canary'
    mac_system_root = '/Applications/Google Chrome.app'
    mac_system = mac_system_root + '/Contents/MacOS/Google Chrome'
    if path_module.IsExecutable(mac_canary):
      browsers.append(PossibleDesktopBrowser('canary', finder_options,
                                             mac_canary, None, False,
                                             mac_canary_root))

    if path_module.IsExecutable(mac_system):
      browsers.append(PossibleDesktopBrowser('system', finder_options,
                                             mac_system, None, False,
                                             mac_system_root))

    if reference_build and path_module.IsExecutable(reference_build):
      reference_root = os.path.dirname(os.path.dirname(os.path.dirname(
          reference_build)))
      browsers.append(PossibleDesktopBrowser('reference', finder_options,
                                             reference_build, None, False,
                                             reference_root))

  # Linux specific options.
  if sys.platform.startswith('linux'):
    versions = {
        'system': os.path.split(os.path.realpath('/usr/bin/google-chrome'))[0],
        'stable': '/opt/google/chrome',
        'beta': '/opt/google/chrome-beta',
        'dev': '/opt/google/chrome-unstable'
    }

    for version, root in six.iteritems(versions):
      browser_path = os.path.join(root, 'chrome')
      if path_module.IsExecutable(browser_path):
        browsers.append(PossibleDesktopBrowser(version, finder_options,
                                               browser_path, None, False, root))
    if reference_build and path_module.IsExecutable(reference_build):
      reference_root = os.path.dirname(reference_build)
      browsers.append(PossibleDesktopBrowser('reference', finder_options,
                                             reference_build, None, False,
                                             reference_root))

  # Win32-specific options.
  if sys.platform.startswith('win'):
    app_paths = [
        ('system', os.path.join('Google', 'Chrome', 'Application')),
        ('canary', os.path.join('Google', 'Chrome SxS', 'Application')),
    ]
    if reference_build:
      app_paths.append(
          ('reference', os.path.dirname(reference_build)))

    for browser_name, app_path in app_paths:
      for chromium_app_name in chromium_app_names:
        full_path = path_module.FindInstalledWindowsApplication(
            os.path.join(app_path, chromium_app_name))
        if full_path:
          browsers.append(PossibleDesktopBrowser(
              browser_name, finder_options, full_path,
              None, False, os.path.dirname(full_path)))

  has_ozone_platform = False
  for arg in finder_options.browser_options.extra_browser_args:
    if "--ozone-platform" in arg:
      has_ozone_platform = True

  if browsers and not has_x11_display and not has_ozone_platform:
    logging.warning(
        'Found (%s), but you do not have a DISPLAY environment set.', ','.join(
            [b.browser_type for b in browsers]))
    return []

  return browsers
