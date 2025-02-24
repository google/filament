# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os

from devil import devil_env
from devil.utils import lazy

with devil_env.SysPath(devil_env.DEPENDENCY_MANAGER_PATH):
  import dependency_manager  # pylint: disable=import-error
  import dependency_manager.exceptions as exceptions


def GetPath(build_tool):
  try:
    return devil_env.config.LocalPath(build_tool)
  except dependency_manager.NoPathFoundError:
    pass

  try:
    return _PathInLocalSdk(build_tool)
  except dependency_manager.NoPathFoundError:
    pass

  return devil_env.config.FetchPath(build_tool)


def _PathInLocalSdk(build_tool):
  build_tools_path = _build_tools_path.read()
  if not build_tools_path:
    raise dependency_manager.NoPathFoundError(build_tool,
                                              devil_env.GetPlatform())

  candidate_path = os.path.join(build_tools_path, build_tool)
  if not os.path.exists(candidate_path):
    raise dependency_manager.NoPathFoundError(build_tool,
                                              devil_env.GetPlatform())

  return candidate_path


def _FindBuildTools():
  try:
    android_sdk_path = devil_env.config.LocalPath('android_sdk')
  except exceptions.NoPathFoundError:
    return None
  if not android_sdk_path:
    return None

  build_tools_path = os.path.join(android_sdk_path, 'build-tools')
  if not os.path.isdir(build_tools_path):
    return None

  build_tools_contents = os.listdir(build_tools_path)

  if not build_tools_contents:
    return None

  if len(build_tools_contents) > 1:
    build_tools_contents.sort()
  return os.path.join(android_sdk_path, 'build-tools', build_tools_contents[-1])


_build_tools_path = lazy.WeakConstant(_FindBuildTools)
