# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import absolute_import
import argparse
import copy
import logging
import os
import shlex
import socket
import sys

from py_utils import cloud_storage  # pylint: disable=import-error
from py_utils import atexit_with_log

from telemetry import compat_mode_options
from telemetry.core import cast_interface
from telemetry.core import optparse_argparse_migration as oam
from telemetry.core import platform
from telemetry.core import util
from telemetry.internal.browser import browser_finder
from telemetry.internal.browser import browser_finder_exceptions
from telemetry.internal.browser import profile_types
from telemetry.internal.platform import android_device
from telemetry.internal.platform import device_finder
from telemetry.internal.platform import remote_platform_options
from telemetry.internal.util import binary_manager
from telemetry.util import wpr_modes


def _IsWin():
  return sys.platform == 'win32'


class BrowserFinderOptions(argparse.Namespace):
  """Options to be used for discovering a browser."""

  emulator_environment = None

  def __init__(self, browser_type=None):
    super().__init__()

    self.browser_type = browser_type
    self.browser_executable = None
    # The set of possible platforms the browser should run on.
    self.target_platforms = None
    self.os_name = None
    self.chrome_root = None  # Path to src/
    self.chromium_output_dir = None  # E.g.: out/Debug
    self.device = None
    self.ssh_identity = None

    self.remote = None
    self.remote_ssh_port = None

    self.verbosity = 0
    self.quiet = 0

    self.browser_options = BrowserOptions()
    self.output_file = None

    self.remote_platform_options = None

    self.performance_mode = None

    # TODO(crbug.com/798703): remove this
    self.no_performance_mode = False

    self.interval_profiling_target = ''
    self.interval_profiling_periods = []
    self.interval_profiling_frequency = 1000
    self.interval_profiler_options = ''

    self.experimental_system_tracing = False
    self.experimental_system_data_sources = False
    self.force_sideload_perfetto = False

  def __repr__(self):
    return str(sorted(self.__dict__.items()))

  def Copy(self):
    return copy.deepcopy(self)

  def CreateParser(self, *args, **kwargs):
    parser = oam.CreateFromOptparseInputs(*args, **kwargs)

    # Options to interact with a potential external results processor.
    parser.set_defaults(
        external_results_processor=False,
        intermediate_dir=None,
        # TODO(crbug.com/928275): Remove these when Telemetry is no longer
        # involved in any results processing.
        output_dir=None,
        output_formats=[],
        legacy_output_formats=[],
        reset_results=True,
        results_label='telemetry_run',
        upload_results=False,
        upload_bucket=None)

    # Selection group
    group = parser.add_argument_group('Which browser to use')
    group.add_argument('--browser',
                       dest='browser_type',
                       choices=['list', 'any'] +
                       browser_finder.FindAllBrowserTypes(),
                       help='Browser type to run, in order of priority.')
    group.add_argument('--cast-receiver',
                       dest='cast_receiver_type',
                       choices=['list'] + cast_interface.CAST_BROWSERS,
                       help='Cast Receiver type to run')
    group.add_argument('--browser-executable', help='The exact browser to run.')
    group.add_argument(
        '--chrome-root',
        help=('Where to look for chrome builds. Defaults to searching parent '
              'dirs by default.'))
    group.add_argument(
        '--chromium-output-directory',
        dest='chromium_output_dir',
        help=('Where to look for build artifacts. Can also be specified by '
              'setting environment variable CHROMIUM_OUTPUT_DIR.'))
    group.add_argument('--remote',
                       help='The hostname of a remote ChromeOS device to use.')
    group.add_argument(
        '--remote-ssh-port',
        type=int,
        # This is set in ParseArgs if necessary.
        default=-1,
        help=('The SSH port of the remote ChromeOS device (requires --remote '
              'or --fetch-cros-remote).'))
    group.add_argument(
        '--fetch-cros-remote',
        action='store_true',
        help=('Will extract device hostname from the SWARMING_BOT_ID env var '
              'if running on ChromeOS Swarming.'))
    compat_mode_options_list = [
        compat_mode_options.NO_FIELD_TRIALS,
        compat_mode_options.IGNORE_CERTIFICATE_ERROR,
        compat_mode_options.LEGACY_COMMAND_LINE_PATH,
        compat_mode_options.GPU_BENCHMARKING_FALLBACKS,
        compat_mode_options.DONT_REQUIRE_ROOTED_DEVICE
    ]
    parser.add_argument(
        '--compatibility-mode',
        action='append',
        choices=compat_mode_options_list,
        default=[],
        help=('Select the compatibility change that you want to enforce when '
              'running benchmarks.'))
    parser.add_argument(
        '--legacy-json-trace-format',
        action='store_true',
        help='Request traces from Chrome in legacy JSON format.')
    parser.add_argument(
        '--experimental-system-tracing',
        action='store_true',
        help='Use system tracing from Perfetto to trace Chrome.')
    parser.add_argument(
        '--experimental-system-data-sources',
        action='store_true',
        help='Use Perfetto tracing to collect power and CPU usage data.')
    parser.add_argument(
        '--force-sideload-perfetto',
        action='store_true',
        help=('Sideload perfetto binaries from the cloud even if the device '
              'already has Perfetto installed.'))
    identity = None
    testing_rsa = os.path.join(
        util.GetTelemetryThirdPartyDir(), 'chromite', 'ssh_keys', 'testing_rsa')
    if os.path.exists(testing_rsa):
      identity = testing_rsa
    group.add_argument(
        '--identity',
        dest='ssh_identity',
        default=identity,
        help='The identity file to use when ssh\'ing into the ChromeOS device')

    # Debugging options
    group = parser.add_argument_group('When things go wrong')
    group.add_argument('--print-bootstrap-deps',
                       action='store_true',
                       help='Output bootstrap deps list.')
    group.add_argument(
        '--extra-chrome-categories',
        help=('Filter string to enable additional chrome tracing categories. '
              'See documentation here: '
              'https://cs.chromium.org/chromium/src/base/trace_event/'
              'trace_config.h?rcl='
              'c8db6c6371ca047c24d41f3972d5819bc83d83ae&l=125'))
    group.add_argument(
        '--extra-atrace-categories',
        help=('Comma-separated list of extra atrace categories. Use atrace'
              ' --list_categories to get full list.'))
    group.add_argument(
        '--enable-systrace',
        action='store_true',
        help=('Enable collection of systrace. (Useful on ChromeOS where atrace '
              'is not supported; collects scheduling information.)'))
    group.add_argument(
        '--capture-screen-video',
        action='store_true',
        help=('Capture the screen during the test and save it to a video file '
              '(note that it is supported only on some platforms)'))
    group.add_argument(
        '--periodic-screenshot-frequency-ms',
        type=int,
        help=('During each story, capture a screenshot every x ms and save it '
              'to a file (Linux/Windows/[La]CrOS only). NOTE: This '
              'significantly impacts performance, so it should only be used '
              'while debugging.'))

    # Platform options
    group = parser.add_argument_group('Platform options')
    group.add_argument(
        '--performance-mode',
        choices=[
            android_device.HIGH_PERFORMANCE_MODE,
            android_device.NORMAL_PERFORMANCE_MODE,
            android_device.LITTLE_ONLY_PERFORMANCE_MODE,
            android_device.KEEP_PERFORMANCE_MODE
        ],
        default=android_device.HIGH_PERFORMANCE_MODE,
        help=(
            'Some platforms run on "full performance mode" where the '
            'test is executed at maximum CPU speed in order to minimize '
            'noise (specially important for dashboards / continuous builds). '
            'This option allows to choose performance mode. Available '
            'choices: high (default): high performance mode; normal: normal '
            'performance mode; little-only: execute the benchmark on little '
            'cores only; keep: don\'t touch the device performance settings.'))
    # TODO(crbug.com/1025207): Rename this to --support-apk
    group.add_argument(
        '--webview-embedder-apk',
        action="append",
        default=[],
        help=('When running tests on android webview, more than one apk needs '
              'to be installed. The apk running the test is said to embed '
              'webview. More than one apk may be specified if needed.'))

    # Remote platform options
    group = parser.add_argument_group('Remote platform options')
    group.add_argument('--android-denylist-file',
                       help='Device denylist JSON file.')
    group.add_argument(
        '--device',
        help=('The device ID to use. If not specified, only 0 or 1 connected '
              'devices are supported. If specified as "android", all available '
              'Android devices are used.'))
    group.add_argument(
        '--initial-find-device-attempts',
        type=int,
        default=1,
        help=('The max number of times devices will be looked for when '
              'finding browsers, retrying if no devices are found. Used to '
              'work around flaky, transient device disappearances.'))
    group.add_argument(
        '--install-bundle-module',
        dest='modules_to_install',
        action='append',
        default=[],
        help=('Specify Android App Bundle modules to install in addition to '
              'the base module. Ignored on Non-Android platforms.'))
    group.add_argument(
        '--compile-apk',
        help=('Compiles the APK under test using dex2oat in the specified '
              'mode. Ignored on non-Android platforms.'))
    group.add_argument(
        '--avd-config',
        help=('A path to an AVD configuration to use for starting an Android '
              'emulator.'))

    # Cast browser options
    group = parser.add_argument_group('Cast browser options')
    group.add_argument('--cast-output-dir',
                       help='Output directory for Cast Core.')
    group.add_argument('--cast-runtime-exe',
                       help='Path to Cast Web Runtime executable.')
    group.add_argument('--local-cast',
                       action='store_true',
                       help='Use a local casting receiver on the host.')
    group.add_argument('--cast-device-ip',
                       help='IP address of the Cast device.')

    group = parser.add_argument_group('Fuchsia platform options')
    group.add_argument('--fuchsia-target-id',
                       help='The Fuchsia target id used by the ffx tool.')

    # CPU profiling on Android/Linux/ChromeOS.
    group = parser.add_argument_group(
        ('CPU profiling over intervals of interest, Android, Linux, and '
         'ChromeOS only'))
    group.add_argument(
        '--interval-profiling-target',
        default='renderer:main',
        metavar='PROCESS_NAME[:THREAD_NAME]|"system_wide"',
        help=('Run the CPU profiler on this process/thread '
              '(default=%(default)s), which is supported only on Linux and '
              'Android, or system-wide, which is supported only on ChromeOS.'))
    group.add_argument(
        '--interval-profiling-period',
        dest='interval_profiling_periods',
        choices=('navigation', 'interactions', 'story_run'),
        action='append',
        default=[],
        metavar='PERIOD',
        help=('Run the CPU profiler during this test period. May be specified '
              'multiple times except when the story_run period is used. '
              'Profile data will be written to artifacts/*.perf.data '
              '(Android/ChromeOS) or artifacts/*.profile.pb (Linux) files in '
              'the output directory. See '
              'https://developer.android.com/ndk/guides/simpleperf for more '
              'info on Android profiling via simpleperf.'))
    group.add_argument(
        '--interval-profiling-frequency',
        default=1000,
        metavar='FREQUENCY',
        type=int,
        help=('Frequency of CPU profiling samples, in samples per second '
              '(default=%(default)s). This flag is used only on Android'))
    group.add_argument(
        '--interval-profiler-options',
        type=str,
        metavar='"--flag <options> ..."',
        help=('Addtional arguments to pass to the CPU profiler. This is used '
              'only on ChromeOS. On ChromeOS, pass the linux perf\'s '
              'subcommand name followed by the options to pass to the perf '
              'tool. Supported perf subcommands are "record" and "stat". E.g.: '
              '"record -e cycles -c 4000000 -g". Note: "-a" flag is added to '
              'the perf command by default. Do not pass options that are '
              'incompatible with the system-wide profile collection.'))

    # Browser options.
    self.browser_options.AddCommandLineArgs(parser)

    real_parse = parser.parse_args
    def ParseArgs(args=None):
      defaults = parser.get_default_values()
      for k, v in defaults.__dict__.items():
        if k in self.__dict__ and self.__dict__[k] is not None:
          continue
        self.__dict__[k] = v
      ret = real_parse(args, self)

      if self.chromium_output_dir:
        self.chromium_output_dir = os.path.abspath(self.chromium_output_dir)
        os.environ['CHROMIUM_OUTPUT_DIR'] = self.chromium_output_dir

      # Set up Android emulator if necessary.
      self.ParseAndroidEmulatorOptions()

      # Parse remote platform options.
      self.BuildRemotePlatformOptions()

      if self.remote_platform_options.device == 'list':
        if binary_manager.NeedsInit():
          binary_manager.InitDependencyManager([])
        devices = device_finder.GetDevicesMatchingOptions(self)
        print('Available devices:')
        for device in devices:
          print(' ', device.name)
        sys.exit(0)

      if self.browser_executable and not self.browser_type:
        self.browser_type = 'exact'
      if self.browser_type == 'list':
        if binary_manager.NeedsInit():
          binary_manager.InitDependencyManager([])
        devices = device_finder.GetDevicesMatchingOptions(self)
        if not devices:
          sys.exit(0)
        browser_types = {}
        for device in devices:
          try:
            possible_browsers = browser_finder.GetAllAvailableBrowsers(self,
                                                                       device)
            browser_types[device.name] = sorted(
                [browser.browser_type for browser in possible_browsers])
          except browser_finder_exceptions.BrowserFinderException as ex:
            print('ERROR: ', ex, file=sys.stderr)
            sys.exit(1)
        print('Available browsers:')
        if len(browser_types) == 0:
          print('  No devices were found.')
        for device_name in sorted(browser_types.keys()):
          print('  ', device_name)
          for browser_type in browser_types[device_name]:
            print('    ', browser_type)
          if len(browser_types[device_name]) == 0:
            print('     No browsers found for this device')
        sys.exit(0)

      if ((self.browser_type == 'cros-chrome'
           or self.browser_type == 'lacros-chrome')
          and (self.remote or self.fetch_cros_remote)
          and (self.remote_ssh_port < 0)):
        try:
          self.remote_ssh_port = socket.getservbyname('ssh')
        except OSError as e:
          raise RuntimeError(
              'Running a CrOS test in remote mode, but failed to retrieve port '
              'used by SSH service. This likely means SSH is not installed on '
              'the system. Original error: %s' % e) from e

      # Profiling other periods along with the story_run period leads to running
      # multiple profiling processes at the same time. The effects of performing
      # muliple CPU profiling at the same time is unclear and may generate
      # incorrect profiles so this will not be supported.
      if (len(self.interval_profiling_periods) > 1
          and 'story_run' in self.interval_profiling_periods):
        print('Cannot specify other periods along with the story_run period.')
        sys.exit(1)

      self.interval_profiler_options = shlex.split(
          self.interval_profiler_options, posix=(not _IsWin()))

      # Parse browser options.
      self.browser_options.UpdateFromParseResults(self)

      return ret

    # This ideally wouldn't need to exist, but the spaghetti code left over from
    # the use of optparse means that code exists that relies on attributes
    # being set before argument parsing is actually done.
    def get_default_values():
      defaults, _ = real_parse([])
      return defaults

    parser.parse_args = ParseArgs
    parser.get_default_values = get_default_values
    return parser

  def IsBrowserTypeRelevant(self, browser_type):
    """Determines if the browser_type is worth initializing.

    This check is used to sidestep any unnecessary work involved with searching
    for a browser that might not actually be needed. For example, this check
    could be used to prevent Telemetry from searching for a Clank browser if
    browser_type is android-webview.
    """
    return (browser_type == self.browser_type or
            self.browser_type in ('list', 'any',))

  def IsBrowserTypeReference(self):
    """Determines if the browser_type is a reference browser_type."""
    return self.browser_type and self.browser_type.startswith('reference-')

  def IsBrowserTypeBundle(self):
    """Determines if the browser_type is a bundle browser_type."""
    return self.browser_type and self.browser_type.endswith('-bundle')

  def _NoOpFunctionForTesting(self):
    """No-op function that can be overridden for unittests."""

  def ParseAndroidEmulatorOptions(self):
    """Parses Android emulator args, and if necessary, starts an emulator.

    No-ops if --avd-config is not specified or if an emulator is already
    started.

    Performing this setup during argument parsing isn't ideal, but we need to
    set up the emulator sometime between arg parsing and browser finding.
    """
    if not self.avd_config:
      return
    if BrowserFinderOptions.emulator_environment:
      return
    build_android_dir = os.path.join(
        util.GetChromiumSrcDir(), 'build', 'android')
    if not os.path.exists(build_android_dir):
      raise RuntimeError(
          '--avd-config specified, but Chromium //build/android directory not '
          'available')
    # Everything after this point only works if //build/android is actually
    # available, which we can't rely on, so use this to exit early in unittests.
    self._NoOpFunctionForTesting()
    sys.path.append(build_android_dir)
    # pylint: disable=import-error,import-outside-toplevel
    from pylib import constants as pylib_constants
    from pylib.local.emulator import local_emulator_environment
    # pylint: enable=import-error,import-outside-toplevel

    # We need to call this so that the Chromium output directory is set if it
    # is not explicitly specified via the command line argument/environment
    # variable.
    pylib_constants.CheckOutputDirectory()

    class AvdArgs():
      """A class to stand in for the AVD argparse.ArgumentParser object.

      Chromium's Android emulator code expects quite a few arguments from
      //build/android/test_runner.py, but the only one we actually care about
      here is avd_config. So, use a stand-in class with some sane defaults.
      """
      def __init__(self, avd_config):
        self.avd_config = avd_config
        self.emulator_count = 1
        self.emulator_window = False
        self.use_webview_provider = False
        self.replace_system_package = False
        self.denylist_file = None
        self.test_devices = []
        self.enable_concurrent_adb = False
        self.disable_test_server = False
        self.logcat_output_dir = None
        self.logcat_output_file = None
        self.num_retries = 1
        self.recover_devices = False
        self.skip_clear_data = True
        self.tool = None
        self.adb_path = None
        self.enable_device_cache = True
        # We don't want to use a persistent shell for setting up an emulator
        # as the persistent shell doesn't work until the emulator is already
        # running.
        self.use_persistent_shell = False
        self.emulator_debug_tags = None
        self.emulator_enable_network = False

    avd_args = AvdArgs(self.avd_config)
    BrowserFinderOptions.emulator_environment =\
        local_emulator_environment.LocalEmulatorEnvironment(
            avd_args, None, None)
    BrowserFinderOptions.emulator_environment.SetUp()
    atexit_with_log.Register(BrowserFinderOptions.emulator_environment.TearDown)

  # TODO(eakuefner): Factor this out into OptionBuilder pattern
  def BuildRemotePlatformOptions(self):
    if self.device or self.android_denylist_file:
      self.remote_platform_options = (
          remote_platform_options.AndroidPlatformOptions(
              self.device, self.android_denylist_file))

      # We delete these options because they should live solely in the
      # AndroidPlatformOptions instance belonging to this class.
      if self.device:
        del self.device
      if self.android_denylist_file:
        del self.android_denylist_file
    else:
      self.remote_platform_options = (
          remote_platform_options.AndroidPlatformOptions())

  def AppendExtraBrowserArgs(self, args):
    self.browser_options.AppendExtraBrowserArgs(args)

  def MergeDefaultValues(self, defaults):
    for k, v in defaults.__dict__.items():
      if not hasattr(self, k) or getattr(self, k) is None:
        setattr(self, k, v)


class BrowserOptions():
  """Options to be used for launching a browser."""
  # Allows clients to check whether they are dealing with a browser_options
  # object, without having to import this module. This may be needed in some
  # cases to avoid cyclic-imports.
  IS_BROWSER_OPTIONS = True

  # Levels of browser logging.
  NO_LOGGING = 'none'
  NON_VERBOSE_LOGGING = 'non-verbose'
  VERBOSE_LOGGING = 'verbose'
  SUPER_VERBOSE_LOGGING = 'super-verbose'

  _LOGGING_LEVELS = (NO_LOGGING, NON_VERBOSE_LOGGING, VERBOSE_LOGGING,
                     SUPER_VERBOSE_LOGGING)
  _DEFAULT_LOGGING_LEVEL = NO_LOGGING

  def __init__(self):
    self.browser_type = None
    self.show_stdout = False

    self.extensions_to_load = []

    # When set to True, the browser will use the default profile.  Telemetry
    # will not provide an alternate profile directory.
    self.dont_override_profile = False
    self.profile_dir = None
    self.profile_type = None

    self.assert_gpu_compositing = False
    self._extra_browser_args = set()
    self.extra_wpr_args = []
    self.wpr_mode = wpr_modes.WPR_OFF

    # The amount of time Telemetry should wait for the browser to start.
    # This property is not exposed as a command line option.
    self._browser_startup_timeout = 60

    self.disable_background_networking = True
    self.browser_user_agent_type = None

    # Some benchmarks (startup, loading, and memory related) need this to get
    # more representative measurements. Only has an effect for page sets based
    # on SharedPageState. New clients should probably define their own shared
    # state and make cache clearing decisions on their own.
    self.flush_os_page_caches_on_start = False

    # Background pages of built-in component extensions can interfere with
    # performance measurements.
    # pylint: disable=invalid-name
    self.disable_component_extensions_with_background_pages = True
    # Disable default apps.
    self.disable_default_apps = True

    self.logging_verbosity = self._DEFAULT_LOGGING_LEVEL

    # Whether to log verbose browser details like the full commandline used to
    # start the browser. This variable can be changed from one run to another
    # in order to cut back on log sizes. See crbug.com/943650.
    self.trim_logs = False

    # The cloud storage bucket & path for uploading logs data produced by the
    # browser to.
    # If logs_cloud_remote_path is None, a random remote path is generated every
    # time the logs data is uploaded.
    self.logs_cloud_bucket = cloud_storage.TELEMETRY_OUTPUT
    self.logs_cloud_remote_path = None

    # Whether to take screen shot for failed page & put them in telemetry's
    # profiling results.
    self.take_screenshot_for_failed_page = False

    # A list of tuples where the first element is path to an existing file,
    # and the second argument is a path (relative to the user-data-dir) to copy
    # the file to. Uses recursive directory creation if directories do not
    # already exist.
    self.profile_files_to_copy = []

    # The list of compatibility change that you want to enforce, mainly for
    # earlier versions of Chrome
    self.compatibility_mode = []

    # If not None, a ProjectConfig object with information about the benchmark
    # runtime environment.
    self.environment = None

  def __repr__(self):
    return str(sorted(self.__dict__.items()))

  def Copy(self):
    return copy.deepcopy(self)

  def IsCrosBrowserOptions(self):
    return False

  @classmethod
  def AddCommandLineArgs(cls, parser):

    ############################################################################
    # Please do not add any more options here without first discussing with    #
    # a telemetry owner. This is not the right place for platform-specific     #
    # options.                                                                 #
    ############################################################################

    group = parser.add_argument_group('Browser options')
    profile_choices = profile_types.GetProfileTypes()
    group.add_argument(
        '--profile-type',
        default='clean',
        choices=profile_choices,
        help='The user profile to use. A clean profile is used by default.')
    group.add_argument(
        '--profile-dir',
        help=('Profile directory to launch the browser with. A clean profile '
              'is used by default'))
    group.add_argument(
        '--extra-browser-args',
        dest='extra_browser_args_as_string',
        help='Additional arguments to pass to the browser when it starts')
    group.add_argument(
        '--extra-wpr-args',
        dest='extra_wpr_args_as_string',
        help=('Additional arguments to pass to Web Page Replay. '
              'See third_party/web-page-replay/replay.py for usage.'))
    group.add_argument(
        '--show-stdout',
        action='store_true',
        help='When possible, will display the stdout of the process')

    group.add_argument(
        '--browser-logging-verbosity',
        dest='logging_verbosity',
        choices=cls._LOGGING_LEVELS,
        default=cls._DEFAULT_LOGGING_LEVEL,
        help=('Browser logging verbosity. The log file is saved in temp '
              "directory. Note that logging affects the browser's "
              'performance. Defaults to %(default)s.'))
    group.add_argument(
        '--assert-gpu-compositing',
        action='store_true',
        help='Assert the browser uses gpu compositing and not software path.')

    group = parser.add_argument_group('Compatibility options')
    group.add_argument(
        '--gtest_output',
        help='Ignored argument for compatibility with runtest.py harness')

  def UpdateFromParseResults(self, finder_options):
    """Copies our options from finder_options."""
    browser_options_list = [
        'extra_browser_args_as_string',
        'extra_wpr_args_as_string',
        'profile_dir',
        'profile_type',
        'show_stdout',
        'compatibility_mode'
        ]
    for o in browser_options_list:
      a = getattr(finder_options, o, None)
      if a is not None:
        setattr(self, o, a)
        delattr(finder_options, o)

    self.browser_type = finder_options.browser_type

    if hasattr(self, 'extra_browser_args_as_string'):
      tmp = shlex.split(self.extra_browser_args_as_string, posix=(not _IsWin()))
      self.AppendExtraBrowserArgs(tmp)
      delattr(self, 'extra_browser_args_as_string')
    if hasattr(self, 'extra_wpr_args_as_string'):
      tmp = shlex.split(self.extra_wpr_args_as_string, posix=(not _IsWin()))
      self.extra_wpr_args.extend(tmp)
      delattr(self, 'extra_wpr_args_as_string')
    if self.profile_type == 'default':
      self.dont_override_profile = True

    if self.profile_dir:
      if self.profile_type != 'clean':
        logging.critical(
            "It's illegal to specify both --profile-type and --profile-dir.\n"
            "For more information see: http://goo.gl/ngdGD5")
        sys.exit(1)
      self.profile_dir = os.path.abspath(self.profile_dir)

    if self.profile_dir and not os.path.isdir(self.profile_dir):
      logging.critical(
          "Directory specified by --profile-dir (%s) doesn't exist "
          "or isn't a directory.\n"
          "For more information see: http://goo.gl/ngdGD5" % self.profile_dir)
      sys.exit(1)

    if not self.profile_dir:
      self.profile_dir = profile_types.GetProfileDir(self.profile_type)

    if getattr(finder_options, 'logging_verbosity'):
      self.logging_verbosity = finder_options.logging_verbosity
      delattr(finder_options, 'logging_verbosity')

    # This deferred import is necessary because browser_options is imported in
    # telemetry/telemetry/__init__.py.
    finder_options.browser_options = CreateChromeBrowserOptions(self)

  @property
  def extra_browser_args(self):
    return self._extra_browser_args

  @property
  def browser_startup_timeout(self):
    return self._browser_startup_timeout

  @browser_startup_timeout.setter
  def browser_startup_timeout(self, value):
    self._browser_startup_timeout = value

  def AppendExtraBrowserArgs(self, args):
    if isinstance(args, list):
      self._extra_browser_args.update(args)
    else:
      self._extra_browser_args.add(args)

  def ConsolidateValuesForArg(self, flag):
    """Consolidates values from multiple instances of a browser arg.

    As a concrete example from Chrome, the --enable-features flag can only be
    passed to the browser once and uses a comma-separated list of feature names.
    If the stored browser arguments originally have ['--enable-features=foo',
    '--enable-features=bar'], then calling ConsolidateValuesForArg(
    '--enable-features') will cause the stored browser arguments to instead
    contain ['--enable-features=bar,foo'].

    Args:
      flag: A string containing the flag/argument to consolidate, including any
          leading dashes.
    """
    consolidated_args = []
    found_values = []
    for arg in self.extra_browser_args:
      if '=' in arg and arg.split('=', 1)[0] == flag:
        # Syntax is `--flag=A,B`.
        # Support for the `--flag A,B` syntax isn't present since the extra
        # browser args are stored as a set, and thus there is no guarantee that
        # space-separated flags will remain together.
        _, value = arg.split('=', 1)
        found_values.append(value)
      else:
        # No consolidation needed.
        consolidated_args.append(arg)

    if found_values:
      consolidated_args.append('%s=%s' % (flag, ','.join(found_values)))
    self._extra_browser_args = set(consolidated_args)


def CreateChromeBrowserOptions(br_options):
  browser_type = br_options.browser_type

  if (platform.GetHostPlatform().GetOSName() == 'chromeos' or
      (browser_type and 'cros' in browser_type)):
    return CrosBrowserOptions(br_options)

  return br_options


class ChromeBrowserOptions(BrowserOptions):
  """Chrome-specific browser options."""

  def __init__(self, br_options):
    super().__init__()
    # Copy to self.
    self.__dict__.update(br_options.__dict__)


class CrosBrowserOptions(ChromeBrowserOptions):
  """ChromeOS-specific browser options."""

  def __init__(self, br_options):
    super().__init__(br_options)
    # Create a browser with oobe property.
    self.create_browser_with_oobe = False
    # Clear enterprise policy before logging in.
    self.clear_enterprise_policy = True
    # By default, allow policy fetches to fail. A side effect is that the user
    # profile may become initialized before policy is available.
    # When this is set to True, chrome will not allow policy fetches to fail and
    # block user profile initialization on policy initialization.
    self.expect_policy_fetch = False
    # Disable GAIA/enterprise services.
    self.disable_gaia_services = True
    # Mute audio.
    self.mute_audio = True

    # TODO(cywang): crbug.com/760414
    # Add login delay for ARC container boot time measurement for now.
    # Should actually simulate username/password typing in the login
    # screen instead or make the wait time fixed for cros login.
    self.login_delay = 0

    self.auto_login = True
    self.gaia_login = False
    self.username = 'test@test.test'
    self.password = 'pwd'
    self.gaia_id = '12345'
    # For non-accelerated QEMU VMs.
    self.browser_startup_timeout = 240

  def IsCrosBrowserOptions(self):
    return True
