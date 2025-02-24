# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import base64
import io
import gzip
import os
import re

import six

from devil import devil_env
from devil.android import device_errors
from devil.utils import cmd_helper

MD5SUM_DEVICE_LIB_PATH = '/data/local/tmp/md5sum'
MD5SUM_DEVICE_BIN_PATH = MD5SUM_DEVICE_LIB_PATH + '/md5sum_bin'

_STARTS_WITH_CHECKSUM_RE = re.compile(r'^[0-9a-fA-F]{16}$')

# We need to cap how many paths we send to the md5_sum binaries at once because
# the ARG_MAX on Android devices is relatively small, typically 131072 bytes.
# However, the more paths we use per invocation, the lower the overhead of
# starting processes, so we want to maximize this number, but we can't compute
# it exactly as we don't know how well our paths will compress.
# 5000 is experimentally determined to be reasonable. 10000 fails, and 7500
# works with existing usage, so 5000 seems like a pretty safe compromise.
_MAX_PATHS_PER_INVOCATION = 5000


def CalculateHostMd5Sums(paths):
  """Calculates the MD5 sum value for all items in |paths|.

  Directories are traversed recursively and the MD5 sum of each file found is
  reported in the result.

  Args:
    paths: A list of host paths to md5sum.
  Returns:
    A dict mapping file paths to their respective md5sum checksums.
  """
  if isinstance(paths, six.string_types):
    paths = [paths]
  paths = list(paths)

  md5sum_bin_host_path = devil_env.config.FetchPath('md5sum_host')
  if not os.path.exists(md5sum_bin_host_path):
    raise IOError('File not built: %s' % md5sum_bin_host_path)
  out = ""
  for i in range(0, len(paths), _MAX_PATHS_PER_INVOCATION):
    mem_file = io.BytesIO()
    compressed = gzip.GzipFile(fileobj=mem_file, mode="wb")
    data = ";".join(
          [os.path.realpath(p) for p in paths[i:i+_MAX_PATHS_PER_INVOCATION]])
    if six.PY3:
      data = data.encode('utf-8')
    compressed.write(data)
    compressed.close()
    compressed_paths = base64.b64encode(mem_file.getvalue())
    out += cmd_helper.GetCmdOutput(
        [md5sum_bin_host_path, "-gz", compressed_paths])

  return dict(zip(paths, out.splitlines()))


def CalculateDeviceMd5Sums(paths, device):
  """Calculates the MD5 sum value for all items in |paths|.

  Directories are traversed recursively and the MD5 sum of each file found is
  reported in the result.

  Args:
    paths: A list of device paths to md5sum.
  Returns:
    A dict mapping file paths to their respective md5sum checksums.
  """
  if not paths:
    return {}

  if isinstance(paths, six.string_types):
    paths = [paths]
  paths = list(paths)

  md5sum_dist_path = devil_env.config.FetchPath('md5sum_device', device=device)

  if os.path.isdir(md5sum_dist_path):
    md5sum_dist_bin_path = os.path.join(md5sum_dist_path, 'md5sum_bin')
  else:
    md5sum_dist_bin_path = md5sum_dist_path

  if not os.path.exists(md5sum_dist_path):
    raise IOError('File not built: %s' % md5sum_dist_path)
  md5sum_file_size = os.path.getsize(md5sum_dist_bin_path)

  # For better performance, make the script as small as possible to try and
  # avoid needing to write to an intermediary file (which RunShellCommand will
  # do if necessary).
  md5sum_script = 'a=%s;' % MD5SUM_DEVICE_BIN_PATH
  # Check if the binary is missing or has changed (using its file size as an
  # indicator), and trigger a (re-)push via the exit code.
  md5sum_script += '! [[ $(ls -l $a) = *%d* ]]&&exit 2;' % md5sum_file_size
  # Make sure it can find libbase.so
  md5sum_script += 'export LD_LIBRARY_PATH=%s;' % MD5SUM_DEVICE_LIB_PATH
  for i in range(0, len(paths), _MAX_PATHS_PER_INVOCATION):
    mem_file = io.BytesIO()
    compressed = gzip.GzipFile(fileobj=mem_file, mode="wb")
    data = ";".join(paths[i:i+_MAX_PATHS_PER_INVOCATION])
    if six.PY3:
      data = data.encode('utf-8')
    compressed.write(data)
    compressed.close()
    compressed_paths = base64.b64encode(mem_file.getvalue())
    if six.PY3:
      compressed_paths = compressed_paths.decode('utf-8')
    md5sum_script += '$a -gz %s;' % compressed_paths
  try:
    out = device.RunShellCommand(
        md5sum_script, shell=True, check_return=True, large_output=True)
  except device_errors.AdbShellCommandFailedError as e:
    # Push the binary only if it is found to not exist
    # (faster than checking up-front).
    if e.status == 2:
      # If files were previously pushed as root (adbd running as root), trying
      # to re-push as non-root causes the push command to report success, but
      # actually fail. So, wipe the directory first.
      device.RunShellCommand(['rm', '-rf', MD5SUM_DEVICE_LIB_PATH],
                             as_root=True,
                             check_return=True)
      if os.path.isdir(md5sum_dist_path):
        device.adb.Push(md5sum_dist_path, MD5SUM_DEVICE_LIB_PATH)
      else:
        mkdir_cmd = 'a=%s;[[ -e $a ]] || mkdir $a' % MD5SUM_DEVICE_LIB_PATH
        device.RunShellCommand(mkdir_cmd, shell=True, check_return=True)
        device.adb.Push(md5sum_dist_bin_path, MD5SUM_DEVICE_BIN_PATH)

      out = device.RunShellCommand(
          md5sum_script, shell=True, check_return=True, large_output=True)
    else:
      raise

  return dict(zip(paths, [l for l in out if _STARTS_WITH_CHECKSUM_RE.match(l)]))
