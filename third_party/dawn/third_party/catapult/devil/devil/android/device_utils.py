# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Provides a variety of device interactions based on adb."""
# pylint: disable=unused-argument

import calendar
import collections
import contextlib
import fnmatch
import json
import logging
import math
import os
import posixpath
import pprint
import random
import re
import shutil
import stat
import sys
import tempfile
import time
import threading
import uuid

import six

from devil import base_error
from devil import devil_env
from devil.utils import cmd_helper
from devil.android import apk_helper
from devil.android import device_signal
from devil.android import decorators
from devil.android import device_errors
from devil.android import device_temp_file
from devil.android import install_commands
from devil.android import logcat_monitor
from devil.android import md5sum
from devil.android.sdk import adb_wrapper
from devil.android.sdk import intent
from devil.android.sdk import keyevent
from devil.android.sdk import version_codes
from devil.utils import host_utils
from devil.utils import parallelizer
from devil.utils import reraiser_thread
from devil.utils import timeout_retry
from devil.utils import zip_utils

with devil_env.SysPath(devil_env.PY_UTILS_PATH):
  from py_utils import tempfile_ext

try:
  # We can't group this import because we want to treat it as optional
  from devil.utils import reset_usb  # pylint: disable=ungrouped-imports
except ImportError:
  # Fail silently if we can't import reset_usb. We're likely on windows.
  reset_usb = None

logger = logging.getLogger(__name__)

_BOOT_TIMEOUT = adb_wrapper.DEFAULT_TIMEOUT * 2
_BOOT_RETRIES = 2
_DEFAULT_TIMEOUT = adb_wrapper.DEFAULT_TIMEOUT
_DEFAULT_RETRIES = 3

# TODO(agrieve): Would be better to make this timeout based off of data size.
# Needs to be large for remote devices & speed depends on internet connection.
# Debug Chrome builds can be 200mb+.
_FILE_TRANSFER_TIMEOUT = adb_wrapper.DEFAULT_SUPER_LONG_TIMEOUT


# A sentinel object for default values
# TODO(jbudorick): revisit how default values are handled by
# the timeout_retry decorators.
DEFAULT = object()

# A sentinel object to require that calls to RunShellCommand force running the
# command with su even if the device has been rooted. To use, pass into the
# as_root param.
_FORCE_SU = object()

# Lists all files for the specified directories.
# In order to minimize data transfer, prints directories as absolute paths
# followed by files within that directory without their path.
_FILE_LIST_SCRIPT = """
  function list_files() {
    for f in "$1"/{.,}*
    do
      if [ "$f" == "." ] || [ "$f" == ".." ] || [ "$f" == "${1}/.*" ] \
          || [ "$f" == "${1}/*" ]
      then
        continue
      fi
      base=${f##*/} # Get the basename for the file, dropping the path.
      echo "$base"
    done
  }
  for dir in %s
  do
    if [ -d "$dir" ]; then
      echo "$dir"
      list_files "$dir"
    fi
  done
"""

_UNZIP_AND_CHMOD_SCRIPT = """
  {bin_dir}/unzip {zip_file} && (for dir in {dirs}
  do
    chmod -R 777 "$dir" || exit 1
  done)
"""

_MKDIR_SCRIPT = """
  for dir in {dirs}
  do
    mkdir -p "$dir"
  done
"""

# Not all permissions can be set.
_PERMISSIONS_DENYLIST_RE = re.compile('|'.join(
    fnmatch.translate(p) for p in [
        'android.permission.ACCESS_LOCATION_EXTRA_COMMANDS',
        'android.permission.ACCESS_MOCK_LOCATION',
        'android.permission.ACCESS_NETWORK_STATE',
        'android.permission.ACCESS_NOTIFICATION_POLICY',
        'android.permission.ACCESS_VR_STATE',
        'android.permission.ACCESS_WIFI_STATE',
        'android.permission.AUTHENTICATE_ACCOUNTS',
        'android.permission.BLUETOOTH',
        'android.permission.BLUETOOTH_ADMIN',
        'android.permission.BROADCAST_STICKY',
        'android.permission.CHANGE_NETWORK_STATE',
        'android.permission.CHANGE_WIFI_MULTICAST_STATE',
        'android.permission.CHANGE_WIFI_STATE',
        'android.permission.CREDENTIAL_MANAGER_QUERY_CANDIDATE_CREDENTIALS',
        'android.permission.CREDENTIAL_MANAGER_SET_ALLOWED_PROVIDERS',
        'android.permission.CREDENTIAL_MANAGER_SET_ORIGIN',
        'android.permission.DISABLE_KEYGUARD',
        'android.permission.DOWNLOAD_WITHOUT_NOTIFICATION',
        'android.permission.EXPAND_STATUS_BAR',
        'android.permission.FOREGROUND_SERVICE',
        'android.permission.FOREGROUND_SERVICE_DATA_SYNC',
        'android.permission.FOREGROUND_SERVICE_MEDIA_PLAYBACK',
        'android.permission.FOREGROUND_SERVICE_SPECIAL_USE',
        'android.permission.GET_PACKAGE_SIZE',
        'android.permission.INSTALL_SHORTCUT',
        'android.permission.INJECT_EVENTS',
        'android.permission.INTERNET',
        'android.permission.KILL_BACKGROUND_PROCESSES',
        'android.permission.MANAGE_ACCOUNTS',
        'android.permission.MANAGE_EXTERNAL_STORAGE',
        'android.permission.MODIFY_AUDIO_SETTINGS',
        'android.permission.NFC',
        'android.permission.QUERY_ALL_PACKAGES',
        'android.permission.READ_SYNC_SETTINGS',
        'android.permission.READ_SYNC_STATS',
        'android.permission.RECEIVE_BOOT_COMPLETED',
        'android.permission.RECORD_VIDEO',
        'android.permission.REORDER_TASKS',
        'android.permission.REQUEST_INSTALL_PACKAGES',
        'android.permission.RESTRICTED_VR_ACCESS',
        'android.permission.RUN_INSTRUMENTATION',
        'android.permission.RUN_USER_INITIATED_JOBS',
        'android.permission.SET_ALARM',
        'android.permission.SET_TIME_ZONE',
        'android.permission.SET_WALLPAPER',
        'android.permission.SET_WALLPAPER_HINTS',
        'android.permission.TRANSMIT_IR',
        'android.permission.USE_BIOMETRIC',
        'android.permission.USE_CREDENTIALS',
        'android.permission.USE_FINGERPRINT',
        'android.permission.VIBRATE',
        'android.permission.WAKE_LOCK',
        'android.permission.WRITE_SYNC_SETTINGS',
        'com.android.browser.permission.READ_HISTORY_BOOKMARKS',
        'com.android.browser.permission.WRITE_HISTORY_BOOKMARKS',
        'com.android.launcher.permission.INSTALL_SHORTCUT',
        'com.chrome.permission.DEVICE_EXTRAS',
        'com.google.android.apps.now.CURRENT_ACCOUNT_ACCESS',
        'com.google.android.c2dm.permission.RECEIVE',
        'com.google.android.finsky.permission.DSE',
        'com.google.android.googlequicksearchbox.permission.LENS_SERVICE',
        'com.google.android.providers.gsf.permission.READ_GSERVICES',
        'com.google.vr.vrcore.permission.VRCORE_INTERNAL',
        'com.sec.enterprise.knox.MDM_CONTENT_PROVIDER',
        '*.permission.C2D_MESSAGE',
        '*.permission.READ_WRITE_BOOKMARK_FOLDERS',
        '*.TOS_ACKED',
    ]))
_SHELL_OUTPUT_SEPARATOR = '~X~'
_PERMISSIONS_EXCEPTION_RE = re.compile(r'java\.lang\.\w+Exception: .*$',
                                       re.MULTILINE)

_CURRENT_FOCUS_CRASH_RE = re.compile(
    r'\s*mCurrentFocus.*Application (Error|Not Responding): (\S+)}')

_GETPROP_RE = re.compile(r'\[(.*?)\]: \[(.*?)\]')
_VERSION_CODE_SDK_RE = re.compile(
    r'\s*versionCode=(\d+).*minSdk=(\d+).*targetSdk=(.*)\s*')

# Regex to parse the long (-l) output of 'ls' command, c.f.
# https://github.com/landley/toybox/blob/master/toys/posix/ls.c#L446
# yapf: disable
_LONG_LS_OUTPUT_RE = re.compile(
    r'(?P<st_mode>[\w-]{10})\s+'                  # File permissions
    r'(?:(?P<st_nlink>\d+)\s+)?'                  # Number of links (optional)
    r'(?P<st_owner>\w+)\s+'                       # Name of owner
    r'(?P<st_group>\w+)\s+'                       # Group of owner
    r'(?:'                                        # Either ...
      r'(?P<st_rdev_major>\d+),\s+'                 # Device major, and
      r'(?P<st_rdev_minor>\d+)\s+'                  # Device minor
    r'|'                                          # .. or
      r'(?P<st_size>\d+)\s+'                        # Size in bytes
    r')?'                                         # .. or nothing
    r'(?P<st_mtime>\d{4}-\d\d-\d\d \d\d:\d\d)\s+' # Modification date/time
    r'(?P<filename>.+?)'                          # File name
    r'(?: -> (?P<symbolic_link_to>.+))?'          # Symbolic link (optional)
    r'$'                                          # End of string
)
# yapf: enable

_LS_DATE_FORMAT = '%Y-%m-%d %H:%M'
_FILE_MODE_RE = re.compile(r'[dbclps-](?:[r-][w-][xSs-]){2}[r-][w-][xTt-]$')
_FILE_MODE_KIND = {
    'd': stat.S_IFDIR,
    'b': stat.S_IFBLK,
    'c': stat.S_IFCHR,
    'l': stat.S_IFLNK,
    'p': stat.S_IFIFO,
    's': stat.S_IFSOCK,
    '-': stat.S_IFREG
}
_FILE_MODE_PERMS = [
    stat.S_IRUSR,
    stat.S_IWUSR,
    stat.S_IXUSR,
    stat.S_IRGRP,
    stat.S_IWGRP,
    stat.S_IXGRP,
    stat.S_IROTH,
    stat.S_IWOTH,
    stat.S_IXOTH,
]
_FILE_MODE_SPECIAL = [
    ('s', stat.S_ISUID),
    ('s', stat.S_ISGID),
    ('t', stat.S_ISVTX),
]
_PS_COLUMNS = {'pid': 1, 'ppid': 2, 'name': -1}
_SELINUX_MODE = {'enforcing': True, 'permissive': False, 'disabled': None}
# Some devices require different logic for checking if root is necessary
_SPECIAL_ROOT_DEVICE_LIST = [
    'marlin',  # Pixel XL
    'sailfish',  # Pixel
    'taimen',  # Pixel 2 XL
    'vega',  # Lenovo Mirage Solo
    'walleye',  # Pixel 2
    'crosshatch',  # Pixel 3 XL
    'blueline',  # Pixel 3
    'sargo',  # Pixel 3a
    'bonito',  # Pixel 3a XL
    'sdk_goog3_x86',  # Crow emulator
]
_SPECIAL_ROOT_DEVICE_LIST += [
    'aosp_%s' % _d for _d in _SPECIAL_ROOT_DEVICE_LIST
]
_ALTERNATE_SCREENSHOT_CMD_DEVICES = {
    'flame',  # Pixel 4
    'oriole',  # Pixel 6
}

# Streamed installation is introduced in Nougat. Some devices and emulators
# at some API levels are slow/timeout with default streaming app install so
# force to use no_streaming instead.
_NO_STREAMING_DEVICE_LIST = [
    'flounder',  # Nexus 9
    'volantis',  # Another product name for Nexus 9
]
_NO_STREAMING_EMULATOR_API_LEVELS = [
    version_codes.NOUGAT,
]

_IMEI_RE = re.compile(r'  Device ID = (.+)$')
# The following regex is used to match result parcels like:
"""
Result: Parcel(
  0x00000000: 00000000 0000000f 00350033 00360033 '........3.5.3.6.'
  0x00000010: 00360032 00370030 00300032 00300039 '2.6.0.7.2.0.9.0.'
  0x00000020: 00380033 00000039                   '3.8.9...        ')
"""
_PARCEL_RESULT_RE = re.compile(
    r'0x[0-9a-f]{8}\: (?:[0-9a-f]{8}\s+){1,4}\'(.{16})\'')

# http://bit.ly/2WLZhUF added a timeout to adb wait-for-device. We sometimes
# want to wait longer than the implicit call within adb root allows.
_WAIT_FOR_DEVICE_TIMEOUT_STR = 'timeout expired while waiting for device'

_WEBVIEW_SYSUPDATE_CURRENT_PKG_RE = re.compile(
    r'Current WebView package.*:.*\(([a-z.]*),\s+(\d+\.\d+\.\d+\.\d+)\)')
_WEBVIEW_SYSUPDATE_NULL_PKG_RE = re.compile(r'Current WebView package is null')
_WEBVIEW_SYSUPDATE_FALLBACK_LOGIC_RE = re.compile(
    r'Fallback logic enabled: (true|false)')
_WEBVIEW_SYSUPDATE_PACKAGE_INSTALLED_RE = re.compile(
    r'(?:Valid|Invalid) package\s+(\S+)\s+\(.*\),?\s+(.*)$')
_WEBVIEW_SYSUPDATE_PACKAGE_NOT_INSTALLED_RE = re.compile(
    r'(\S+)\s+(is NOT installed\.)')
_WEBVIEW_SYSUPDATE_MIN_VERSION_CODE = re.compile(
    r'Minimum WebView version code: (\d+)')

_GOOGLE_FEATURES_RE = re.compile(r'^\s*com\.google\.')

# On Android < 12, "ro.product.device" starts with "generic_"
# On Android >= 12, "ro.product.device" starts with "emulator64_"
_EMULATOR_RE = re.compile(r'^(generic_|emulator64_).*$')

# Regular expressions for determining if a package is installed using the
# output of `dumpsys package`.
# Matches lines like "Package [com.google.android.youtube] (c491050):".
# or "Package [org.chromium.trichromelibrary_425300033] (e476383):"
_DUMPSYS_PACKAGE_RE_STR =\
    r'^\s*Package\s*\[{package}\]\s*\(\w*\):$'
# Regular expressions for determining if a package is installed for a user
# usuing the output of `dumpsys package`.
# Matches lines like "User 10: ceDataInode=736318 installed=true hidden=false"
_DUMPSYS_PACKAGE_USER_RE_STR =\
    r'^\s+User {user_id}:.*\sinstalled=(?P<is_installed>\w+)\s'

PS_COLUMNS = ('name', 'pid', 'ppid')
ProcessInfo = collections.namedtuple('ProcessInfo', PS_COLUMNS)

# The list of Rock960 device family.
ROCK960_DEVICE_LIST = [
    'rk3399', 'rk3399-all', 'rk3399-box'
]

_USER_LRU_RE = re.compile(r"^\s*mUserLru:\s*\[([\d\s,]+)\]$")
# Match user info like "UserInfo{0:Driver:813}" or "UserInfo{10:a:b:c:412}".
#  * 0 and 10 are user ids in integer.
#  * "Driver" and "a:b:c" are user names in string.
#  * 813 and 412 are user flags in hex string.
# More details can be found in https://bit.ly/3YQV03P
_USER_INFO_RE = re.compile(
    r'\s*UserInfo\{(?P<id>\d+):(?P<name>.+):(?P<flags>[0-9A-Fa-f]+)\}')
# User with administrative privileges. Such a user can create and delete users.
_USER_FLAG_ADMIN = 0x00000002
# Indicates that this user is a non-profile human user.
_USER_FLAG_FULL = 0x00000400
# Flagged as main user on the device.
#  * on Headless System User Mode (hsum), main user is the first human user.
#  * on non-hsum, main user is the system user (user 0)
_USER_FLAG_MAIN = 0x00004000


# Namespaces for settings
class SettingsNamespace:
  GLOBAL = 'global'
  SECURE = 'secure'
  SYSTEM = 'system'


@decorators.WithExplicitTimeoutAndRetries(_DEFAULT_TIMEOUT, _DEFAULT_RETRIES)
def GetAVDs():
  """Returns a list of Android Virtual Devices.

  Returns:
    A list containing the configured AVDs.
  """
  lines = cmd_helper.GetCmdOutput([
      os.path.join(
          devil_env.config.LocalPath('android_sdk'), 'tools', 'android'),
      'list', 'avd'
  ]).splitlines()
  avds = []
  for line in lines:
    if 'Name:' not in line:
      continue
    key, value = (s.strip() for s in line.split(':', 1))
    if key == 'Name':
      avds.append(value)
  return avds


def _ParseModeString(mode_str):
  """Parse a mode string, e.g. 'drwxrwxrwx', into a st_mode value.

  Effectively the reverse of |mode_to_string| in, e.g.:
  https://github.com/landley/toybox/blob/master/lib/lib.c#L896
  """
  if not _FILE_MODE_RE.match(mode_str):
    raise ValueError('Unexpected file mode %r' % mode_str)
  mode = _FILE_MODE_KIND[mode_str[0]]
  for c, flag in zip(mode_str[1:], _FILE_MODE_PERMS):
    if c != '-' and c.islower():
      mode |= flag
  for c, (t, flag) in zip(mode_str[3::3], _FILE_MODE_SPECIAL):
    if c.lower() == t:
      mode |= flag
  return mode


def _GetTimeStamp():
  """Return a basic ISO 8601 time stamp with the current local time."""
  return time.strftime('%Y%m%dT%H%M%S', time.localtime())


def _JoinLines(lines):
  # makes sure that the last line is also terminated, and is more memory
  # efficient than first appending an end-line to each line and then joining
  # all of them together.
  return ''.join(s for line in lines for s in (line, '\n'))


def _CreateAdbWrapper(device, **kwargs):
  if isinstance(device, adb_wrapper.AdbWrapper):
    return device

  return adb_wrapper.AdbWrapper(device, **kwargs)


def _FormatPartialOutputError(output):
  lines = output.splitlines() \
          if isinstance(output, six.string_types) else output
  message = ['Partial output found:']
  if len(lines) > 11:
    message.extend('- %s' % line for line in lines[:5])
    message.extend('<snip>')
    message.extend('- %s' % line for line in lines[-5:])
  else:
    message.extend('- %s' % line for line in lines)
  return '\n'.join(message)


_PushableComponents = collections.namedtuple('_PushableComponents',
                                             ('host', 'device', 'collapse'))


def _IterPushableComponents(host_path, device_path):
  """Yields a sequence of paths that can be pushed directly via adb push.

  `adb push` doesn't currently handle pushing directories that contain
  symlinks: https://bit.ly/2pMBlW5

  To circumvent this issue, we get the smallest set of files and/or
  directories that can be pushed without attempting to push a directory
  that contains a symlink.

  This function does so by recursing through |host_path|. Each call
  yields 3-tuples that include the smallest set of (host, device) path pairs
  that can be passed to adb push and a bool indicating whether the parent
  directory can be pushed -- i.e., if True, the host path is neither a
  symlink nor a directory that contains a symlink.

  Args:
    host_path: an absolute path of a file or directory on the host
    device_path: an absolute path of a file or directory on the device
  Yields:
    3-tuples containing
      host (str): the host path, with symlinks dereferenced
      device (str): the device path
      collapse (bool): whether this entity permits its parent to be pushed
        in its entirety. (Parents need permission from all child entities
        in order to be pushed in their entirety.)
  """
  if os.path.isfile(host_path):
    yield _PushableComponents(
        os.path.realpath(host_path), device_path, not os.path.islink(host_path))
  else:
    components = []
    for child in os.listdir(host_path):
      components.extend(
          _IterPushableComponents(
              os.path.join(host_path, child), posixpath.join(
                  device_path, child)))

    if all(c.collapse for c in components):
      yield _PushableComponents(
          os.path.realpath(host_path), device_path,
          not os.path.islink(host_path))
    else:
      for c in components:
        yield c


class DeviceUtils(object):

  _MAX_ADB_COMMAND_LENGTH = 512
  _MAX_ADB_OUTPUT_LENGTH = 32768
  _RESUMED_LAUNCHER_ACTIVITY_RE = re.compile(
      r'\s*(m|top)ResumedActivity.*(Launcher|launcher).*')
  _VALID_SHELL_VARIABLE = re.compile('^[a-zA-Z_][a-zA-Z0-9_]*$')

  LOCAL_PROPERTIES_PATH = posixpath.join('/', 'data', 'local.prop')

  # Property in /data/local.prop that controls Java assertions.
  JAVA_ASSERT_PROPERTY = 'dalvik.vm.enableassertions'

  def __init__(self,
               device,
               enable_device_files_cache=False,
               target_user=None,
               default_timeout=_DEFAULT_TIMEOUT,
               default_retries=_DEFAULT_RETRIES,
               persistent_shell=False):
    """DeviceUtils constructor.

    Args:
      device: Either a device serial, an existing AdbWrapper instance, or an
        an existing AndroidCommands instance.
      enable_device_files_cache: For PushChangedFiles(), cache checksums of
        pushed files rather than recomputing them on a subsequent call.
      target_user: Explicitly run applicable shell commands with the target
        user on device.
      default_timeout: An integer containing the default number of seconds to
        wait for an operation to complete if no explicit value is provided.
      default_retries: An integer containing the default number or times an
        operation should be retried on failure if no explicit value is provided.
      persistent_shell: A boolean indicating if a persistent shell connection
        should be used.
    """
    self.adb = None
    if isinstance(device, six.string_types):
      self.adb = _CreateAdbWrapper(device, persistent_shell=persistent_shell)

    elif isinstance(device, adb_wrapper.AdbWrapper):
      self.adb = device
    else:
      raise ValueError('Unsupported device value: %r' % device)
    self._commands_installed = None
    self._default_timeout = default_timeout
    self._default_retries = default_retries
    self._enable_device_files_cache = enable_device_files_cache
    self._target_user = target_user
    self._cache = {}
    self._client_caches = {}
    self._cache_lock = threading.RLock()
    self._apex_lock = threading.Lock()
    assert hasattr(self, decorators.DEFAULT_TIMEOUT_ATTR)
    assert hasattr(self, decorators.DEFAULT_RETRIES_ATTR)

    self.ClearCache()

  @property
  def serial(self):
    """Returns the device serial."""
    return self.adb.GetDeviceSerial()

  @property
  def target_user(self):
    return self._target_user

  @target_user.setter
  def target_user(self, user_id):
    self._target_user = int(user_id)

  def __eq__(self, other):
    """Checks whether |other| refers to the same device as |self|.

    Args:
      other: The object to compare to. This can be a basestring, an instance
        of adb_wrapper.AdbWrapper, or an instance of DeviceUtils.
    Returns:
      Whether |other| refers to the same device as |self|.
    """
    return self.serial == str(other)

  def __lt__(self, other):
    """Compares two instances of DeviceUtils.

    This merely compares their serial numbers.

    Args:
      other: The instance of DeviceUtils to compare to.
    Returns:
      Whether |self| is less than |other|.
    """
    return self.serial < other.serial

  def __str__(self):
    """Returns the device serial."""
    return self.serial

  @decorators.WithTimeoutAndRetriesFromInstance()
  def IsOnline(self, timeout=None, retries=None):
    """Checks whether the device is online.

    Args:
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      True if the device is online, False otherwise.

    Raises:
      CommandTimeoutError on timeout.
    """
    try:
      return self.adb.GetState() == 'device'
    except base_error.BaseError as exc:
      logger.info('Failed to get state: %s', exc, exc_info=True)
      return False

  @decorators.WithTimeoutAndRetriesFromInstance()
  def HasRoot(self, timeout=None, retries=None):
    """Checks whether or not adbd has root privileges.

    A device is considered to have root if all commands are implicitly run
    with elevated privileges, i.e. without having to use "su" to run them.

    Note that some devices do not allow this implicit privilige elevation,
    but _can_ run commands as root just fine when done explicitly with "su".
    To check if your device can run commands with elevated privileges at all
    use:

      device.HasRoot() or device.NeedsSU()

    Luckily, for the most part you don't need to worry about this and using
    RunShellCommand(cmd, as_root=True) will figure out for you the right
    command incantation to run with elevated privileges.

    Args:
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      True if adbd has root privileges, False otherwise.

    Raises:
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    if self.build_type == 'eng':
      # 'eng' builds have root enabled by default and the adb session cannot
      # be unrooted.
      return True
    # Check if uid is 0. Such behavior has remained unchanged since
    # android 2.2.3 (https://bit.ly/2QQzg67)
    output = self.RunShellCommand(['id'], single_line=True)
    return output.startswith('uid=0(root)')

  def NeedsSU(self, timeout=DEFAULT, retries=DEFAULT):
    """Checks whether 'su' is needed to access protected resources.

    Args:
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      True if 'su' is available on the device and is needed to to access
        protected resources; False otherwise if either 'su' is not available
        (e.g. because the device has a user build), or not needed (because adbd
        already has root privileges).

    Raises:
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    if 'needs_su' not in self._cache:
      cmd = '%s && ! ls /root' % self._Su('ls /root')
      # Devices using the system-as-root partition layout appear to not have
      # a /root directory. See http://bit.ly/37F34sx for more context.
      if (self.build_system_root_image == 'true'
          or self.build_version_sdk >= version_codes.Q
          # This may be redundant with the checks above.
          or self.product_name in _SPECIAL_ROOT_DEVICE_LIST):
        if self.HasRoot():
          self._cache['needs_su'] = False
          return False
        cmd = 'which which && which su'
      try:
        self.RunShellCommand(
            cmd,
            shell=True,
            check_return=True,
            timeout=self._default_timeout if timeout is DEFAULT else timeout,
            retries=self._default_retries if retries is DEFAULT else retries)
        self._cache['needs_su'] = True
      except device_errors.AdbCommandFailedError:
        self._cache['needs_su'] = False
    return self._cache['needs_su']

  def _Su(self, command):
    if self.build_version_sdk >= version_codes.MARSHMALLOW:
      return 'su 0 %s' % command
    return 'su -c %s' % command

  @decorators.WithTimeoutAndRetriesFromInstance()
  def EnableRoot(self, timeout=None, retries=None):
    """Restarts adbd with root privileges.

    Args:
      timeout: timeout in seconds
      retries: number of retries

    Raises:
      CommandFailedError if root could not be enabled.
      CommandTimeoutError on timeout.
    """
    if 'needs_su' in self._cache:
      del self._cache['needs_su']

    try:
      self.adb.Root()
    except device_errors.AdbCommandFailedError as e:
      if self.IsUserBuild():
        raise device_errors.RootUserBuildError(device_serial=str(self))
      if e.output and _WAIT_FOR_DEVICE_TIMEOUT_STR in e.output:
        # adb 1.0.41 added a call to wait-for-device *inside* root
        # with a timeout that can be too short in some cases.
        # If we hit that timeout, ignore it & do our own wait below.
        pass
      else:
        raise  # Failed probably due to some other reason.

    def device_online_with_root():
      try:
        self.adb.WaitForDevice()
        return self.HasRoot()
      except (device_errors.AdbCommandFailedError,
              device_errors.DeviceUnreachableError):
        return False

    timeout_retry.WaitFor(device_online_with_root, wait_period=1)

  @decorators.WithTimeoutAndRetriesFromInstance()
  def IsUserBuild(self, timeout=None, retries=None):
    """Checks whether or not the device is running a user build.

    Args:
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      True if the device is running a user build, False otherwise (i.e. if
        it's running a userdebug build).

    Raises:
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    return self.build_type == 'user'

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetExternalStoragePath(self, timeout=None, retries=None):
    """Get the device's path to its SD card.

    Note: this path is read-only by apps in R+. Use GetAppWritablePath() to
    obtain a path writable by apps.

    Args:
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      The device's path to its SD card.

    Raises:
      CommandFailedError if the external storage path could not be determined.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    self._EnsureCacheInitialized()
    if not self._cache['external_storage']:
      raise device_errors.CommandFailedError('$EXTERNAL_STORAGE is not set',
                                             str(self))
    return self._cache['external_storage']

  def GetAppWritablePath(self, timeout=None, retries=None):
    """Get a path that on the device's SD card that apps can write.

    Args:
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      A app-writeable path on the device's SD card.

    Raises:
      CommandFailedError if the external storage path could not be determined.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    if self.build_version_sdk >= version_codes.Q:
      # On Q+ apps don't require permissions to access well-defined media
      # locations like /sdcard/Download. On R+ the WRITE_EXTERNAL_STORAGE
      # permission no longer provides access to the external storage root. See
      # https://developer.android.com/preview/privacy/storage#permissions-target-11
      # So use /sdcard/Download for the app-writable path on those versions.
      return posixpath.join(self.GetExternalStoragePath(), 'Download')
    return self.GetExternalStoragePath()

  def ResolveSpecialPath(self, device_path):
    """Convert a path to one that is accessible by the shell.

    Usually need root permission.

    For example, system user 0 doesn't have the permission to access secondary
    user 10's sdcard via the path "/sdcard" or "/storage/emulated/10". However
    it can using the path like "/data/media/10" with the root permission.

    Returns:
      The converted path.
    """
    assert self.target_user is not None
    if self.target_user == 0:
      return device_path

    if device_path.startswith('/sdcard'):
      writable_path_base = f'/data/media/{self.target_user}'
      return writable_path_base + device_path[len('/sdcard'):]

    if device_path.startswith('/data/data'):
      writable_path_base = f'/data/user/{self.target_user}'
      return writable_path_base + device_path[len('/data/data'):]

    return device_path

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetIMEI(self, timeout=None, retries=None):
    """Get the device's IMEI.

    Args:
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      The device's IMEI.

    Raises:
      AdbCommandFailedError on error
    """
    if self._cache.get('imei') is not None:
      return self._cache.get('imei')

    if self.build_version_sdk < 21:
      out = self.RunShellCommand(['dumpsys', 'iphonesubinfo'],
                                 raw_output=True,
                                 check_return=True)
      if out:
        match = re.search(_IMEI_RE, out)
        if match:
          self._cache['imei'] = match.group(1)
          return self._cache['imei']
    else:
      out = self.RunShellCommand(['service', 'call', 'iphonesubinfo', '1'],
                                 check_return=True)
      if out:
        imei = ''
        for line in out:
          match = re.search(_PARCEL_RESULT_RE, line)
          if match:
            imei = imei + match.group(1)
        imei = imei.replace('.', '').strip()
        if imei:
          self._cache['imei'] = imei
          return self._cache['imei']

    raise device_errors.CommandFailedError('Unable to fetch IMEI.')

  @decorators.WithTimeoutAndRetriesFromInstance()
  def IsApplicationInstalled(self,
                             package,
                             library_version=None,
                             timeout=None,
                             retries=None):
    """Determines whether a particular package is installed on the device.

    Note for multi-user: when "--user" param is not specified,
      the "pm list packages" command applies to all users.

    Args:
      package: Name of the package.
      library_version: Required for shared-library apks. The version of the
          package to check for as an int.

    Returns:
      True if the application is installed, False otherwise.
    """
    # `pm list packages` doesn't include the version code, so if it was
    # provided, skip this since we can't guarantee that the installed
    # version is the requested version.
    if library_version is None:
      # `pm list packages` allows matching substrings, but we want exact matches
      # only.
      cmd = ['pm', 'list', 'packages']
      if self.target_user is not None:
        cmd.extend(['--user', str(self.target_user)])
      cmd.append(package)
      matching_packages = self.RunShellCommand(cmd, check_return=True)
      desired_line = 'package:' + package
      found_package = desired_line in matching_packages
      if found_package:
        return True

    # Some packages do not properly show up via `pm list packages`, so fall back
    # to checking via `dumpsys package`.
    return self._IsApplicationInstalledDumpsys(package,
                                               library_version=library_version)

  @decorators.WithTimeoutAndRetriesFromInstance()
  def _IsApplicationInstalledDumpsys(self,
                                     package,
                                     library_version=None,
                                     timeout=None,
                                     retries=None):
    # If the package exists, only its information is outputted. Otherwise, all
    # packages are output making for very large output.
    package_with_version = package
    if library_version:
      package_with_version += '_' + str(library_version)
    package_matcher = re.compile(
        _DUMPSYS_PACKAGE_RE_STR.format(package=re.escape(package_with_version)))

    package_user_matcher = None
    if self.target_user is not None:
      package_user_matcher = re.compile(
          _DUMPSYS_PACKAGE_USER_RE_STR.format(user_id=self.target_user))
    dumpsys_output = self.RunShellCommand(
        ['dumpsys', 'package', package_with_version],
        check_return=True,
        large_output=True)

    package_found = False
    for line in dumpsys_output:
      package_match = package_matcher.match(line)
      if package_match:
        # Keep checking if the package is installed for the given user
        if package_user_matcher:
          package_found = True
        else:
          return True
      if package_found and package_user_matcher:
        package_user_match = package_user_matcher.match(line)
        if package_user_match:
          is_installed = package_user_match.groupdict().get('is_installed')
          return is_installed == 'true'
    return False

  @decorators.WithTimeoutAndRetriesFromInstance()
  def IsSystemModuleInstalled(self,
                              package,
                              version_code,
                              timeout=None,
                              retries=None):
    """
    Checks the version for a mainline module (apex) to confirm if it's installed
    """
    dumpsys_output = self.RunShellCommand(['dumpsys', 'package', package],
                                          check_return=True,
                                          large_output=True)

    expected_version_line = 'Version: %s' % version_code

    for line in dumpsys_output:
      if expected_version_line in line:
        return True
    return False

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetApplicationPaths(self, package, timeout=None, retries=None):
    """Get the paths of the installed apks on the device for the given package.

    Args:
      package: Name of the package.

    Returns:
      List of paths to the apks on the device for the given package.
    """
    return self._GetApplicationPathsInternal(package)

  def _GetApplicationPathsInternal(self, package, skip_cache=False):
    """
    Note for multi-user: Though there may be multi-users, an application will
    only have exactly one copy on the device.
    """
    cached_result = self._cache['package_apk_paths'].get(package)
    if cached_result is not None and not skip_cache:
      if package in self._cache['package_apk_paths_to_verify']:
        self._cache['package_apk_paths_to_verify'].remove(package)
        # Don't verify an app that is not thought to be installed. We are
        # concerned only with apps we think are installed having been
        # uninstalled manually.
        if cached_result and not self.PathExists(cached_result):
          cached_result = None
          self._cache['package_apk_checksums'].pop(package, 0)
      if cached_result is not None:
        return list(cached_result)
    # 'pm path' is liable to incorrectly exit with a nonzero number starting
    # in Lollipop.
    # TODO(jbudorick): Check if this is fixed as new Android versions are
    # released to put an upper bound on this.
    should_check_return = (self.build_version_sdk < version_codes.LOLLIPOP)
    output = self.RunShellCommand(['pm', 'path', package],
                                  check_return=should_check_return)
    apks = []
    bad_output = False
    for line in output:
      if line.startswith('package:'):
        apks.append(line[len('package:'):])
      elif line.startswith('WARNING:'):
        continue
      else:
        bad_output = True  # Unexpected line in output.
    if not apks and output:
      if bad_output:
        raise device_errors.CommandFailedError(
            'Unexpected pm path output: %r' % '\n'.join(output), str(self))
      logger.warning('pm returned no paths but the following warnings:')
      for line in output:
        logger.warning('- %s', line)
    self._cache['package_apk_paths'][package] = list(apks)
    return apks

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetApplicationVersion(self, package, timeout=None, retries=None):
    """Get the version name of a package installed on the device.

    Args:
      package: Name of the package.

    Returns:
      A string with the version name or None if the package is not found
      on the device.
    """
    return self._GetPackageDetailFromDumpsys(package,
                                             'versionName=',
                                             timeout=timeout,
                                             retries=retries)

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetApplicationTargetSdk(self, package, timeout=None, retries=None):
    """Get the targetSdkVersion of a package installed on the device.

    Args:
      package: Name of the package.

    Returns:
      A string with the targetSdkVersion or None if the package is not found on
      the device. Note: this cannot always be cast to an integer. If this
      application targets a pre-release SDK, this returns the version codename
      instead (ex. "R").
    """
    if not self.IsApplicationInstalled(package):
      return None
    lines = self._GetDumpsysOutput(['package', package], 'targetSdk=')
    for line in lines:
      m = _VERSION_CODE_SDK_RE.match(line)
      if m:
        value = m.group(3)
        # 10000 is the code used by Android for a pre-finalized SDK.
        if value == '10000':
          return self.GetProp('ro.build.version.codename', cache=True)
        return value
    raise device_errors.CommandFailedError(
        'targetSdkVersion for %s not found on dumpsys output' % package,
        str(self))

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetPackageArchitecture(self, package, timeout=None, retries=None):
    """Get the architecture of a package installed on the device.

    Args:
      package: Name of the package.

    Returns:
      A string with the architecture, or None if the package is missing.
    """
    lines = self._GetDumpsysOutput(['package', package], 'primaryCpuAbi')
    if lines:
      _, _, package_arch = lines[-1].partition('=')
      return package_arch.strip()
    return None

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetApplicationDataDirectory(self, package, timeout=None, retries=None):
    """Get the data directory on the device for the given package.

    Args:
      package: Name of the package.

    Returns:
      The package's data directory.
    Raises:
      CommandFailedError if the package's data directory can't be found,
        whether because it's not installed or otherwise.
    """
    if not self.IsApplicationInstalled(package):
      raise device_errors.CommandFailedError('%s is not installed' % package,
                                             str(self))
    # The shell command "pm dump" may not always show the info for secondary
    # users. So handcraft the path.
    user_id = 0
    if self.target_user is not None:
      user_id = self.target_user
    data_dir = f'/data/user/{user_id}/{package}'
    if not self.PathExists(data_dir, as_root=True):
      raise device_errors.CommandFailedError(
          'Could not find data directory for %s' % package, str(self))
    return data_dir

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetSecurityContextForPackage(self,
                                   package,
                                   encrypted=False,
                                   timeout=None,
                                   retries=None):
    """Gets the SELinux security context for the given package.

    Args:
      package: Name of the package.
      encrypted: Whether to check in
        the encrypted data directory (/data/user_de/<user_id>/) or
        the unencrypted data directory (/data/user/<user_id>/).

    Returns:
      The package's security context as a string, or None if not found.
    """
    user_id = 0
    if self.target_user is not None:
      user_id = self.target_user
    directory = f'/data/user/{user_id}'
    if encrypted:
      directory = f'/data/user_de/{user_id}/'
    for line in self.RunShellCommand(['ls', '-Z', directory],
                                     as_root=True,
                                     check_return=True):
      split_line = line.split()
      # ls -Z output differs between Android versions, but the package is
      # always last and the context always starts with "u:object"
      if split_line[-1] == package:
        for column in split_line:
          if column.startswith('u:object'):
            return column
    return None

  def TakeBugReport(self, path, timeout=60 * 5, retries=None):
    """Takes a bug report and dumps it to the specified path.

    This doesn't use adb's bugreport option since its behavior is dependent on
    both adb version and device OS version. To make it simpler, this directly
    runs the bugreport command on the device itself and dumps the stdout to a
    file.

    Args:
      path: Path on the host to drop the bug report.
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.
    """
    with device_temp_file.DeviceTempFile(self.adb) as device_tmp_file:
      cmd = '( bugreport )>%s 2>&1' % device_tmp_file.name
      self.RunShellCommand(
          cmd, check_return=True, shell=True, timeout=timeout, retries=retries)
      self.PullFile(device_tmp_file.name, path)

  @decorators.WithTimeoutAndConditionalRetries(
      adb_wrapper.ShouldRetryAfterAdbServerRestart)
  def WaitUntilFullyBooted(self,
                           wifi=False,
                           decrypt=False,
                           timeout=_BOOT_TIMEOUT,
                           retries=_BOOT_RETRIES):
    """Wait for the device to fully boot.

    This means waiting for the device to boot, the package manager to be
    available, and the SD card to be ready.
    It can optionally wait the following:
     - Wait for wifi to come up.
     - Wait for full-disk decryption to complete.

    Args:
      wifi: A boolean indicating if we should wait for wifi to come up or not.
      decrypt: A boolean indicating if we should wait for full-disk decryption
        to complete.
      timeout: timeout in seconds
      retries: number of retries

    Raises:
      CommandFailedError on failure.
      CommandTimeoutError if one of the component waits times out.
      DeviceUnreachableError if the device becomes unresponsive.
    """

    def is_device_connection_ready():
      # Rock960 devices re-connect during boot process, presumably
      # due to change in USB protocol configuration, which causes restart of adb
      # daemon running on device and "re-connection" seen from adb client side.
      # Since device is unreachable after disconnecting and before re-connecting
      # for the second time, we must wait for re-connection and give control
      # back to devil code only when sys.usb.config property, which allows us to
      # differentiate between these states, switches to the right value. This
      # way we avoid "device unreachable" errors occuring when re-connections
      # happens.
      try:
        if self.GetProp('ro.product.model') not in ROCK960_DEVICE_LIST:
          return True
      except device_errors.CommandFailedError as e:
        logging.warning('Failed to get product_model: %s', e)
        return False
      except device_errors.DeviceUnreachableError:
        logging.warning('Failed to get product_model: device unreachable')
        return False

      try:
        return self.GetProp('sys.usb.config') == 'adb'
      except device_errors.CommandFailedError as e:
        logging.warning('Failed to get prop "sys.usb.config": %s', e)
        return False
      except device_errors.DeviceUnreachableError:
        logging.warning(
            'Failed to get prop "sys.usb.config": device unreachable')
        return False

    def is_sd_card_ready():
      try:
        self.RunShellCommand(
            ['test', '-d', self.GetExternalStoragePath()], check_return=True)
        return True
      except device_errors.DeviceUnreachableError:
        logging.warning('Failed to check sd_card_ready: device unreachable')
        return False
      except device_errors.AdbCommandFailedError:
        return False

    def is_pm_ready():
      try:
        return self._GetApplicationPathsInternal('android', skip_cache=True)
      except device_errors.DeviceUnreachableError:
        logging.warning('Failed to check pm_ready: device unreachable')
        return False
      except device_errors.CommandFailedError:
        return False

    def is_boot_completed():
      try:
        return any(
            self.GetProp(prop, cache=False) == '1'
            for prop in ('sys.boot_completed', 'dev.bootcomplete'))
      except device_errors.DeviceUnreachableError:
        logging.warning('Failed to check boot_completed: device unreachable')
        return False
      except device_errors.CommandFailedError:
        return False

    def is_wifi_enabled():
      return 'Wi-Fi is enabled' in self.RunShellCommand(['dumpsys', 'wifi'],
                                                        check_return=False)

    def is_decryption_completed():
      try:
        decrypt = self.GetProp('vold.decrypt', cache=False)
        # The prop "void.decrypt" will only be set when the device uses
        # full-disk encryption (FDE).
        # Return true when:
        #  - The prop is empty, which means the device is unencrypted or uses
        #    file-based encryption (FBE).
        #  - or the prop has value "trigger_restart_framework", which means
        #    the decription is finished.
        return decrypt in ('', 'trigger_restart_framework')
      except device_errors.CommandFailedError:
        return False

    self.adb.WaitForDevice()
    # Check that the device has booted
    timeout_retry.WaitFor(is_boot_completed)
    # Rock960 devices connected twice. Wait for device ready.
    timeout_retry.WaitFor(is_device_connection_ready)
    timeout_retry.WaitFor(is_sd_card_ready)
    timeout_retry.WaitFor(is_pm_ready)
    if wifi:
      timeout_retry.WaitFor(is_wifi_enabled)
    if decrypt:
      timeout_retry.WaitFor(is_decryption_completed)

  REBOOT_DEFAULT_TIMEOUT = adb_wrapper.DEFAULT_LONG_TIMEOUT

  @decorators.WithTimeoutAndRetriesFromInstance(
      min_default_timeout=REBOOT_DEFAULT_TIMEOUT)
  def Reboot(self,
             block=True,
             wifi=False,
             decrypt=False,
             timeout=None,
             retries=None):
    """Reboot the device.

    Note if the device has the root privilege, it will likely lose it after the
    reboot. When |block| is True, it will try to restore the root status if
    applicable.

    Args:
      block: A boolean indicating if we should wait for the reboot to complete.
      wifi: A boolean indicating if we should wait for wifi to be enabled after
        the reboot.
        The option has no effect unless |block| is also True.
      decrypt: A boolean indicating if we should wait for full-disk decryption
        to complete after the reboot.
        The option has no effect unless |block| is also True.
      timeout: timeout in seconds
      retries: number of retries

    Raises:
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """

    def device_offline():
      return not self.IsOnline()

    # Only check the root when block is True
    should_restore_root = self.HasRoot() if block else False
    self.adb.Reboot()
    self.ClearCache()
    timeout_retry.WaitFor(device_offline, wait_period=1)
    if block:
      self.WaitUntilFullyBooted(wifi=wifi, decrypt=decrypt)
      if should_restore_root:
        self.EnableRoot()

  INSTALL_DEFAULT_TIMEOUT = _FILE_TRANSFER_TIMEOUT
  MODULES_TMP_DIRECTORY_PATH = '/data/local/tmp/modules'
  MODULES_LOCAL_TESTING_PATH_TEMPLATE = (
      '/sdcard/Android/data/{}/files/local_testing')

  @decorators.WithTimeoutAndRetriesFromInstance(
      min_default_timeout=INSTALL_DEFAULT_TIMEOUT)
  def Install(self,
              apk,
              allow_downgrade=False,
              reinstall=False,
              permissions=None,
              timeout=None,
              retries=None,
              modules=None,
              fake_modules=None,
              additional_locales=None,
              instant_app=False,
              force_queryable=False):
    """Install an APK or app bundle.

    Noop if an identical APK is already installed. If installing a bundle, the
    bundletools helper script (bin/*_bundle) should be used rather than the .aab
    file.

    Args:
      apk: An ApkHelper instance or string containing the path to the APK or
          bundle.
      allow_downgrade: A boolean indicating if we should allow downgrades.
      reinstall: A boolean indicating if we should keep any existing app data.
          Ignored if |apk| is a bundle.
      permissions: Set of permissions to set. If not set, finds permissions with
          apk helper. To set no permissions, pass [].
      timeout: timeout in seconds
      retries: number of retries
      modules: An iterable containing specific bundle modules to install.
          Error if set and |apk| points to an APK instead of a bundle.
      fake_modules: An iterable containing specific bundle modules that should
          have their apks copied to |MODULES_LOCAL_TESTING_PATH_TEMPLATE|
          rather than installed. Thus the app can emulate SplitCompat while
          running. This should not have any overlap with |modules|.
      additional_locales: An iterable with additional locales to install for a
          bundle.
      instant_app: A boolean that selects if the APK should be installed as an
          instant app or not. Instant apps are installed in a more
          restrictive execution environment. - Supported from SDK 29
      force_queryable: A boolean that allows the installed application to be
        queryable by all other applications regardless of if they have declared
        the package as queryable in their manifests - Supported from SDK 30

    Raises:
      CommandFailedError if the installation fails.
      CommandTimeoutError if the installation times out.
      DeviceUnreachableError on missing device.
      DeviceVersionError if the device SDK level does not support instant
        apps or forcing queryable
    """
    apk = apk_helper.ToHelper(apk)
    modules_set = set(modules or [])
    fake_modules_set = set(fake_modules or [])
    assert modules_set.isdisjoint(fake_modules_set), (
        'These modules overlap: %s' % (modules_set & fake_modules_set))
    all_modules = modules_set | fake_modules_set
    package_name = apk.GetPackageName()

    with apk.GetApkPaths(self,
                         modules=all_modules,
                         additional_locales=additional_locales) as apk_paths:
      if apk.SupportsSplits():
        fake_apk_paths = self._GetFakeInstallPaths(apk_paths, fake_modules)
        self._FakeInstall(fake_apk_paths, fake_modules, package_name)
        apk_paths_to_install = [p for p in apk_paths if p not in fake_apk_paths]
      else:
        apk_paths_to_install = apk_paths
      self._InstallInternal(apk,
                            apk_paths_to_install,
                            allow_downgrade=allow_downgrade,
                            reinstall=reinstall,
                            permissions=permissions,
                            instant_app=instant_app,
                            force_queryable=force_queryable)

  @decorators.WithTimeoutAndRetriesFromInstance(
      min_default_timeout=INSTALL_DEFAULT_TIMEOUT)
  def InstallApex(self, apex, timeout=None, retries=None):
    """
    Installs a mainline module and manages rebooting the device. Can only be
    used from Android 10 onwards with devices that have the correct
    kernal support.

    Args:
      base_apk: The path to an apex file
      timeout: timeout in seconds
      retries: number of retries

    Raises:
      CommandFailedError if the installation fails
      DeviceVersionError if the device SDK level does not support
        mainline modules
    """

    self._CheckSdkLevel(version_codes.Q)

    apex = apk_helper.ToHelper(apex)
    package_name = apex.GetPackageName()
    version = apex.GetVersionCode()

    # Only one module can be installed at a time so
    # we use a lock to prevent two parallel InstallApex calls
    # because the device needs to reboot before it is safe to
    # install another module
    with self._apex_lock:
      with apex.GetApkPaths(self) as apex_file_paths:
        if len(apex_file_paths) != 1:
          raise device_errors.CommandFailedError(
              'Expected one apex path but received: %s' %
              pprint.pformat(apex_file_paths))

        apex_file_path = apex_file_paths[0]

        if not os.path.exists(apex_file_path):
          raise device_errors.CommandFailedError(
              'Attempted to install non-existent apex: %s' % apex_file_path)

        logger.info('Installing module %s using apex %s', package_name,
                    apex_file_path)

        try:
          self.adb.Install(apex_file_path)
        except device_errors.AdbCommandFailedError as adb_error:
          # If the device already has a module staged, it is in an unexpected
          # state We will throw an error to allow the developer to work out
          # how to deal with this
          # While ADB will already throw this, it will be a little harder
          # to debug what is happening
          if 'Cannot stage multiple sessions without checkpoint support' in str(
              adb_error):
            raise device_errors.CommandFailedError(
                'Apex module is already staged - the device must be restarted')
          # Even though we do a SDK version check, the device could still not
          # support APEX files because they have further kernal requirements
          # that aren't necessarily enforced in Android 10
          if "device doesn't support the installation of APEX" in str(
              adb_error):
            raise device_errors.CommandFailedError(
                'The device used does not support installing apex files')
          raise adb_error

      logger.info('Rebooting device')
      self.Reboot()

    if not self.IsSystemModuleInstalled(package_name, version):
      raise device_errors.CommandFailedError(
          'Module %s with version %s not installed on device after rebooting '
          'install attempt.' % (package_name, version))

    logger.info('Apex %s installed', package_name)

  @staticmethod
  def _GetFakeInstallPaths(apk_paths, fake_modules):
    def IsFakeModulePath(path):
      filename = os.path.basename(path)
      return any(filename.startswith(f + '-') for f in fake_modules)

    if not fake_modules:
      return set()
    return set(p for p in apk_paths if IsFakeModulePath(p))

  def _FakeInstall(self, fake_apk_paths, fake_modules, package_name):
    with tempfile_ext.NamedTemporaryDirectory() as modules_dir:
      tmp_dir = posixpath.join(self.MODULES_TMP_DIRECTORY_PATH, package_name)
      dest_dir = self.MODULES_LOCAL_TESTING_PATH_TEMPLATE.format(package_name)
      # Always clear MODULES_LOCAL_TESTING_PATH_TEMPLATE of stale files.
      if self.target_user is not None:
        # Convert to a path that is accessible by the system user
        dest_dir = self.ResolveSpecialPath(dest_dir)
      self.RunShellCommand(['rm', '-rf', dest_dir], as_root=True)
      if not fake_modules:
        return

      still_need_master = set(fake_modules)
      for path in fake_apk_paths:
        filename = os.path.basename(path)
        # Example names: base-en.apk, test_dummy-master.apk.
        module_name, suffix = filename.split('-', 1)
        if 'master' in suffix:
          assert module_name in still_need_master, (
              'Duplicate master apk file for %s' % module_name)
          still_need_master.remove(module_name)
          new_filename = '%s.apk' % module_name
        else:
          # |suffix| includes .apk extension.
          new_filename = '%s.config.%s' % (module_name, suffix)
        new_path = os.path.join(modules_dir, new_filename)
        os.rename(path, new_path)

      assert not still_need_master, (
          'Missing master apk file for %s' % still_need_master)
      self.PushChangedFiles([(modules_dir, tmp_dir)], delete_device_stale=True)
      # Make sure the destination dir exists since we want to copy the contents
      # of the temporary location to this dir. This indirection is necessary on
      # Android 11 emulator as there is a permission issue for the files under
      # /sdcard/Android/data.
      self.RunShellCommand(['mkdir', '-p', dest_dir], as_root=True)
      # Use cp instead of mv in case destinations are on different disks. Use
      # shell=True to use the * wild card so that the contents of tmp_dir not
      # the dir itself is copied into dest_dir.
      self.RunShellCommand('cp -a {}/* {}/'.format(tmp_dir, dest_dir),
                           shell=True,
                           as_root=True)

  @decorators.WithTimeoutAndRetriesFromInstance(
      min_default_timeout=INSTALL_DEFAULT_TIMEOUT)
  def InstallSplitApk(self,
                      base_apk,
                      split_apks,
                      allow_downgrade=False,
                      reinstall=False,
                      allow_cached_props=False,
                      permissions=None,
                      timeout=None,
                      retries=None,
                      instant_app=False,
                      force_queryable=False):
    """Install a split APK.

    Noop if all of the APK splits are already installed.

    Args:
      base_apk: An ApkHelper instance or string containing the path to the base
          APK.
      split_apks: A list of strings of paths of all of the APK splits.
      allow_downgrade: A boolean indicating if we should allow downgrades.
      reinstall: A boolean indicating if we should keep any existing app data.
      allow_cached_props: Whether to use cached values for device properties.
      permissions: Set of permissions to set. If not set, finds permissions with
          apk helper. To set no permissions, pass [].
      timeout: timeout in seconds
      retries: number of retries
      instant_app: A boolean that selects if the APK should be installed as an
          instant app or not. Instant apps are installed in a more
          restrictive execution environment. - Supported from SDK 29
      force_queryable: A boolean that allows the installed application to be
        queryable by all other applications regardless of if they have declared
        the package as queryable in their manifests - Supported from SDK 30

    Raises:
      CommandFailedError if the installation fails.
      CommandTimeoutError if the installation times out.
      DeviceUnreachableError on missing device.
      DeviceVersionError if device SDK is less than Android L.
      DeviceVersionError if the device SDK level does not support instant
        apps or forcing queryable
    """
    apk = apk_helper.ToSplitHelper(base_apk, split_apks)
    with apk.GetApkPaths(
        self, allow_cached_props=allow_cached_props) as apk_paths:
      self._InstallInternal(apk,
                            apk_paths,
                            reinstall=reinstall,
                            permissions=permissions,
                            allow_downgrade=allow_downgrade,
                            instant_app=instant_app,
                            force_queryable=force_queryable)

  def _InstallInternal(self,
                       apk,
                       apk_paths,
                       allow_downgrade=False,
                       reinstall=False,
                       permissions=None,
                       instant_app=False,
                       force_queryable=False):
    if not apk_paths:
      raise device_errors.CommandFailedError('Did not get any APKs to install')

    if len(apk_paths) > 1:
      self._CheckSdkLevel(version_codes.LOLLIPOP)

    missing_apks = [a for a in apk_paths if not os.path.exists(a)]
    if missing_apks:
      raise device_errors.CommandFailedError(
          'Attempted to install non-existent apks: %s' %
          pprint.pformat(missing_apks))

    package_name = apk.GetPackageName()
    device_apk_paths = self._GetApplicationPathsInternal(package_name)

    host_checksums = None
    if not device_apk_paths:
      apks_to_install = apk_paths
    elif len(device_apk_paths) > 1 and len(apk_paths) == 1:
      logger.warning(
          'Installing non-split APK when split APK was previously installed')
      apks_to_install = apk_paths
    elif len(device_apk_paths) == 1 and len(apk_paths) > 1:
      logger.warning(
          'Installing split APK when non-split APK was previously installed')
      apks_to_install = apk_paths
    else:
      try:
        apks_to_install, host_checksums = (self._ComputeStaleApks(
            package_name, apk_paths))
      except device_errors.CommandFailedError as e:
        logger.warning('Error calculating md5: %s', e)
        apks_to_install, host_checksums = apk_paths, None
      if apks_to_install and not reinstall:
        apks_to_install = apk_paths

    if device_apk_paths and not reinstall:
      if apks_to_install:
        logger.info('Uninstalling package %s', package_name)
        self.Uninstall(package_name)
      else:
        # Running adb uninstall clears the data, so to be consistent, we
        # explicitly clear it when skipping the uninstall.
        self.ClearApplicationState(package_name)

    if apks_to_install:
      # Assume that we won't know the resulting device state.
      self._cache['package_apk_paths'].pop(package_name, 0)
      self._cache['package_apk_checksums'].pop(package_name, 0)
      partial = package_name if len(apks_to_install) < len(apk_paths) else None
      streaming = None
      if self.product_name in _NO_STREAMING_DEVICE_LIST:
        streaming = False
      if (self.is_emulator
          and self.build_version_sdk in _NO_STREAMING_EMULATOR_API_LEVELS):
        streaming = False
      logger.info('Installing package %s using APKs %s',
                  package_name, apks_to_install)
      if len(apks_to_install) > 1 or partial:
        self.adb.InstallMultiple(apks_to_install,
                                 partial=partial,
                                 reinstall=reinstall,
                                 streaming=streaming,
                                 allow_downgrade=allow_downgrade,
                                 instant_app=instant_app,
                                 force_queryable=force_queryable)
      else:
        self.adb.Install(apks_to_install[0],
                         reinstall=reinstall,
                         streaming=streaming,
                         allow_downgrade=allow_downgrade,
                         instant_app=instant_app,
                         force_queryable=force_queryable)
    else:
      logger.info('Skipping installation of package %s', package_name)
      # Running adb install terminates running instances of the app, so to be
      # consistent, we explicitly terminate it when skipping the install.
      self.ForceStop(package_name)

    # There have been cases of APKs not being detected after being explicitly
    # installed, so perform a sanity check now and fail early if the
    # installation somehow failed.
    library_version = apk.GetLibraryVersion()
    if not self.IsApplicationInstalled(package_name, library_version):
      raise device_errors.CommandFailedError(
          'Package %s with version %s not installed on device after explicit '
          'install attempt.' % (package_name, library_version))

    if (permissions is None
        and self.build_version_sdk >= version_codes.MARSHMALLOW):
      permissions = apk.GetPermissions()
    self.GrantPermissions(package_name, permissions)
    # Upon success, we know the device checksums, but not their paths.
    if host_checksums is not None:
      self._cache['package_apk_checksums'][package_name] = host_checksums

  @decorators.WithTimeoutAndRetriesFromInstance()
  def Uninstall(self, package_name, keep_data=False, timeout=None,
                retries=None):
    """Remove the app |package_name| from the device.

    This is a no-op if the app is not already installed.

    Args:
      package_name: The package to uninstall.
      keep_data: (optional) Whether to keep the data and cache directories.
      timeout: Timeout in seconds.
      retries: Number of retries.

    Raises:
      CommandFailedError if the uninstallation fails.
      CommandTimeoutError if the uninstallation times out.
      DeviceUnreachableError on missing device.
    """
    installed = self._GetApplicationPathsInternal(package_name)
    if not installed:
      return
    # cached package paths are indeterminate due to system apps taking over
    # user apps after uninstall, so clear it
    self._cache['package_apk_paths'].pop(package_name, 0)
    self._cache['package_apk_checksums'].pop(package_name, 0)
    self.adb.Uninstall(package_name, keep_data)

  def _CheckSdkLevel(self, required_sdk_level):
    """Raises an exception if the device does not have the required SDK level.
    """
    if self.build_version_sdk < required_sdk_level:
      raise device_errors.DeviceVersionError(
          ('Requires SDK level %s, device is SDK level %s' %
           (required_sdk_level, self.build_version_sdk)),
          device_serial=self.serial)

  @decorators.WithTimeoutAndRetriesFromInstance()
  def RunShellCommand(self,
                      cmd,
                      shell=False,
                      check_return=False,
                      cwd=None,
                      env=None,
                      run_as=None,
                      as_root=False,
                      single_line=False,
                      large_output=False,
                      raw_output=False,
                      timeout=None,
                      retries=None,
                      encoding='utf8'):
    """Run an ADB shell command.

    The command to run |cmd| should be a sequence of program arguments
    (preferred) or a single string with a shell script to run.

    When |cmd| is a sequence, it is assumed to contain the name of the command
    to run followed by its arguments. In this case, arguments are passed to the
    command exactly as given, preventing any further processing by the shell.
    This allows callers to easily pass arguments with spaces or special
    characters without having to worry about quoting rules. Whenever possible,
    it is recomended to pass |cmd| as a sequence.

    When |cmd| is passed as a single string, |shell| should be set to True.
    The command will be interpreted and run by the shell on the device,
    allowing the use of shell features such as pipes, wildcards, or variables.
    Failing to set shell=True will issue a warning, but this will be changed
    to a hard failure in the future (see: catapult:#3242).

    This behaviour is consistent with that of command runners in cmd_helper as
    well as Python's own subprocess.Popen.

    TODO(crbug.com/1029769) Change the default of |check_return| to True when
    callers have switched to the new behaviour.

    Args:
      cmd: A sequence containing the command to run and its arguments, or a
        string with a shell script to run (should also set shell=True).
      shell: A boolean indicating whether shell features may be used in |cmd|.
      check_return: A boolean indicating whether or not the return code should
        be checked.
      cwd: The device directory in which the command should be run.
      env: The environment variables with which the command should be run.
      run_as: A string containing the package as which the command should be
        run.
      as_root: A boolean indicating whether the shell command should be run
        with root privileges.
      single_line: A boolean indicating if only a single line of output is
        expected.
      large_output: Uses a work-around for large shell command output. Without
        this large output will be truncated.
      raw_output: Whether to only return the raw output
          (no splitting into lines).
      timeout: timeout in seconds
      retries: number of retries
      encoding: the expected encoding when reading the large_output. No encoding
          when the value is None.

    Returns:
      If single_line is False, the output of the command as a list of lines,
      otherwise, a string with the unique line of output emmited by the command
      (with the optional newline at the end stripped).

    Raises:
      AdbCommandFailedError if check_return is True and the exit code of
        the command run on the device is non-zero.
      CommandFailedError if single_line is True but the output contains two or
        more lines.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """

    def env_quote(key, value):
      if not DeviceUtils._VALID_SHELL_VARIABLE.match(key):
        raise KeyError('Invalid shell variable name %r' % key)
      # using double quotes here to allow interpolation of shell variables
      return '%s=%s' % (key, cmd_helper.DoubleQuote(value))

    def run(cmd):
      return self.adb.Shell(cmd)

    def handle_check_return(cmd):
      try:
        return run(cmd)
      except device_errors.AdbCommandFailedError as exc:
        if check_return:
          raise
        return exc.output

    def handle_large_command(cmd):
      if len(cmd) < self._MAX_ADB_COMMAND_LENGTH:
        return handle_check_return(cmd)
      with device_temp_file.DeviceTempFile(self.adb, suffix='.sh') as script:
        self._WriteFileWithPush(script.name, cmd)
        logger.debug('Large shell command will be run from file: %s', cmd)
        return handle_check_return('sh %s' % script.name_quoted)

    def handle_large_output(cmd, large_output_mode):
      if large_output_mode:
        with device_temp_file.DeviceTempFile(self.adb) as large_output_file:
          large_output_cmd = '( %s )>%s 2>&1' % (cmd, large_output_file.name)
          logger.debug('Large output mode enabled. Will write output to '
                       'device and read results from file.')
          try:
            handle_large_command(large_output_cmd)
            return self.ReadFile(large_output_file.name,
                                 force_pull=True,
                                 encoding=encoding)
          except device_errors.AdbShellCommandFailedError as exc:
            output = self.ReadFile(large_output_file.name,
                                   force_pull=True,
                                   encoding=encoding)
            raise device_errors.AdbShellCommandFailedError(
                cmd, output, exc.status, exc.device_serial)
      else:
        try:
          return handle_large_command(cmd)
        except device_errors.AdbCommandFailedError as exc:
          if exc.status is None:
            logger.error(_FormatPartialOutputError(exc.output))
            logger.warning('Attempting to run in large_output mode.')
            logger.warning('Use RunShellCommand(..., large_output=True) for '
                           'shell commands that expect a lot of output.')
            return handle_large_output(cmd, True)
          raise

    if isinstance(cmd, six.string_types):
      if not shell:
        # TODO(crbug.com/1029769): Make this an error instead.
        logger.warning(
            'The command to run should preferably be passed as a sequence of'
            ' args. If shell features are needed (pipes, wildcards, variables)'
            ' clients should explicitly set shell=True.')
    else:
      cmd = ' '.join(cmd_helper.SingleQuote(s) for s in cmd)
    if env:
      env = ' '.join(env_quote(k, v) for k, v in env.items())
      cmd = '%s %s' % (env, cmd)
    if cwd:
      cmd = 'cd %s && %s' % (cmd_helper.SingleQuote(cwd), cmd)
    if run_as:
      cmd = 'run-as %s sh -c %s' % (cmd_helper.SingleQuote(run_as),
                                    cmd_helper.SingleQuote(cmd))
    if (as_root is _FORCE_SU) or (as_root and self.NeedsSU()):
      # "su -c sh -c" allows using shell features in |cmd|
      cmd = self._Su('sh -c %s' % cmd_helper.SingleQuote(cmd))

    output = handle_large_output(cmd, large_output)

    if raw_output:
      return output

    output = output.splitlines()
    if single_line:
      if not output:
        return ''
      if len(output) == 1:
        return output[0]
      msg = 'one line of output was expected, but got: %s'
      raise device_errors.CommandFailedError(msg % output, str(self))
    return output

  def _RunPipedShellCommand(self, script, **kwargs):
    PIPESTATUS_LEADER = 'PIPESTATUS: '

    script += '; echo "%s${PIPESTATUS[@]}"' % PIPESTATUS_LEADER
    kwargs.update(shell=True, check_return=True)
    output = self.RunShellCommand(script, **kwargs)
    pipestatus_line = output[-1]

    if not pipestatus_line.startswith(PIPESTATUS_LEADER):
      logger.error('Pipe exit statuses of shell script missing.')
      raise device_errors.AdbShellCommandFailedError(
          script, output, status=None, device_serial=self.serial)

    output = output[:-1]
    statuses = [
        int(s) for s in pipestatus_line[len(PIPESTATUS_LEADER):].split()
    ]
    if any(statuses):
      raise device_errors.AdbShellCommandFailedError(
          script, output, status=statuses, device_serial=self.serial)
    return output

  @decorators.WithTimeoutAndRetriesFromInstance()
  def KillAll(self,
              process_name,
              exact=False,
              signum=device_signal.SIGKILL,
              as_root=False,
              blocking=False,
              quiet=False,
              timeout=None,
              retries=None):
    """Kill all processes with the given name on the device.

    Args:
      process_name: A string containing the name of the process to kill.
      exact: A boolean indicating whether to kill all processes matching
             the string |process_name| exactly, or all of those which contain
             |process_name| as a substring. Defaults to False.
      signum: An integer containing the signal number to send to kill. Defaults
              to SIGKILL (9).
      as_root: A boolean indicating whether the kill should be executed with
               root privileges.
      blocking: A boolean indicating whether we should wait until all processes
                with the given |process_name| are dead.
      quiet: A boolean indicating whether to ignore the fact that no processes
             to kill were found.
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      The number of processes attempted to kill.

    Raises:
      CommandFailedError if no process was killed and |quiet| is False.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    processes = self.ListProcesses(process_name)
    if exact:
      processes = [p for p in processes if p.name == process_name]
    if not processes:
      if quiet:
        return 0
      raise device_errors.CommandFailedError(
          'No processes matching %r (exact=%r)' % (process_name, exact),
          str(self))

    logger.info('KillAll(%r, ...) attempting to kill the following:',
                process_name)
    for p in processes:
      logger.info('  %05d %s', p.pid, p.name)

    pids = set(p.pid for p in processes)
    cmd = ['kill', '-%d' % signum] + sorted(str(p) for p in pids)
    self.RunShellCommand(cmd, as_root=as_root, check_return=True)

    def all_pids_killed():
      pids_left = (p.pid for p in self.ListProcesses(process_name))
      return not pids.intersection(pids_left)

    if blocking:
      timeout_retry.WaitFor(all_pids_killed, wait_period=0.1)

    return len(pids)

  @decorators.WithTimeoutAndRetriesFromInstance()
  def StartActivity(self,
                    intent_obj,
                    blocking=False,
                    trace_file_name=None,
                    force_stop=False,
                    timeout=None,
                    retries=None):
    """Start package's activity on the device.

    Note for multi-user: when "--user" param is not specified,
      the "am start" command applies to current user.

    Args:
      intent_obj: An Intent object to send.
      blocking: A boolean indicating whether we should wait for the activity to
        finish launching.
      trace_file_name: If present, a string that both indicates that we want to
        profile the activity and contains the path to which the trace should be
        saved.
      force_stop: A boolean indicating whether we should stop the activity
        before starting it.
      timeout: timeout in seconds
      retries: number of retries

    Raises:
      CommandFailedError if the activity could not be started.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    cmd = ['am', 'start']
    if blocking:
      cmd.append('-W')
    if trace_file_name:
      cmd.extend(['--start-profiler', trace_file_name])
    if force_stop:
      cmd.append('-S')
    if self.target_user is not None:
      cmd.extend(['--user', str(self.target_user)])
    cmd.extend(intent_obj.am_args)
    for line in self.RunShellCommand(cmd, check_return=True):
      if line.startswith('Error:'):
        raise device_errors.CommandFailedError(line, str(self))

  @decorators.WithTimeoutAndRetriesFromInstance()
  def StartService(self, intent_obj, timeout=None, retries=None):
    """Start a service on the device.

    Note for multi-user: when "--user" param is not specified,
      the "am start-service" command applies to current user.

    Args:
      intent_obj: An Intent object to send describing the service to start.
      timeout: Timeout in seconds.
      retries: Number of retries

    Raises:
      CommandFailedError if the service could not be started.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    # For whatever reason, startservice was changed to start-service on O and
    # above.
    cmd = ['am', 'startservice']
    if self.build_version_sdk >= version_codes.OREO:
      cmd[1] = 'start-service'
    if self.target_user is not None:
      cmd.extend(['--user', str(self.target_user)])
    cmd.extend(intent_obj.am_args)
    for line in self.RunShellCommand(cmd, check_return=True):
      if line.startswith('Error:'):
        raise device_errors.CommandFailedError(line, str(self))

  @decorators.WithTimeoutAndRetriesFromInstance()
  def StartInstrumentation(self,
                           component,
                           finish=True,
                           raw=False,
                           extras=None,
                           timeout=None,
                           retries=None):
    """Start an instrumentation on the device.

    Note for multi-user: when "--user" param is not specified,
      the "am instrument" command applies to current user.

    Args:
      component: The component to run the instrumentation.
      finish: A boolean indicating if waiting for the instrumentation to finish.
      raw: A boolean indicating if printing raw results.
      extras: A dict mapping the testing options as key-value pairs.
      timeout: Timeout in seconds.
      retries: Number of retries

    Raises:
      CommandFailedError if the service could not be started.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    if extras is None:
      extras = {}

    cmd = ['am', 'instrument']
    if finish:
      cmd.append('-w')
    if raw:
      cmd.append('-r')
    for k, v in extras.items():
      cmd.extend(['-e', str(k), str(v)])
    if self.target_user is not None:
      cmd.extend(['--user', str(self.target_user)])
    cmd.append(component)

    # Store the package name in a shell variable to help the command stay under
    # the _MAX_ADB_COMMAND_LENGTH limit.
    package = component.split('/')[0]
    shell_snippet = 'p=%s;%s' % (package,
                                 cmd_helper.ShrinkToSnippet(cmd, 'p', package))
    return self.RunShellCommand(
        shell_snippet, shell=True, check_return=True, large_output=True)

  @decorators.WithTimeoutAndRetriesFromInstance()
  def BroadcastIntent(self, intent_obj, timeout=None, retries=None):
    """Send a broadcast intent.

    Note for multi-user: when "--user" param is not specified,
      the "am broadcast" command applies to all users.
    This param won't be added even when "target_user" is set.

    Args:
      intent: An Intent to broadcast.
      timeout: timeout in seconds
      retries: number of retries

    Raises:
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    cmd = ['am', 'broadcast'] + intent_obj.am_args
    self.RunShellCommand(cmd, check_return=True)

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetCurrentUser(self, cache=False, timeout=None, retries=None):
    """Return an integer representing the id of the current foreground user.

    Args:
      cache: Whether to use cached properties when available.
      timeout: timeout in seconds
      retries: number of retries

    Raises:
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    current_user = self._cache['current_user']
    if cache and current_user is not None:
      return current_user
    with self._cache_lock:
      # Android older than Nougat does not support get-current-user.
      # Use dumpsys instead.
      if self.build_version_sdk < version_codes.NOUGAT:
        current_user = self._GetCurrentUserDumpsys()
      else:
        cmd = ['am', 'get-current-user']
        # Only actual user id is extracted. Warning is skipped if it exists.
        current_user = int(self.RunShellCommand(cmd, check_return=True)[-1])
      self._cache['current_user'] = current_user
    return current_user

  @decorators.WithTimeoutAndRetriesFromInstance()
  def _GetCurrentUserDumpsys(self, timeout=None, retries=None):
    # mUserLru is a LRU list of history of current users.
    # Most recently current is at the end.
    lines = self._GetDumpsysOutput(['activity'], 'mUserLru:')
    for line in lines:
      m = _USER_LRU_RE.match(line)
      if m:
        user_ids = [user_id.strip() for user_id in m.group(1).split(',')]
        return int(user_ids[-1])
    raise device_errors.CommandFailedError(
        'mUserLru not found on dumpsys output')

  @decorators.WithTimeoutAndRetriesFromInstance()
  def ListUsers(self, timeout=None, retries=None):
    """List all the users with their userinfo on the device.

    Return a list of dict with the following keys:
      * id: User id as an integer
      * name: User name as a string
      * flags: User flags as an integer
    """
    users = []
    lines = self.RunShellCommand(['pm', 'list', 'users'], check_return=True)
    for line in lines:
      match = _USER_INFO_RE.match(line)
      if match:
        user_info = match.groupdict()
        user_info['id'] = int(user_info['id'])
        # flags from pm output is a hex string. Convert it to integer.
        user_info['flags'] = int(user_info['flags'], 16)
        users.append(user_info)
    return users

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetMainUser(self, timeout=None, retries=None):
    """Get the id of the main human user on the device.

    On devices with Headless System User Mode (hsum) enabled, i.e. Android
      Automotive OS, main user is the first human user.
    On non-hsum, main user is the system user (user 0).

    Such a user will have the admin permission, and may have access to certain
    features which are limited to at most one user.
    """
    users = self.ListUsers()
    # Since _USER_FLAG_MAIN is added in newer Android OS, if not found, fallback
    # to the user that has both _USER_FLAG_ADMIN and the _USER_FLAG_FULL.
    for flag_main in [_USER_FLAG_MAIN, (_USER_FLAG_ADMIN | _USER_FLAG_FULL)]:
      for user_info in users:
        if (user_info['flags'] & flag_main) == flag_main:
          return user_info['id']

    raise device_errors.CommandFailedError(
        f'Failed to find the main user from existing users {users}')

  @decorators.WithTimeoutAndRetriesFromInstance()
  def SwitchUser(self, user_id, timeout=None, retries=None):
    """Switch to user with the given user id and put the user in the foreground.

    Args:
      user_id: A specific user to switch to.
      timeout: timeout in seconds
      retries: number of retries

    Raises:
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    cmd = ['am', 'switch-user', str(user_id)]
    self.RunShellCommand(cmd, check_return=True)
    self._cache['current_user'] = None

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GoHome(self, timeout=None, retries=None):
    """Return to the home screen and obtain launcher focus.

    This command launches the home screen and attempts to obtain
    launcher focus until the timeout is reached.

    Args:
      timeout: timeout in seconds
      retries: number of retries

    Raises:
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """

    def is_launcher_focused():
      output = self.RunShellCommand(['dumpsys', 'activity', 'activities'],
                                    check_return=True,
                                    large_output=True)
      return any(self._RESUMED_LAUNCHER_ACTIVITY_RE.match(l) for l in output)

    def dismiss_popups():
      # There is a dialog present; attempt to get rid of it.
      # Not all dialogs can be dismissed with back.
      self.SendKeyEvent(keyevent.KEYCODE_ENTER)
      self.SendKeyEvent(keyevent.KEYCODE_BACK)
      return is_launcher_focused()

    # If Home is already focused, return early to avoid unnecessary work.
    if is_launcher_focused():
      return

    self.StartActivity(
        intent.Intent(
            action='android.intent.action.MAIN',
            category='android.intent.category.HOME'),
        blocking=True)

    if not is_launcher_focused():
      timeout_retry.WaitFor(dismiss_popups, wait_period=1)

  @decorators.WithTimeoutAndRetriesFromInstance()
  def Unlock(self, timeout=None, retries=None):
    """Wakes up the device screen and unlocks (if not PIN protected).

    This is a NOOP if the device screen is already unlocked.

    Args:
      timeout: timeout in seconds
      retries: number of retries

    Raises:
      CommandFailedError if device cannot be unlocked (ex. if PIN protected).
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    # Must wake up the screen before unlocking. This is a NOOP if already awake.
    self.SendKeyEvent(keyevent.KEYCODE_WAKEUP)

    def is_screen_locked():
      lines = self.RunShellCommand(['dumpsys', 'nfc'])
      screen_locked = False
      screen_locked_pattern = re.compile(r'mScreenState=.*\bON_LOCKED$')
      for line in lines:
        if screen_locked_pattern.match(line):
          screen_locked = True
          break
      return screen_locked

    if is_screen_locked():
      self.SendKeyEvent(keyevent.KEYCODE_MENU)
      if is_screen_locked():
        raise device_errors.CommandFailedError('Screen is still locked. Is the '
                                               'device password protected?')

  @decorators.WithTimeoutAndRetriesFromInstance()
  def ForceStop(self, package, timeout=None, retries=None):
    """Close the application.

    Note for multi-user: when "--user" param is not specified,
      the "am force-stop" command applies to all users.
    This param won't be added even when "target_user" is set.

    Args:
      package: A string containing the name of the package to stop.
      timeout: timeout in seconds
      retries: number of retries

    Raises:
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    if self.GetApplicationPids(package):
      self.RunShellCommand(['am', 'force-stop', package], check_return=True)

  @decorators.WithTimeoutAndRetriesFromInstance()
  def ClearApplicationState(self,
                            package,
                            permissions=None,
                            timeout=None,
                            retries=None,
                            wait_for_asynchronous_intent=False):
    """Clear all state for the given package.

    Note for multi-user: when "--user" param is not specified,
      the "pm clear" command applies to system user.

    Args:
      package: A string containing the name of the package to stop.
      permissions: List of permissions to set after clearing data.
      timeout: timeout in seconds
      retries: number of retries
      wait_for_asynchronous_intent: Wait for the asynchronous MediaProvider
          intent to finish before returning. This intent can end up deleting
          data after this function returns if not waited for.

    Raises:
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    # Check that the package exists before clearing it for android builds below
    # JB MR2. Necessary because calling pm clear on a package that doesn't exist
    # may never return.
    if ((self.build_version_sdk >= version_codes.JELLY_BEAN_MR2)
        or self._GetApplicationPathsInternal(package)):

      cmd = ['pm', 'clear']
      if self.target_user is not None:
        cmd.extend(['--user', str(self.target_user)])
      cmd.append(package)
      self.RunShellCommand(cmd, check_return=True)
      self.GrantPermissions(package, permissions)

      if wait_for_asynchronous_intent:

        def intent_test():
          # This command should block until any outstanding external media file
          # modification (in this case deletion) is finished.
          output = self.RunShellCommand([
              'content',
              'call',
              '--uri',
              'content://media/external/file',
              '--method',
              'wait_for_idle',
          ],
                                        check_return=True)
          # We check output instead of relying on check_return since the return
          # value appears to be 0 even if there's an error.
          output = ''.join(output)
          if 'Result: null' in output:
            return True
          logging.warning('Received unexpected wait_for_idle output %s', output)
          return False

        timeout_retry.WaitFor(intent_test, wait_period=30, max_tries=1)

  @decorators.WithTimeoutAndRetriesFromInstance()
  def SendKeyEvent(self, keycode, timeout=None, retries=None):
    """Sends a keycode to the device.

    See the devil.android.sdk.keyevent module for suitable keycode values.

    Args:
      keycode: A integer keycode to send to the device.
      timeout: timeout in seconds
      retries: number of retries

    Raises:
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    self.RunShellCommand(
        ['input', 'keyevent', format(keycode, 'd')], check_return=True)

  @decorators.WithTimeoutAndRetriesFromInstance(
      min_default_timeout=_FILE_TRANSFER_TIMEOUT)
  def PushChangedFiles(self,
                       host_device_tuples,
                       delete_device_stale=False,
                       timeout=None,
                       retries=None,
                       run_as=None,
                       as_root=False):
    """Push files to the device, skipping files that don't need updating.

    When a directory is pushed, it is traversed recursively on the host and
    all files in it are pushed to the device as needed.
    Additionally, if delete_device_stale option is True,
    files that exist on the device but don't exist on the host are deleted.

    Args:
      host_device_tuples: A list of (host_path, device_path) tuples, where
        |host_path| is an absolute path of a file or directory on the host
        that should be minimially pushed to the device, and |device_path| is
        an absolute path of the destination on the device.
      delete_device_stale: option to delete stale files on device
      timeout: timeout in seconds
      retries: number of retries
      run_as: A string containing the package as which the command should be
        run.
      as_root: A boolean indicating whether the shell command should be run
        with root privileges.

    Raises:
      CommandFailedError on failure.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    if self.target_user is not None:
      host_device_tuples = [(h, self.ResolveSpecialPath(d))
                            for h, d in host_device_tuples]

    # TODO(crbug.com/1005504): Experiment with this on physical devices after
    # upgrading devil's default adb beyond 1.0.39.
    # TODO(crbug.com/1020716): disabled as can result in extra directory.
    enable_push_sync = False

    if enable_push_sync:
      try:
        self._PushChangedFilesSync(host_device_tuples)
        return
      except device_errors.AdbVersionError as e:
        # If we don't meet the adb requirements, fall back to the previous
        # sync-unaware implementation.
        logging.warning(str(e))

    changed_files, missing_dirs, cache_commit_func = (self._GetChangedFiles(
        host_device_tuples, delete_device_stale))

    if changed_files:
      if missing_dirs:
        # Read dirs from temp file to avoid potential errors like
        # "Argument list too long" (crbug.com/1174331) when the list
        # is too long.
        with device_temp_file.DeviceTempFile(self.adb, suffix='.sh') as script:
          script_contents = _MKDIR_SCRIPT.format(dirs=' '.join(
              cmd_helper.SingleQuote(d) for d in missing_dirs))
          self.WriteFile(script.name, script_contents)
          self.RunShellCommand(['source', script.name],
                               check_return=True,
                               run_as=run_as,
                               as_root=as_root)
      self._PushFilesImpl(host_device_tuples, changed_files)
    cache_commit_func()

  def _PushChangedFilesSync(self, host_device_tuples):
    """Push changed files via `adb sync`.

    Args:
      host_device_tuples: Same as PushChangedFiles.
    """
    for h, d in host_device_tuples:
      for ph, pd, _ in _IterPushableComponents(h, d):
        self.adb.Push(ph, pd, sync=True)


  def _GetDeviceNodes(self, paths):
    """Get the set of all files and directories on the device contained within
    the provided list of paths, without recursively expanding directories.

    Args:
      paths: The list of paths for which to list files and directories.

    Returns:
      a set containing all files and directories contained within |paths| on the
      device.
    """
    nodes = set()
    paths = [p.replace(' ', r'\ ') for p in paths]
    command = _FILE_LIST_SCRIPT % ' '.join(paths)
    current_path = ""
    # We use shell=True to evaluate the command as a script through the shell,
    # otherwise RunShellCommand tries to interpret it as the name of a (non
    # existent) command to run.
    for line in self.RunShellCommand(command, shell=True, check_return=True):
      # If the line is an absolute path it's a directory, otherwise it's a file
      # within the most recent directory.
      if posixpath.isabs(line):
        current_path = line + '/'
      else:
        line = current_path + line
      nodes.add(line)

    return nodes

  def _GetChangedFiles(self, host_device_tuples, delete_stale=False):
    """Get files to push and delete.

    Args:
      host_device_tuples: a list of (host_files_path, device_files_path) tuples
        to find changed files from
      delete_stale: Whether to delete stale files

    Returns:
      a three-element tuple
      1st element: a list of (host_files_path, device_files_path) tuples to push
      2nd element: a list of missing device directories to mkdir
      3rd element: a cache commit function
    """
    # The fully expanded list of host/device tuples of files to push.
    file_tuples = []
    # All directories we're pushing files to.
    device_dirs_to_push_to = set()
    # All files and directories we expect to have on the device after pushing
    # files.
    expected_device_nodes = set()

    for h, d in host_device_tuples:
      assert os.path.isabs(h) and posixpath.isabs(d)
      h = os.path.realpath(h)
      host_path = h.rstrip('/')
      device_dir = d.rstrip('/')

      expected_device_nodes.add(device_dir)

      # Add all parent directories to the directories we expect to have so we
      # don't delete empty nested directories.
      parent = posixpath.dirname(device_dir)
      while parent and parent != '/':
        expected_device_nodes.add(parent)
        parent = posixpath.dirname(parent)

      if os.path.isdir(host_path):
        device_dirs_to_push_to.add(device_dir)
        for root, _, filenames in os.walk(host_path):
          # ignore hidden directories
          if os.path.sep + '.' in root:
            continue
          relative_dir = os.path.relpath(root, host_path).rstrip('.')
          device_path = posixpath.join(device_dir, relative_dir).rstrip('/')
          expected_device_nodes.add(device_path)
          device_dirs_to_push_to.add(device_path)
          files = (
            [posixpath.join(device_dir, relative_dir, f) for f in filenames])
          expected_device_nodes.update(files)
          file_tuples.extend(zip(
            (os.path.join(root, f) for f in filenames), files))
      else:
        device_dirs_to_push_to.add(posixpath.dirname(device_dir))
        file_tuples.append((host_path, device_dir))

    if file_tuples or delete_stale:
      current_device_nodes = self._GetDeviceNodes(device_dirs_to_push_to)
      nodes_to_delete = current_device_nodes - expected_device_nodes

    missing_dirs = device_dirs_to_push_to - current_device_nodes

    if not file_tuples:
      if delete_stale and nodes_to_delete:
        self.RemovePath(nodes_to_delete, force=True, recursive=True)
      return (host_device_tuples, missing_dirs, lambda: 0)

    possibly_stale_device_nodes = current_device_nodes - nodes_to_delete
    possibly_stale_tuples = (
      [t for t in file_tuples if t[1] in possibly_stale_device_nodes])

    def calculate_host_checksums():
      # Need to compute all checksums when caching.
      if self._enable_device_files_cache:
        return md5sum.CalculateHostMd5Sums([t[0] for t in file_tuples])
      return md5sum.CalculateHostMd5Sums([t[0] for t in possibly_stale_tuples])

    def calculate_device_checksums():
      paths = {t[1] for t in possibly_stale_tuples}
      if not paths:
        return dict()
      sums = dict()
      if self._enable_device_files_cache:
        paths_not_in_cache = set()
        for path in paths:
          cache_entry = self._cache['device_path_checksums'].get(path)
          if cache_entry:
            sums[path] = cache_entry
          else:
            paths_not_in_cache.add(path)
        paths = paths_not_in_cache
      sums.update(dict(md5sum.CalculateDeviceMd5Sums(paths, self)))
      if self._enable_device_files_cache:
        for path, checksum in sums.items():
          self._cache['device_path_checksums'][path] = checksum
      return sums
    try:
      host_checksums, device_checksums = reraiser_thread.RunAsync(
          (calculate_host_checksums, calculate_device_checksums))
    except device_errors.CommandFailedError as e:
      logger.warning('Error calculating md5: %s', e)
      return (host_device_tuples, set(), lambda: 0)

    up_to_date = set()

    for host_path, device_path in possibly_stale_tuples:
      device_checksum = device_checksums.get(device_path, None)
      host_checksum = host_checksums.get(host_path, None)
      if device_checksum == host_checksum and device_checksum is not None:
        up_to_date.add(device_path)
      else:
        nodes_to_delete.add(device_path)

    if delete_stale and nodes_to_delete:
      self.RemovePath(nodes_to_delete, force=True, recursive=True)

    to_push = (
        [t for t in file_tuples if t[1] not in up_to_date])

    def cache_commit_func():
      if not self._enable_device_files_cache:
        return
      for host_path, device_path in file_tuples:
        host_checksum = host_checksums.get(host_path, None)
        self._cache['device_path_checksums'][device_path] = host_checksum

    return (to_push, missing_dirs, cache_commit_func)

  def _ComputeDeviceChecksumsForApks(self, package_name):
    ret = self._cache['package_apk_checksums'].get(package_name)
    if ret is None:
      # TODO(hypan): Double check for multi-user
      if self.PathExists('/data/data/' + package_name, as_root=True):
        device_paths = self._GetApplicationPathsInternal(package_name)
        file_to_checksums = md5sum.CalculateDeviceMd5Sums(device_paths, self)
        ret = set(file_to_checksums.values())
      else:
        logger.info('Cannot reuse package %s (data directory missing)',
                    package_name)
        ret = set()
      self._cache['package_apk_checksums'][package_name] = ret
    return ret

  def _ComputeStaleApks(self, package_name, host_apk_paths):
    def calculate_host_checksums():
      return md5sum.CalculateHostMd5Sums(host_apk_paths)

    def calculate_device_checksums():
      return self._ComputeDeviceChecksumsForApks(package_name)

    host_checksums, device_checksums = reraiser_thread.RunAsync(
        (calculate_host_checksums, calculate_device_checksums))
    stale_apks = [
        k for (k, v) in host_checksums.items() if v not in device_checksums
    ]
    return stale_apks, set(host_checksums.values())

  def _PushFilesImpl(self, host_device_tuples, files):
    if not files:
      return

    # If the target_user is set to a secondary user, it will need root in order
    # to push to paths like user's /sdcard. But adb does not allow to
    # "push with su", so we force to push via zip.
    if self.target_user is not None:
      if not self._PushChangedFilesZipped(files,
                                          [d for _, d in host_device_tuples]):
        raise device_errors.CommandFailedError(
            'Failed to push changed files for user %s' % self.target_user,
            str(self))
      return

    size = sum(host_utils.GetRecursiveDiskUsage(h) for h, _ in files)
    file_count = len(files)
    dir_size = sum(
        host_utils.GetRecursiveDiskUsage(h) for h, _ in host_device_tuples)
    dir_file_count = 0
    for h, _ in host_device_tuples:
      if os.path.isdir(h):
        dir_file_count += sum(len(f) for _r, _d, f in os.walk(h))
      else:
        dir_file_count += 1

    push_duration = self._ApproximateDuration(file_count, file_count, size,
                                              False)
    dir_push_duration = self._ApproximateDuration(
        len(host_device_tuples), dir_file_count, dir_size, False)
    zip_duration = self._ApproximateDuration(1, 1, size, True)

    # TODO(https://crbug.com/1338098): Resume directory pushing once
    # clients have switched to 1.0.36-compatible syntax.
    # pylint: disable=condition-evals-to-constant
    if (dir_push_duration < push_duration and dir_push_duration < zip_duration
        and False):
      # pylint: enable=condition-evals-to-constant
      self._PushChangedFilesIndividually(host_device_tuples)
    elif push_duration < zip_duration:
      self._PushChangedFilesIndividually(files)
    elif self._commands_installed is False:
      # Already tried and failed to install unzip command.
      self._PushChangedFilesIndividually(files)
    elif not self._PushChangedFilesZipped(files,
                                          [d for _, d in host_device_tuples]):
      self._PushChangedFilesIndividually(files)

  def _MaybeInstallCommands(self):
    if self._commands_installed is None:
      try:
        if not install_commands.Installed(self):
          install_commands.InstallCommands(self)
        self._commands_installed = True
      except device_errors.CommandFailedError as e:
        logger.warning('unzip not available: %s', str(e))
        self._commands_installed = False
    return self._commands_installed

  @staticmethod
  def _ApproximateDuration(adb_calls, file_count, byte_count, is_zipping):
    # We approximate the time to push a set of files to a device as:
    #   t = c1 * a + c2 * f + c3 + b / c4 + b / (c5 * c6), where
    #     t: total time (sec)
    #     c1: adb call time delay (sec)
    #     a: number of times adb is called (unitless)
    #     c2: push time delay (sec)
    #     f: number of files pushed via adb (unitless)
    #     c3: zip time delay (sec)
    #     c4: zip rate (bytes/sec)
    #     b: total number of bytes (bytes)
    #     c5: transfer rate (bytes/sec)
    #     c6: compression ratio (unitless)

    # All of these are approximations.
    ADB_CALL_PENALTY = 0.1  # seconds
    ADB_PUSH_PENALTY = 0.01  # seconds
    ZIP_PENALTY = 2.0  # seconds
    ZIP_RATE = 10000000.0  # bytes / second
    TRANSFER_RATE = 2000000.0  # bytes / second
    COMPRESSION_RATIO = 2.0  # unitless

    adb_call_time = ADB_CALL_PENALTY * adb_calls
    adb_push_setup_time = ADB_PUSH_PENALTY * file_count
    if is_zipping:
      zip_time = ZIP_PENALTY + byte_count / ZIP_RATE
      transfer_time = byte_count / (TRANSFER_RATE * COMPRESSION_RATIO)
    else:
      zip_time = 0
      transfer_time = byte_count / TRANSFER_RATE
    return adb_call_time + adb_push_setup_time + zip_time + transfer_time

  def _PushChangedFilesIndividually(self, files):
    for h, d in files:
      self.adb.Push(h, d)

  def _PushChangedFilesZipped(self, files, dirs):
    if not self._MaybeInstallCommands():
      return False

    with tempfile_ext.NamedTemporaryDirectory() as working_dir:
      zip_path = os.path.join(working_dir, 'tmp.zip')
      try:
        zip_utils.WriteZipFile(zip_path, files)
      except zip_utils.ZipFailedError:
        return False

      logger.info('Pushing %d files via .zip of size %d', len(files),
                  os.path.getsize(zip_path))
      self.NeedsSU()
      with device_temp_file.DeviceTempFile(
          self.adb, suffix='.zip') as device_temp:
        self.adb.Push(zip_path, device_temp.name)

        with device_temp_file.DeviceTempFile(self.adb, suffix='.sh') as script:
          # Read dirs from temp file to avoid potential errors like
          # "Argument list too long" (crbug.com/1174331) when the list
          # is too long.
          script_contents = _UNZIP_AND_CHMOD_SCRIPT.format(
              bin_dir=install_commands.BIN_DIR,
              zip_file=device_temp.name,
              dirs=' '.join(cmd_helper.SingleQuote(d) for d in dirs))
          self.WriteFile(script.name, script_contents)
          self.RunShellCommand(['source', script.name],
                               check_return=True,
                               as_root=True)

    return True

  # TODO(crbug.com/1111556): remove this and migrate the callsite to
  # PathExists().
  @decorators.WithTimeoutAndRetriesFromInstance()
  def FileExists(self, device_path, timeout=None, retries=None):
    """Checks whether the given file exists on the device.

    Arguments are the same as PathExists.
    """
    return self.PathExists(device_path, timeout=timeout, retries=retries)

  @decorators.WithTimeoutAndRetriesFromInstance()
  def PathExists(self, device_paths, as_root=False, timeout=None, retries=None):
    """Checks whether the given path(s) exists on the device.

    Args:
      device_path: A string containing the absolute path to the file on the
                   device, or an iterable of paths to check.
      as_root: Whether root permissions should be use to check for the existence
               of the given path(s).
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      True if the all given paths exist on the device, False otherwise.

    Raises:
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    paths = device_paths
    if isinstance(paths, six.string_types):
      paths = (paths, )
    if not paths:
      return True
    cmd = ['test', '-e', paths[0]]
    for p in paths[1:]:
      cmd.extend(['-a', '-e', p])
    try:
      self.RunShellCommand(
          cmd,
          as_root=as_root,
          check_return=True,
          timeout=timeout,
          retries=retries)
      return True
    except device_errors.CommandFailedError:
      return False

  @decorators.WithTimeoutAndRetriesFromInstance()
  def RemovePath(self,
                 device_path,
                 force=False,
                 recursive=False,
                 as_root=False,
                 rename=False,
                 timeout=None,
                 retries=None):
    """Removes the given path(s) from the device.

    Args:
      device_path: A string containing the absolute path to the file on the
                   device, or an iterable of paths to check.
      force: Whether to remove the path(s) with force (-f).
      recursive: Whether to remove any directories in the path(s) recursively.
      as_root: Whether root permissions should be use to remove the given
               path(s).
      rename: Whether to rename the path(s) before removing to help avoid
            filesystem errors. See https://stackoverflow.com/questions/11539657
      timeout: timeout in seconds
      retries: number of retries
    """

    def _RenamePath(path):
      random_suffix = hex(random.randint(2**12, 2**16 - 1))[2:]
      dest = '%s-%s' % (path, random_suffix)
      try:
        self.RunShellCommand(['mv', path, dest],
                             as_root=as_root,
                             check_return=True)
        return dest
      except device_errors.AdbShellCommandFailedError:
        # If it couldn't be moved, just try rm'ing the original path instead.
        return path

    args = ['rm']
    if force:
      args.append('-f')
    if recursive:
      args.append('-r')
    if isinstance(device_path, six.string_types):
      args.append(device_path if not rename else _RenamePath(device_path))
    else:
      args.extend(
          device_path if not rename else [_RenamePath(p) for p in device_path])
    self.RunShellCommand(args, as_root=as_root, check_return=True)

  @contextlib.contextmanager
  def _CopyToReadableLocation(self, device_path):
    """Context manager to copy a file to a globally readable temp file.

    This uses root permission to copy a file to a globally readable named
    temporary file. The temp file is removed when this contextmanager is closed.

    Args:
      device_path: A string containing the absolute path of the file (on the
        device) to copy.
    Yields:
      The globally readable file object.
    """
    with device_temp_file.DeviceTempFile(self.adb) as device_temp:
      cmd = 'SRC=%s DEST=%s;cp "$SRC" "$DEST" && chmod 666 "$DEST"' % (
          cmd_helper.SingleQuote(device_path),
          cmd_helper.SingleQuote(device_temp.name))
      self.RunShellCommand(cmd, shell=True, as_root=True, check_return=True)
      yield device_temp

  @decorators.WithTimeoutAndRetriesFromInstance(
      min_default_timeout=_FILE_TRANSFER_TIMEOUT)
  def PullFile(self,
               device_path,
               host_path,
               as_root=False,
               timeout=None,
               retries=None):
    """Pull a file from the device.

    Args:
      device_path: A string containing the absolute path of the file to pull
                   from the device.
      host_path: A string containing the absolute path of the destination on
                 the host.
      as_root: Whether root permissions should be used to pull the file.
      timeout: timeout in seconds
      retries: number of retries

    Raises:
      CommandFailedError on failure.
      CommandTimeoutError on timeout.
    """
    # Create the base dir if it doesn't exist already
    dirname = os.path.dirname(host_path)
    if dirname and not os.path.exists(dirname):
      os.makedirs(dirname)
    if as_root and self.NeedsSU():
      if not self.PathExists(device_path, as_root=True):
        raise device_errors.CommandFailedError(
            '%r: No such file or directory' % device_path, str(self))
      with self._CopyToReadableLocation(device_path) as readable_temp_file:
        self.adb.Pull(readable_temp_file.name, host_path)
    else:
      self.adb.Pull(device_path, host_path)

  def _ReadFileWithPull(self, device_path, encoding='utf8', errors='replace'):
    try:
      d = tempfile.mkdtemp()
      host_temp_path = os.path.join(d, 'tmp_ReadFileWithPull')
      self.adb.Pull(device_path, host_temp_path)
      with open(host_temp_path, 'rb') as host_temp:
        file_content = host_temp.read()
        return (file_content if encoding is None
                else six.ensure_str(file_content, encoding, errors))
    finally:
      if os.path.exists(d):
        shutil.rmtree(d)

  @decorators.WithTimeoutAndRetriesFromInstance()
  def ReadFile(self,
               device_path,
               as_root=False,
               force_pull=False,
               timeout=None,
               retries=None,
               encoding='utf8',
               errors='replace'):
    """Reads the contents of a file from the device.

    Parameters |encoding| and |errors| are used in Python3 for decoding
    bytes to |str|. UTF8 encoding and errors handling scheme 'replace' are
    using by default. Return type is |str| by default.

    For read file as bytes instead of text |encoding=None| can be used
    in Python3. In Python2 this method return bytes always.

    Args:
      device_path: A string containing the absolute path of the file to read
                   from the device.
      as_root: A boolean indicating whether the read should be executed with
               root privileges.
      force_pull: A boolean indicating whether to force the operation to be
          performed by pulling a file from the device. The default is, when the
          contents are short, to retrieve the contents using cat instead.
      timeout: timeout in seconds
      retries: number of retries
      encoding: file encoding
      errors: encoding errors handling

    Returns:
      The contents of |device_path| as a string. Contents are intepreted using
      universal newlines, so the caller will see them encoded as '\n'. Also,
      all lines will be terminated.

    Raises:
      AdbCommandFailedError if the file can't be read.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """

    def get_size(path):
      return self.FileSize(path, as_root=as_root)

    # Reading by pulling is faster than first getting the file size and cat-ing,
    # so only read by cat when we need root.
    if as_root and self.NeedsSU():
      if (not force_pull
          and 0 < get_size(device_path) <= self._MAX_ADB_OUTPUT_LENGTH):
        return _JoinLines(
            self.RunShellCommand(['cat', device_path],
                                 as_root=as_root,
                                 check_return=True))
      with self._CopyToReadableLocation(device_path) as readable_temp_file:
        return self._ReadFileWithPull(readable_temp_file.name, encoding, errors)
    return self._ReadFileWithPull(device_path, encoding, errors)

  def _WriteFileWithPush(self, device_path, contents):
    with tempfile.NamedTemporaryFile(mode='w+') as host_temp:
      host_temp.write(contents)
      host_temp.flush()
      self.adb.Push(host_temp.name, device_path)

  @decorators.WithTimeoutAndRetriesFromInstance()
  def WriteFile(self,
                device_path,
                contents,
                as_root=False,
                force_push=False,
                timeout=None,
                retries=None,
                run_as=None):
    """Writes |contents| to a file on the device.

    Args:
      device_path: A string containing the absolute path to the file to write
          on the device.
      contents: A string containing the data to write to the device.
      as_root: A boolean indicating whether the write should be executed with
          root privileges (if available).
      force_push: A boolean indicating whether to force the operation to be
          performed by pushing a file to the device. The default is, when the
          contents are short, to pass the contents using a shell script instead.
      timeout: timeout in seconds
      retries: number of retries

    Raises:
      CommandFailedError if the file could not be written on the device.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    logger.debug('The following contents will be written to the file %s: %s',
                 device_path, contents)
    if not force_push and len(contents) < self._MAX_ADB_COMMAND_LENGTH:
      # If the contents are small, for efficieny we write the contents with
      # a shell command rather than pushing a file.
      cmd = 'echo -n %s > %s' % (cmd_helper.SingleQuote(contents),
                                 cmd_helper.SingleQuote(device_path))
      self.RunShellCommand(cmd,
                           shell=True,
                           as_root=as_root,
                           run_as=run_as,
                           check_return=True)
    elif as_root and self.NeedsSU():
      # Adb does not allow to "push with su", so we first push to a temp file
      # on a safe location, and then copy it to the desired location with su.
      with device_temp_file.DeviceTempFile(self.adb) as device_temp:
        self._WriteFileWithPush(device_temp.name, contents)
        # Here we need 'cp' rather than 'mv' because the temp and
        # destination files might be on different file systems (e.g.
        # on internal storage and an external sd card).
        self.RunShellCommand(['cp', device_temp.name, device_path],
                             as_root=True,
                             check_return=True)
    else:
      # If root is not needed, we can push directly to the desired location.
      self._WriteFileWithPush(device_path, contents)

  def _ParseLongLsOutput(self, device_path, as_root=False, **kwargs):
    """Run and scrape the output of 'ls -a -l' on a device directory."""
    device_path = posixpath.join(device_path, '')  # Force trailing '/'.
    output = self.RunShellCommand(['ls', '-a', '-l', device_path],
                                  as_root=as_root,
                                  check_return=True,
                                  env={'TZ': 'utc'},
                                  **kwargs)
    if output and output[0].startswith('total '):
      output.pop(0)  # pylint: disable=maybe-no-member

    entries = []
    for line in output:
      m = _LONG_LS_OUTPUT_RE.match(line)
      if m:
        if m.group('filename') not in ['.', '..']:
          item = m.groupdict()
          # A change in toybox is causing recent Android versions to escape
          # spaces in file names. Here we just unquote those spaces. If we
          # later find more essoteric characters in file names, a more careful
          # unquoting mechanism may be needed. But hopefully not.
          # See: https://goo.gl/JAebZj
          item['filename'] = item['filename'].replace('\\ ', ' ')
          entries.append(item)
      else:
        logger.info('Skipping: %s', line)

    return entries

  def ListDirectory(self, device_path, as_root=False, **kwargs):
    """List all files on a device directory.

    Mirroring os.listdir (and most client expectations) the resulting list
    does not include the special entries '.' and '..' even if they are present
    in the directory.

    Args:
      device_path: A string containing the path of the directory on the device
                   to list.
      as_root: A boolean indicating whether the to use root privileges to list
               the directory contents.
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      A list of filenames for all entries contained in the directory.

    Raises:
      AdbCommandFailedError if |device_path| does not specify a valid and
          accessible directory in the device.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    entries = self._ParseLongLsOutput(device_path, as_root=as_root, **kwargs)
    return [d['filename'] for d in entries]

  def StatDirectory(self, device_path, as_root=False, **kwargs):
    """List file and stat info for all entries on a device directory.

    Implementation notes: this is currently implemented by parsing the output
    of 'ls -a -l' on the device. Whether possible and convenient, we attempt to
    make parsing strict and return values mirroring those of the standard |os|
    and |stat| Python modules.

    Mirroring os.listdir (and most client expectations) the resulting list
    does not include the special entries '.' and '..' even if they are present
    in the directory.

    Args:
      device_path: A string containing the path of the directory on the device
                   to list.
      as_root: A boolean indicating whether the to use root privileges to list
               the directory contents.
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      A list of dictionaries, each containing the following keys:
        filename: A string with the file name.
        st_mode: File permissions, use the stat module to interpret these.
        st_nlink: Number of hard links (may be missing).
        st_owner: A string with the user name of the owner.
        st_group: A string with the group name of the owner.
        st_rdev_pair: Device type as (major, minior) (only if inode device).
        st_size: Size of file, in bytes (may be missing for non-regular files).
        st_mtime: Time of most recent modification, in seconds since epoch
          (although resolution is in minutes).
        symbolic_link_to: If entry is a symbolic link, path where it points to;
          missing otherwise.

    Raises:
      AdbCommandFailedError if |device_path| does not specify a valid and
          accessible directory in the device.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    entries = self._ParseLongLsOutput(device_path, as_root=as_root, **kwargs)
    for d in entries:
      for key, value in list(d.items()):
        if value is None:
          del d[key]  # Remove missing fields.
      d['st_mode'] = _ParseModeString(d['st_mode'])
      d['st_mtime'] = calendar.timegm(
          time.strptime(d['st_mtime'], _LS_DATE_FORMAT))
      for key in ['st_nlink', 'st_size', 'st_rdev_major', 'st_rdev_minor']:
        if key in d:
          d[key] = int(d[key])
      if 'st_rdev_major' in d and 'st_rdev_minor' in d:
        d['st_rdev_pair'] = (d.pop('st_rdev_major'), d.pop('st_rdev_minor'))
    return entries

  def StatPath(self, device_path, as_root=False, **kwargs):
    """Get the stat attributes of a file or directory on the device.

    Args:
      device_path: A string containing the path of a file or directory from
                   which to get attributes.
      as_root: A boolean indicating whether the to use root privileges to
               access the file information.
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      A dictionary with the stat info collected; see StatDirectory for details.

    Raises:
      CommandFailedError if device_path cannot be found on the device.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    dirname, filename = posixpath.split(posixpath.normpath(device_path))
    for entry in self.StatDirectory(dirname, as_root=as_root, **kwargs):
      if entry['filename'] == filename:
        return entry
    raise device_errors.CommandFailedError(
        'Cannot find file or directory: %r' % device_path, str(self))

  def FileSize(self, device_path, as_root=False, **kwargs):
    """Get the size of a file on the device.

    Note: This is implemented by parsing the output of the 'ls' command on
    the device. On some Android versions, when passing a directory or special
    file, the size is *not* reported and this function will throw an exception.

    Args:
      device_path: A string containing the path of a file on the device.
      as_root: A boolean indicating whether the to use root privileges to
               access the file information.
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      The size of the file in bytes.

    Raises:
      CommandFailedError if device_path cannot be found on the device, or
        its size cannot be determited for some reason.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    entry = self.StatPath(device_path, as_root=as_root, **kwargs)
    try:
      return entry['st_size']
    except KeyError:
      raise device_errors.CommandFailedError(
          'Could not determine the size of: %s' % device_path, str(self))

  @decorators.WithTimeoutAndRetriesFromInstance()
  def SetJavaAsserts(self, enabled, timeout=None, retries=None):
    """Enables or disables Java asserts.

    Args:
      enabled: A boolean indicating whether Java asserts should be enabled
               or disabled.
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      True if the device-side property changed and a restart is required as a
      result, False otherwise.

    Raises:
      CommandTimeoutError on timeout.
    """

    def find_property(lines, property_name):
      for index, line in enumerate(lines):
        if line.strip() == '':
          continue
        key_value = tuple(s.strip() for s in line.split('=', 1))
        if len(key_value) != 2:
          continue
        key, value = key_value
        if key == property_name:
          return index, value
      return None, ''

    new_value = 'all' if enabled else ''

    # First ensure the desired property is persisted.
    try:
      properties = self.ReadFile(self.LOCAL_PROPERTIES_PATH).splitlines()
    except device_errors.CommandFailedError:
      properties = []
    index, value = find_property(properties, self.JAVA_ASSERT_PROPERTY)
    if new_value != value:
      if new_value:
        new_line = '%s=%s' % (self.JAVA_ASSERT_PROPERTY, new_value)
        if index is None:
          properties.append(new_line)
        else:
          properties[index] = new_line
      else:
        assert index is not None  # since new_value == '' and new_value != value
        properties.pop(index)
      self.WriteFile(self.LOCAL_PROPERTIES_PATH, _JoinLines(properties))

    # Next, check the current runtime value is what we need, and
    # if not, set it and report that a reboot is required.
    value = self.GetProp(self.JAVA_ASSERT_PROPERTY)
    if new_value != value:
      self.SetProp(self.JAVA_ASSERT_PROPERTY, new_value)
      return True
    return False

  def GetLocale(self, cache=False):
    """Returns the locale setting on the device.

    Args:
      cache: Whether to use cached properties when available.
    Returns:
      A pair (language, country).
    """
    locale = self.GetProp('persist.sys.locale', cache=cache)
    if locale:
      if '-' not in locale:
        logging.error('Unparsable locale: %s', locale)
        return ('', '')  # Behave as if persist.sys.locale is undefined.
      return tuple(locale.split('-', 1))
    return (self.GetProp('persist.sys.language', cache=cache),
            self.GetProp('persist.sys.country', cache=cache))

  def GetLanguage(self, cache=False):
    """Returns the language setting on the device.

    DEPRECATED: Prefer GetLocale() instead.

    Args:
      cache: Whether to use cached properties when available.
    """
    return self.GetLocale(cache=cache)[0]

  def GetCountry(self, cache=False):
    """Returns the country setting on the device.

    DEPRECATED: Prefer GetLocale() instead.

    Args:
      cache: Whether to use cached properties when available.
    """
    return self.GetLocale(cache=cache)[1]

  @property
  def screen_density(self):
    """Returns the screen density of the device."""
    DPI_TO_DENSITY = {
        120: 'ldpi',
        160: 'mdpi',
        240: 'hdpi',
        320: 'xhdpi',
        480: 'xxhdpi',
        640: 'xxxhdpi',
    }
    return DPI_TO_DENSITY.get(self.pixel_density, 'tvdpi')

  @property
  def pixel_density(self):
    density = self.GetProp('ro.sf.lcd_density', cache=True)
    if not density:
      # It might be an emulator, try the qemu prop.
      density = self.GetProp('qemu.sf.lcd_density', cache=True)
    return int(density)

  @property
  def is_emulator(self):
    return _EMULATOR_RE.match(self.GetProp('ro.product.device', cache=True))

  @property
  def build_description(self):
    """Returns the build description of the system.

    For example:
      nakasi-user 4.4.4 KTU84P 1227136 release-keys
    """
    return self.GetProp('ro.build.description', cache=True)

  @property
  def build_fingerprint(self):
    """Returns the build fingerprint of the system.

    For example:
      google/nakasi/grouper:4.4.4/KTU84P/1227136:user/release-keys
    """
    return self.GetProp('ro.build.fingerprint', cache=True)

  @property
  def build_id(self):
    """Returns the build ID of the system (e.g. 'KTU84P')."""
    return self.GetProp('ro.build.id', cache=True)

  @property
  def build_product(self):
    """Returns the build product of the system (e.g. 'grouper')."""
    return self.GetProp('ro.build.product', cache=True)

  @property
  def build_system_root_image(self):
    """Returns the system_root_image property.

    This seems to indicate whether the device is using a system-as-root
    partition layout. See http://bit.ly/37F34sx for more info.
    """
    return self.GetProp('ro.build.system_root_image', cache=True)

  @property
  def build_type(self):
    """Returns the build type of the system (e.g. 'user')."""
    return self.GetProp('ro.build.type', cache=True)

  @property
  def build_version_sdk(self):
    """Returns the build version sdk of the system as a number (e.g. 19).

    For version code numbers see:
    http://developer.android.com/reference/android/os/Build.VERSION_CODES.html

    For named constants see devil.android.sdk.version_codes

    Raises:
      CommandFailedError if the build version sdk is not a number.
    """
    value = self.GetProp('ro.build.version.sdk', cache=True)
    try:
      return int(value)
    except ValueError:
      raise device_errors.CommandFailedError(
          'Invalid build version sdk: %r' % value)

  @property
  def tracing_path(self):
    """Returns the tracing path of the device for atrace."""
    return self.GetTracingPath()

  @property
  def product_cpu_abi(self):
    """Returns the product cpu abi of the device (e.g. 'armeabi-v7a').

    For supported ABIs, the return value will be one of the values defined in
    devil.android.ndk.abis.
    """
    return self.GetProp('ro.product.cpu.abi', cache=True)

  @property
  def product_cpu_abis(self):
    """Returns all product cpu abi of the device."""
    return self.GetProp('ro.product.cpu.abilist', cache=True).split(',')

  @property
  def product_model(self):
    """Returns the name of the product model (e.g. 'Nexus 7')."""
    return self.GetProp('ro.product.model', cache=True)

  @property
  def product_name(self):
    """Returns the product name of the device (e.g. 'nakasi')."""
    return self.GetProp('ro.product.name', cache=True)

  @property
  def product_board(self):
    """Returns the product board name of the device (e.g. 'shamu')."""
    return self.GetProp('ro.product.board', cache=True)

  def _EnsureCacheInitialized(self):
    """Populates cache token, runs getprop and fetches $EXTERNAL_STORAGE."""
    if self._cache['token']:
      return
    with self._cache_lock:
      if self._cache['token']:
        return
      # Change the token every time to ensure that it will match only the
      # previously dumped cache.
      token = str(uuid.uuid1())
      cmd = ('c=/data/local/tmp/cache_token;'
             'echo $EXTERNAL_STORAGE;'
             'cat $c 2>/dev/null||echo;'
             'echo "%s">$c &&' % token + 'getprop')
      output = self.RunShellCommand(
          cmd, shell=True, check_return=True, large_output=True)
      # Error-checking for this existing is done in GetExternalStoragePath().
      self._cache['external_storage'] = output[0]
      self._cache['prev_token'] = output[1]
      output = output[2:]

      prop_cache = self._cache['getprop']
      prop_cache.clear()
      for key, value in _GETPROP_RE.findall(''.join(output)):
        prop_cache[key] = value
      self._cache['token'] = token

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetTracingPath(self, timeout=None, retries=None):
    """Gets tracing path from the device.

    Args:
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      /sys/kernel/debug/tracing for device with debugfs mount support;
      /sys/kernel/tracing for device with tracefs support;
      /sys/kernel/debug/tracing if support can't be determined.

    Raises:
      CommandTimeoutError on timeout.
    """
    tracing_path = self._cache['tracing_path']
    if tracing_path:
      return tracing_path
    with self._cache_lock:
      tracing_path = '/sys/kernel/debug/tracing'
      try:
        lines = self.RunShellCommand(['mount'],
                                     check_return=True,
                                     timeout=timeout,
                                     retries=retries)
        if not any('debugfs' in line for line in lines):
          tracing_path = '/sys/kernel/tracing'
      except device_errors.AdbCommandFailedError:
        pass
      self._cache['tracing_path'] = tracing_path
    return tracing_path

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetProp(self, property_name, cache=False, timeout=None, retries=None):
    """Gets a property from the device.

    Args:
      property_name: A string containing the name of the property to get from
                     the device.
      cache: Whether to use cached properties when available.
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      The value of the device's |property_name| property.

    Raises:
      CommandTimeoutError on timeout.
    """
    assert isinstance(
        property_name,
        six.string_types), ("property_name is not a string: %r" % property_name)

    if cache:
      # It takes ~120ms to query a single property, and ~130ms to query all
      # properties. So, when caching we always query all properties.
      self._EnsureCacheInitialized()
    else:
      # timeout and retries are handled down at run shell, because we don't
      # want to apply them in the other branch when reading from the cache
      value = self.RunShellCommand(['getprop', property_name],
                                   single_line=True,
                                   check_return=True,
                                   timeout=timeout,
                                   retries=retries)
      self._cache['getprop'][property_name] = value
    # Non-existent properties are treated as empty strings by getprop.
    return self._cache['getprop'].get(property_name, '')

  @decorators.WithTimeoutAndRetriesFromInstance()
  def SetProp(self,
              property_name,
              value,
              check=False,
              timeout=None,
              retries=None):
    """Sets a property on the device.

    Args:
      property_name: A string containing the name of the property to set on
                     the device.
      value: A string containing the value to set to the property on the
             device.
      check: A boolean indicating whether to check that the property was
             successfully set on the device.
      timeout: timeout in seconds
      retries: number of retries

    Raises:
      CommandFailedError if check is true and the property was not correctly
        set on the device (e.g. because it is not rooted).
      CommandTimeoutError on timeout.
    """
    assert isinstance(
        property_name,
        six.string_types), ("property_name is not a string: %r" % property_name)
    assert isinstance(
        value,
        six.string_types), "value is not a string: %r" % value

    self.RunShellCommand(['setprop', property_name, value], check_return=True)
    prop_cache = self._cache['getprop']
    if property_name in prop_cache:
      del prop_cache[property_name]
    # TODO(crbug.com/1029772) remove the option and make the check mandatory,
    # but using a single shell script to both set- and getprop.
    if check and value != self.GetProp(property_name, cache=False):
      raise device_errors.CommandFailedError(
          'Unable to set property %r on the device to %r' % (property_name,
                                                             value), str(self))

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetABI(self, timeout=None, retries=None):
    """Gets the device main ABI.

    Args:
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      The device's main ABI name. For supported ABIs, the return value will be
      one of the values defined in devil.android.ndk.abis.

    Raises:
      CommandTimeoutError on timeout.
    """
    return self.GetProp('ro.product.cpu.abi', cache=True)

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetSupportedABIs(self, timeout=None, retries=None):
    """Gets all ABIs supported by the device.

    Args:
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      The device's supported ABIs list. For supported ABIs, the returned list
      will consist of the values defined in devil.android.ndk.abis.

    Raises:
      CommandTimeoutError on timeout.
    """
    supported_abis = self.GetProp('ro.product.cpu.abilist', cache=True)
    return [
        supported_abi for supported_abi in supported_abis.split(',')
        if supported_abi
    ]

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetFeatures(self, timeout=None, retries=None):
    """Returns the features supported on the device."""
    lines = self.RunShellCommand(['pm', 'list', 'features'], check_return=True)
    return [f[8:] for f in lines if f.startswith('feature:')]

  def _GetPsOutput(self, pattern):
    """Runs |ps| command on the device and returns its output,

    This private method abstracts away differences between Android verions for
    calling |ps|, and implements support for filtering the output by a given
    |pattern|, but does not do any output parsing.
    """
    try:
      ps_cmd = 'ps'
      # ps behavior was changed in Android O and above, http://crbug.com/686716
      if self.build_version_sdk >= version_codes.OREO:
        ps_cmd = 'ps -e'
      if pattern:
        return self._RunPipedShellCommand(
            '%s | grep -F %s' % (ps_cmd, cmd_helper.SingleQuote(pattern)))
      return self.RunShellCommand(ps_cmd.split(),
                                  check_return=True,
                                  large_output=True)
    except device_errors.AdbShellCommandFailedError as e:
      if e.status and isinstance(e.status, list) and not e.status[0]:
        # If ps succeeded but grep failed, there were no processes with the
        # given name.
        return []
      raise

  @decorators.WithTimeoutAndRetriesFromInstance()
  def ListProcesses(self, process_name=None, timeout=None, retries=None):
    """Returns a list of tuples with info about processes on the device.

    This essentially parses the output of the |ps| command into convenient
    ProcessInfo tuples.

    Args:
      process_name: A string used to filter the returned processes. If given,
                    only processes whose name have this value as a substring
                    will be returned.
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      A list of ProcessInfo tuples with |name|, |pid|, and |ppid| fields.
    """
    # pylint: disable=broad-except
    process_name = process_name or ''
    processes = []
    for line in self._GetPsOutput(process_name):
      row = line.split()
      try:
        row = {k: row[i] for k, i in _PS_COLUMNS.items()}
        if row['pid'] == 'PID' or process_name not in row['name']:
          # Skip over header and non-matching processes.
          continue
        row['pid'] = int(row['pid'])
        row['ppid'] = int(row['ppid'])
      except Exception:  # e.g. IndexError, TypeError, ValueError.
        logging.warning('failed to parse ps line: %r', line)
        continue
      processes.append(ProcessInfo(**row))
    return processes

  def _GetSettings(self, namespace):
    """Return a dictionary containing global settings

    Note for multi-user: when "--user" param is not specified,
      the "settings list" command applies to current user.

    Args:
      namespace: Category of settings. Can be either 'system', 'global'
        or 'secure'.

    Returns:
      A dictionary mapping settings to their values.
    """
    if namespace not in (SettingsNamespace.SECURE, SettingsNamespace.GLOBAL,
                         SettingsNamespace.SYSTEM):
      raise ValueError('Unsupported namespace: %s' % namespace)
    cmd = ['settings', 'list']
    if self.target_user is not None:
      cmd.extend(['--user', str(self.target_user)])
    cmd.append(namespace)
    output_lines = self.RunShellCommand(cmd,
                                        check_return=True,
                                        large_output=True)
    return dict(map(lambda line: line.split('=', 1), output_lines))

  def _GetDumpsysOutput(self, extra_args, pattern=None):
    """Runs |dumpsys| command on the device and returns its output.

    This private method implements support for filtering the output by a given
    |pattern|, but does not do any output parsing.
    """
    try:
      cmd = ['dumpsys'] + extra_args
      if pattern:
        cmd = ' '.join(cmd_helper.SingleQuote(s) for s in cmd)
        return self._RunPipedShellCommand(
            '%s | grep -F %s' % (cmd, cmd_helper.SingleQuote(pattern)))
      cmd = ['dumpsys'] + extra_args
      return self.RunShellCommand(cmd, check_return=True, large_output=True)
    except device_errors.AdbShellCommandFailedError as e:
      if e.status and isinstance(e.status, list) and not e.status[0]:
        # If dumpsys succeeded but grep failed, there were no lines matching
        # the given pattern.
        return []
      raise

  def GetUidForPackage(self, package_name):
    """Get user id for package name on device

    Args:
      package_name: Package name installed on device

    Returns:
      A string containing the package UID, and if the package
      is not installed then None

    Raises:
      CommandFailedError if dumpsys does not return any output
    """
    return self._GetPackageDetailFromDumpsys(package_name, 'userId=')

  @decorators.WithTimeoutAndRetriesFromInstance()
  def _GetPackageDetailFromDumpsys(self,
                                   package_name,
                                   pattern,
                                   timeout=None,
                                   retries=None):
    """Get detail for a package installed on device from it's dumpsys

    Args:
      package_name: Package name installed on device
      pattern: Pattern for the detail to get from the dumpsys

    Returns:
      A string containing the detail.

    Raises:
      CommandFailedError if dumpsys does not return any output
    """
    dumpsys_output = self._GetDumpsysOutput(['package', package_name], pattern)

    if not dumpsys_output:
      raise device_errors.CommandFailedError(
          'No output was received from dumpsys')

    return (re.compile('.*{}'.format(pattern)).sub('', dumpsys_output[0])
            or None)

  # TODO(#4103): Remove after migrating clients to ListProcesses.
  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetPids(self, process_name=None, timeout=None, retries=None):
    """Returns the PIDs of processes containing the given name as substring.

    DEPRECATED

    Note that the |process_name| is often the package name.

    Args:
      process_name: A string containing the process name to get the PIDs for.
                    If missing returns PIDs for all processes.
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      A dict mapping process name to a list of PIDs for each process that
      contained the provided |process_name|.

    Raises:
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    procs_pids = collections.defaultdict(list)
    for p in self.ListProcesses(process_name):
      procs_pids[p.name].append(str(p.pid))
    return procs_pids

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetApplicationPids(self,
                         process_name,
                         at_most_one=False,
                         timeout=None,
                         retries=None):
    """Returns the PID or PIDs of a given process name.

    Note that the |process_name|, often the package name, must match exactly.

    Args:
      process_name: A string containing the process name to get the PIDs for.
      at_most_one: A boolean indicating that at most one PID is expected to
                   be found.
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      A list of the PIDs for the named process. If at_most_one=True returns
      the single PID found or None otherwise.

    Raises:
      CommandFailedError if at_most_one=True and more than one PID is found
          for the named process.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    pids = [
        p.pid for p in self.ListProcesses(process_name)
        if p.name == process_name
    ]
    if at_most_one:
      if len(pids) > 1:
        raise device_errors.CommandFailedError(
            'Expected a single PID for %r but found: %r.' % (process_name,
                                                             pids),
            device_serial=str(self))
      return pids[0] if pids else None
    return pids

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetEnforce(self, timeout=None, retries=None):
    """Get the current mode of SELinux.

    Args:
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      True (enforcing), False (permissive), or None (disabled).

    Raises:
      CommandFailedError on failure.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    output = self.RunShellCommand(['getenforce'],
                                  check_return=True,
                                  single_line=True).lower()
    if output not in _SELINUX_MODE:
      raise device_errors.CommandFailedError(
          'Unexpected getenforce output: %s' % output)
    return _SELINUX_MODE[output]

  @decorators.WithTimeoutAndRetriesFromInstance()
  def SetEnforce(self, enabled, timeout=None, retries=None):
    """Modify the mode SELinux is running in.

    Args:
      enabled: a boolean indicating whether to put SELinux in encorcing mode
               (if True), or permissive mode (otherwise).
      timeout: timeout in seconds
      retries: number of retries

    Raises:
      CommandFailedError on failure.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    self.RunShellCommand(['setenforce', '1' if int(enabled) else '0'],
                         as_root=True,
                         check_return=True)

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetWebViewProvider(self, timeout=None, retries=None):
    """Returns the webview_provider setting from global settings.
    More information on WebView providers can be found at
    //android_webview/docs/webview-providers.md

    Args:
      timeout: Timeout for method.
      retries: Number of maximum retries for the function if the
        function fails

    Returns:
      The value for the webview_provider setting in global settings."""
    self._CheckSdkLevel(version_codes.NOUGAT)
    return self._GetSettings(SettingsNamespace.GLOBAL).get('webview_provider')

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GetWebViewUpdateServiceDump(self, timeout=None, retries=None):
    """Get the WebView update command sysdump on the device.

    Returns:
      A dictionary with these possible entries:
        FallbackLogicEnabled: True|False
        CurrentWebViewPackage: "package name" or None
        CurrentWebViewVersion: Version of the current WebView provider
        MinimumWebViewVersionCode: int
        WebViewPackages: Dict of installed WebView providers, mapping "package
            name" to "reason it's valid/invalid."

    The returned dictionary may not include all of the above keys: this depends
    on the support of the platform's underlying WebViewUpdateService. This may
    return an empty dictionary on OS versions which do not support querying the
    WebViewUpdateService.

    Raises:
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    result = {}

    # Command was implemented starting in Oreo
    if self.build_version_sdk < version_codes.OREO:
      return result

    output = self.RunShellCommand(['dumpsys', 'webviewupdate'],
                                  check_return=True)
    webview_packages = {}
    for line in output:
      match = re.search(_WEBVIEW_SYSUPDATE_CURRENT_PKG_RE, line)
      if match:
        result['CurrentWebViewPackage'] = match.group(1)
        result['CurrentWebViewVersion'] = match.group(2)
      match = re.search(_WEBVIEW_SYSUPDATE_NULL_PKG_RE, line)
      if match:
        result['CurrentWebViewPackage'] = None
      match = re.search(_WEBVIEW_SYSUPDATE_FALLBACK_LOGIC_RE, line)
      if match:
        result['FallbackLogicEnabled'] = match.group(1) == 'true'
      match = re.search(_WEBVIEW_SYSUPDATE_PACKAGE_INSTALLED_RE, line)
      if match:
        package_name = match.group(1)
        reason = match.group(2)
        webview_packages[package_name] = reason
      match = re.search(_WEBVIEW_SYSUPDATE_PACKAGE_NOT_INSTALLED_RE, line)
      if match:
        package_name = match.group(1)
        reason = match.group(2)
        webview_packages[package_name] = reason
      match = re.search(_WEBVIEW_SYSUPDATE_MIN_VERSION_CODE, line)
      if match:
        result['MinimumWebViewVersionCode'] = int(match.group(1))
    if webview_packages:
      result['WebViewPackages'] = webview_packages
    return result

  @decorators.WithTimeoutAndRetriesFromInstance()
  def SetWebViewImplementation(self, package_name, timeout=None, retries=None):
    """Select the WebView implementation to the specified package.

    Args:
      package_name: The package name of a WebView implementation. The package
        must be already installed on the device.
      timeout: timeout in seconds
      retries: number of retries

    Raises:
      CommandFailedError on failure.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    if not self.IsApplicationInstalled(package_name):
      raise device_errors.CommandFailedError(
          '%s is not installed' % package_name, str(self))
    output = self.RunShellCommand(
        ['cmd', 'webviewupdate', 'set-webview-implementation', package_name],
        single_line=True,
        check_return=False)
    if output == 'Success':
      logging.info('WebView provider set to: %s', package_name)
    else:
      dumpsys_output = self.GetWebViewUpdateServiceDump()
      webview_packages = dumpsys_output.get('WebViewPackages')
      if webview_packages:
        reason = webview_packages.get(package_name)
        if not reason:
          all_provider_package_names = webview_packages.keys()
          raise device_errors.CommandFailedError(
              '%s is not in the system WebView provider list. Must choose one '
              'of %r.' % (package_name, all_provider_package_names), str(self))
        if re.search(r'is\s+NOT\s+installed/enabled for all users', reason):
          raise device_errors.CommandFailedError(
              '%s is disabled, make sure to disable WebView fallback logic' %
              package_name, str(self))
        if re.search(r'No WebView-library manifest flag', reason):
          raise device_errors.CommandFailedError(
              '%s does not declare a WebView native library, so it cannot '
              'be a WebView provider' % package_name, str(self))
        if re.search(r'SDK version too low', reason):
          app_target_sdk_version = self.GetApplicationTargetSdk(package_name)
          is_preview_sdk = self.GetProp('ro.build.version.preview_sdk') == '1'
          if is_preview_sdk:
            codename = self.GetProp('ro.build.version.codename')
            raise device_errors.CommandFailedError(
                '%s targets a finalized SDK (%r), but valid WebView providers '
                'must target a pre-finalized SDK (%r) on this device' %
                (package_name, app_target_sdk_version, codename), str(self))
          raise device_errors.CommandFailedError(
              '%s has targetSdkVersion %r, but valid WebView providers must '
              'target >= %r on this device' %
              (package_name, app_target_sdk_version, self.build_version_sdk),
              str(self))
        if re.search(r'Version code too low', reason):
          raise device_errors.CommandFailedError(
              '%s needs a higher versionCode (must be >= %d)' %
              (package_name, dumpsys_output.get('MinimumWebViewVersionCode')),
              str(self))
        if re.search(r'Incorrect signature', reason):
          raise device_errors.CommandFailedError(
              '%s is not signed with release keys (but user builds require '
              'this for WebView providers)' % package_name, str(self))
      raise device_errors.CommandFailedError(
          'Error setting WebView provider: %s' % output, str(self))

  @decorators.WithTimeoutAndRetriesFromInstance()
  def SetWebViewFallbackLogic(self, enabled, timeout=None, retries=None):
    """Set whether WebViewUpdateService's "fallback logic" should be enabled.

    WebViewUpdateService has nonintuitive "fallback logic" for devices where
    Monochrome (Chrome Stable) is preinstalled as the WebView provider, with a
    "stub" (little-to-no code) implementation of standalone WebView.

    "Fallback logic" (enabled by default) is designed, in the case where the
    user has disabled Chrome, to fall back to the stub standalone WebView by
    enabling the package. The implementation plumbs through the Chrome APK until
    Play Store installs an update with the full implementation.

    A surprising side-effect of "fallback logic" is that, immediately after
    sideloading WebView, WebViewUpdateService re-disables the package and
    uninstalls the update. This can prevent successfully using standalone
    WebView for development, although "fallback logic" can be disabled on
    userdebug/eng devices.

    Because this is only relevant for devices with the standalone WebView stub,
    this command is only relevant on N-P (inclusive).

    You can determine if "fallback logic" is currently enabled by checking
    FallbackLogicEnabled in the dictionary returned by
    GetWebViewUpdateServiceDump.

    Args:
      enabled: bool - True for enabled, False for disabled
      timeout: timeout in seconds
      retries: number of retries

    Raises:
      CommandFailedError on failure.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """

    # Command is only available on devices which preinstall stub WebView.
    if not version_codes.NOUGAT <= self.build_version_sdk <= version_codes.PIE:
      return

    # redundant-packages is the opposite of fallback logic
    enable_string = 'disable' if enabled else 'enable'
    output = self.RunShellCommand(
        ['cmd', 'webviewupdate',
         '%s-redundant-packages' % enable_string],
        single_line=True,
        check_return=True)
    if output == 'Success':
      logging.info('WebView Fallback Logic is %s',
                   'enabled' if enabled else 'disabled')
    else:
      raise device_errors.CommandFailedError(
          'Error setting WebView Fallback Logic: %s' % output, str(self))

  @decorators.WithTimeoutAndRetriesFromInstance()
  def TakeScreenshot(self, host_path=None, timeout=None, retries=None):
    """Takes a screenshot of the device.

    Args:
      host_path: A string containing the path on the host to save the
                 screenshot to. If None, a file name in the current
                 directory will be generated.
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      The name of the file on the host to which the screenshot was saved.

    Raises:
      CommandFailedError on failure.
      CommandTimeoutError on timeout.
      DeviceUnreachableError on missing device.
    """
    if not host_path:
      host_path = os.path.abspath(
          'screenshot-%s-%s.png' % (self.serial, _GetTimeStamp()))
    with device_temp_file.DeviceTempFile(self.adb, suffix='.png') as device_tmp:
      # For whatever reason, certain devices can hang when specifying a file to
      # screencap, but work fine when redirecting output to a file.
      # See crbug.com/1446736.
      if self.product_name in _ALTERNATE_SCREENSHOT_CMD_DEVICES:
        cmd = '/system/bin/screencap -p > %s' % device_tmp.name
        use_shell = True
      else:
        cmd = ['/system/bin/screencap', '-p', device_tmp.name]
        use_shell = False
      self.RunShellCommand(cmd, check_return=True, shell=use_shell)
      self.PullFile(device_tmp.name, host_path)
    return host_path

  @decorators.WithTimeoutAndRetriesFromInstance()
  def DismissCrashDialogIfNeeded(self, timeout=None, retries=None):
    """Dismiss the error/ANR dialog if present.

    Returns: Name of the crashed package if a dialog is focused,
             None otherwise.
    """

    def _FindFocusedWindow():
      match = None
      # Note: This will fail to find system dialogs on Android Q+. System
      # dialogs should not be shown on Android Q+ because we set
      # hide_error_dialogs=1. http://crbug.com/1107896#c26
      # TODO(jbudorick): Try to grep the output on the device instead of using
      # large_output if/when DeviceUtils exposes a public interface for piped
      # shell command handling.
      for line in self.RunShellCommand(['dumpsys', 'window', 'windows'],
                                       check_return=True,
                                       large_output=True):
        match = re.match(_CURRENT_FOCUS_CRASH_RE, line)
        if match:
          break
      return match

    match = _FindFocusedWindow()
    if not match:
      return None
    package = match.group(2)
    logger.warning('Trying to dismiss %s dialog for %s', *match.groups())

    if self.build_version_sdk >= version_codes.NOUGAT:
      # Broadcast does not work pre-N. Send broadcast because Android N+
      # sometimes displays system dialog where only option is "Open app Again"
      # when app crashes.
      self.BroadcastIntent(
          intent.Intent(action='android.intent.action.CLOSE_SYSTEM_DIALOGS'))
    else:
      self.SendKeyEvent(keyevent.KEYCODE_DPAD_RIGHT)
      self.SendKeyEvent(keyevent.KEYCODE_DPAD_RIGHT)
      self.SendKeyEvent(keyevent.KEYCODE_ENTER)

    match = _FindFocusedWindow()
    if match:
      logger.error('Still showing a %s dialog for %s', *match.groups())
    return package

  def GetLogcatMonitor(self, *args, **kwargs):
    """Returns a new LogcatMonitor associated with this device.

    Parameters passed to this function are passed directly to
    |logcat_monitor.LogcatMonitor| and are documented there.
    """
    return logcat_monitor.LogcatMonitor(self.adb, *args, **kwargs)

  def GetClientCache(self, client_name):
    """Returns client cache."""
    if client_name not in self._client_caches:
      self._client_caches[client_name] = {}
    return self._client_caches[client_name]

  def ClearCache(self):
    """Clears all caches."""
    for client in self._client_caches:
      self._client_caches[client].clear()
    self._cache = {
        # Map of packageId -> list of on-device .apk paths
        'package_apk_paths': {},
        # Set of packageId that were loaded from LoadCacheData and not yet
        # verified.
        'package_apk_paths_to_verify': set(),
        # Map of packageId -> set of on-device .apk checksums
        'package_apk_checksums': {},
        # Map of property_name -> value
        'getprop': {},
        # Map of device path -> checksum]
        'device_path_checksums': {},
        # Location of sdcard ($EXTERNAL_STORAGE).
        'external_storage': None,
        # Token used to detect when LoadCacheData is stale.
        'token': None,
        'prev_token': None,
        # Path for tracing.
        'tracing_path': None,
        # The id of the current foreground user.
        'current_user': None,
    }

  @decorators.WithTimeoutAndRetriesFromInstance()
  def LoadCacheData(self, data, timeout=None, retries=None):
    """Initializes the cache from data created using DumpCacheData.

    The cache is used only if its token matches the one found on the device.
    This prevents a stale cache from being used (which can happen when sharing
    devices).

    Args:
      data: A previously serialized cache (string).
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      Whether the cache was loaded.
    """
    try:
      obj = json.loads(data)
    except ValueError:
      logger.error('Unable to parse cache file. Not using it.')
      return False
    self._EnsureCacheInitialized()
    given_token = obj.get('token')
    if not given_token or self._cache['prev_token'] != given_token:
      logger.warning('Stale cache detected. Not using it.')
      return False

    self._cache['package_apk_paths'] = obj.get('package_apk_paths', {})
    # When using a cache across script invokations, verify that apps have
    # not been uninstalled.
    self._cache['package_apk_paths_to_verify'] = set(
        self._cache['package_apk_paths'])

    package_apk_checksums = obj.get('package_apk_checksums', {})
    for k, v in package_apk_checksums.items():
      package_apk_checksums[k] = set(v)
    self._cache['package_apk_checksums'] = package_apk_checksums
    device_path_checksums = obj.get('device_path_checksums', {})
    self._cache['device_path_checksums'] = device_path_checksums
    return True

  @decorators.WithTimeoutAndRetriesFromInstance()
  def DumpCacheData(self, timeout=None, retries=None):
    """Dumps the current cache state to a string.

    Args:
      timeout: timeout in seconds
      retries: number of retries

    Returns:
      A serialized cache as a string.
    """
    self._EnsureCacheInitialized()
    obj = {}
    obj['token'] = self._cache['token']
    obj['package_apk_paths'] = self._cache['package_apk_paths']
    obj['package_apk_checksums'] = self._cache['package_apk_checksums']
    # JSON can't handle sets.
    for k, v in obj['package_apk_checksums'].items():
      obj['package_apk_checksums'][k] = list(v)
    obj['device_path_checksums'] = self._cache['device_path_checksums']
    return json.dumps(obj, separators=(',', ':'))

  @classmethod
  def parallel(cls, devices, asyn=False):
    """Creates a Parallelizer to operate over the provided list of devices.

    Args:
      devices: A list of either DeviceUtils instances or objects from
               from which DeviceUtils instances can be constructed. If None,
               all attached devices will be used.
      asyn: If true, returns a Parallelizer that runs operations
             asynchronously.

    Returns:
      A Parallelizer operating over |devices|.
    """
    devices = [d if isinstance(d, cls) else cls(d) for d in devices]
    if asyn:
      return parallelizer.Parallelizer(devices)
    return parallelizer.SyncParallelizer(devices)

  @classmethod
  def HealthyDevices(cls,
                     denylist=None,
                     device_arg='default',
                     retries=1,
                     enable_usb_resets=False,
                     abis=None,
                     persistent_shell=False,
                     **kwargs):
    """Returns a list of DeviceUtils instances.

    Returns a list of DeviceUtils instances that are attached, not denylisted,
    and optionally filtered by --device flags or ANDROID_SERIAL environment
    variable.

    Args:
      denylist: A DeviceDenylist instance (optional). Device serials in this
          denylist will never be returned, but a warning will be logged if they
          otherwise would have been.
      device_arg: The value of the --device flag. This can be:
          'default' -> Same as [], but returns an empty list rather than raise a
              NoDevicesError.
          [] -> Returns all devices, unless $ANDROID_SERIAL is set.
          None -> Use $ANDROID_SERIAL if set, otherwise looks for a single
              attached device. Raises an exception if multiple devices are
              attached.
          'serial' -> Returns an instance for the given serial, if not
              denylisted.
          ['A', 'B', ...] -> Returns instances for the subset that is not
              denylisted.
      retries: Number of times to restart adb server and query it again if no
          devices are found on the previous attempts, with exponential backoffs
          up to 60s between each retry.
      enable_usb_resets: If true, will attempt to trigger a USB reset prior to
          the last attempt if there are no available devices. It will only reset
          those that appear to be android devices.
      abis: A list of ABIs for which the device needs to support at least one of
          (optional). See devil.android.ndk.abis for valid values.
      persistent_shell: Makes AdbWrapper pipe commands through a single
          "adb shell" instead of spawning a new shell each invocation. Can
          save a lot of time as adb shell startup can be nearly 100ms.
      A device serial, or a list of device serials (optional).

    Returns:
      A list of DeviceUtils instances.

    Raises:
      NoDevicesError: Raised when no non-denylisted devices exist and
          device_arg is passed.
      MultipleDevicesError: Raise when multiple devices exist, but |device_arg|
          is None.
    """
    allow_no_devices = False
    if device_arg == 'default':
      allow_no_devices = True
      device_arg = ()

    select_multiple = True
    if not isinstance(device_arg, (list, tuple)):
      select_multiple = False
      if device_arg:
        device_arg = (device_arg, )

    denylisted_devices = denylist.Read() if denylist else []

    # adb looks for ANDROID_SERIAL, so support it as well.
    android_serial = os.environ.get('ANDROID_SERIAL')
    if not device_arg and android_serial:
      device_arg = (android_serial, )

    def denylisted(serial):
      if serial in denylisted_devices:
        logger.warning('Device %s is denylisted.', serial)
        return True
      return False

    def supports_abi(abi, serial):
      if abis and abi not in abis:
        return False
      return True

    def _get_devices():
      if device_arg:
        devices = [cls(x, **kwargs) for x in device_arg if not denylisted(x)]
      else:
        devices = []
        for adb in adb_wrapper.AdbWrapper.Devices(
            persistent_shell=persistent_shell):
          serial = adb.GetDeviceSerial()
          if not denylisted(serial):
            device = cls(_CreateAdbWrapper(adb), **kwargs)
            supported_abis = device.GetSupportedABIs()
            if not supported_abis:
              supported_abis = [device.GetABI()]
            for supported_abi in supported_abis:
              if supports_abi(supported_abi, serial):
                devices.append(device)
                break
            else:
              logger.warning(
                  "Device %s doesn't support required ABIs "
                  "(supported: %s, required: %s)", serial,
                  ','.join(supported_abis), ','.join(abis))

      if len(devices) == 0 and not allow_no_devices:
        raise device_errors.NoDevicesError()
      if len(devices) > 1 and not select_multiple:
        raise device_errors.MultipleDevicesError(devices)
      return sorted(devices)

    def _reset_devices():
      if not reset_usb:
        logging.error(
            'reset_usb.py not supported on this platform (%s). Skipping usb '
            'resets.', sys.platform)
        return
      if device_arg:
        for serial in device_arg:
          reset_usb.reset_android_usb(serial)
      else:
        reset_usb.reset_all_android_devices()

    for attempt in range(retries + 1):
      try:
        return _get_devices()
      except device_errors.NoDevicesError:
        if attempt == retries:
          logging.error('No devices found after exhausting all retries.')
          raise
        if attempt == retries - 1 and enable_usb_resets:
          logging.warning(
              'Attempting to reset relevant USB devices prior to the last '
              'attempt.')
          _reset_devices()
        # math.pow returns floats, so cast to int for easier testing
        sleep_s = min(int(math.pow(2, attempt + 1)), 60)
        logger.warning(
            'No devices found. Will try again after restarting adb server '
            'and a short nap of %d s.', sleep_s)
        time.sleep(sleep_s)
        adb_wrapper.RestartServer()

  @decorators.WithTimeoutAndRetriesFromInstance()
  def RestartAdbd(self, timeout=None, retries=None):
    logger.info('Restarting adbd on device.')
    self.RunShellCommand(['setprop', 'ctl.restart', 'adbd'],
                         check_return=False,
                         as_root=True)
    # Need to kill persistent shells as the restart kills the connection.
    self.adb.KillAllPersistentAdbs()
    self.adb.WaitForDevice()

  @decorators.WithTimeoutAndRetriesFromInstance()
  def GrantPermissions(self, package, permissions, timeout=None, retries=None):
    """Grant permissions to a package.

    Note for multi-user: when "--user" param is not specified,
      the "appops set" command applies to current user.
      the "pm grant" command applies to system user.
    """

    if not permissions:
      return

    user_param = ''
    if self.target_user is not None:
      user_param = '--user {}'.format(self.target_user)

    # For Andorid-11(R), enable MANAGE_EXTERNAL_STORAGE for testing.
    # See https://bit.ly/2MBjBIM for details.
    if ('android.permission.MANAGE_EXTERNAL_STORAGE' in permissions
        and self.build_version_sdk >= version_codes.R):
      script_manage_ext_storage = [
          'appops set {user_param} {package} MANAGE_EXTERNAL_STORAGE allow',
          'echo "{sep}MANAGE_EXTERNAL_STORAGE{sep}$?{sep}"',
      ]
    else:
      script_manage_ext_storage = []

    permissions = set(p for p in permissions
                      if not _PERMISSIONS_DENYLIST_RE.match(p))

    if ('android.permission.WRITE_EXTERNAL_STORAGE' in permissions
        and 'android.permission.READ_EXTERNAL_STORAGE' not in permissions):
      permissions.add('android.permission.READ_EXTERNAL_STORAGE')

    # This was introduced in API level 33:
    # https://developer.android.com/develop/ui/views/notifications/notification-permission
    if self.build_version_sdk < 33:
      permissions.discard('android.permission.POST_NOTIFICATIONS')

    script_raw = [
        'p={package}',
        'for q in {permissions}',
        'do pm grant {user_param} "$p" "$q"',
        'echo "{sep}$q{sep}$?{sep}"',
        'done',
    ] + script_manage_ext_storage

    script = ';'.join(script_raw).format(
        package=cmd_helper.SingleQuote(package),
        permissions=' '.join(
            cmd_helper.SingleQuote(p) for p in sorted(permissions)),
        user_param=user_param,
        sep=_SHELL_OUTPUT_SEPARATOR)

    logger.info('Setting permissions for %s.', package)
    res = self.RunShellCommand(
        script,
        shell=True,
        raw_output=True,
        large_output=True,
        check_return=True)
    res = res.split(_SHELL_OUTPUT_SEPARATOR)
    failures = [
        (permission, output.strip())
        for permission, status, output in zip(res[1::3], res[2::3], res[0::3])
        if int(status)
    ]

    if failures:
      logger.warning(
          'Failed to grant some permissions. Denylist may need to be updated?')
      for permission, output in failures:
        # Try to grab the relevant error message from the output.
        m = _PERMISSIONS_EXCEPTION_RE.search(output)
        if m:
          error_msg = m.group(0)
        elif len(output) > 200:
          error_msg = repr(output[:200]) + ' (truncated)'
        else:
          error_msg = repr(output)
        logger.warning('- %s: %s', permission, error_msg)

  @decorators.WithTimeoutAndRetriesFromInstance()
  def IsScreenOn(self, timeout=None, retries=None):
    """Determines if screen is on.

    Dumpsys input_method exposes screen on/off state. Below is an explination of
    the states.

    Pre-L:
      On: mScreenOn=true
      Off: mScreenOn=false
    L+:
      On: mInteractive=true
      Off: mInteractive=false

    Returns:
      True if screen is on, false if it is off.

    Raises:
      device_errors.CommandFailedError: If screen state cannot be found.
    """
    if self.build_version_sdk < version_codes.LOLLIPOP:
      input_check = 'mScreenOn'
      check_value = 'mScreenOn=true'
    else:
      input_check = 'mInteractive'
      check_value = 'mInteractive=true'
    dumpsys_out = self._RunPipedShellCommand(
        'dumpsys input_method | grep %s' % input_check)
    if not dumpsys_out:
      raise device_errors.CommandFailedError('Unable to detect screen state',
                                             str(self))
    return check_value in dumpsys_out[0]

  @decorators.WithTimeoutAndRetriesFromInstance()
  def SetScreen(self, on, timeout=None, retries=None):
    """Turns screen on and off.

    Args:
      on: bool to decide state to switch to. True = on False = off.
    """

    def screen_test():
      return self.IsScreenOn() == on

    if screen_test():
      logger.info('Screen already in expected state.')
      return
    self.SendKeyEvent(keyevent.KEYCODE_POWER)
    timeout_retry.WaitFor(screen_test, wait_period=1)

  @decorators.WithTimeoutAndRetriesFromInstance()
  def ChangeOwner(self, owner_group, paths, timeout=None, retries=None):
    """Changes file system ownership for permissions.

    Args:
      owner_group: New owner and group to assign. Note that this should be a
        string in the form user[.group] where the group is option.
      paths: Paths to change ownership of.

      Note that the -R recursive option is not supported by all Android
      versions.
    """
    if not paths:
      return
    self.RunShellCommand(['chown', owner_group] + paths, check_return=True)

  @decorators.WithTimeoutAndRetriesFromInstance()
  def ChangeSecurityContext(self,
                            security_context,
                            paths,
                            timeout=None,
                            retries=None):
    """Changes the SELinux security context for files.

    Args:
      security_context: The new security context as a string
      paths: Paths to change the security context of.

      Note that the -R recursive option is not supported by all Android
      versions.
    """
    if not paths:
      return
    command = ['chcon', security_context] + paths

    # Note, need to force su because chcon can fail with permission errors even
    # if the device is rooted.
    self.RunShellCommand(command, as_root=_FORCE_SU, check_return=True)

  @decorators.WithTimeoutAndRetriesFromInstance()
  def PlaceNomediaFile(self, device_path, timeout=None, retries=None):
    """Places .nomedia file in a given path on device.

    This helps to prevent system from scanning media files inside that path.

    Args:
      device_path: Base path on device to place .nomedia file.
    """

    if self.target_user is not None:
      device_path = self.ResolveSpecialPath(device_path)

    self.RunShellCommand(['mkdir', '-p', device_path],
                         check_return=True,
                         as_root=self.target_user is not None)
    self.WriteFile('%s/.nomedia' % device_path,
                   'https://crbug.com/796640',
                   as_root=self.target_user is not None)
