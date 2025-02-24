# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os

import six

class LocalPathInfo():

  def __init__(self, path_priority_groups):
    """Container for a set of local file paths where a given dependency
    can be stored.

    Organized as a list of groups, where each group is itself a file path list.
    See GetLocalPath() to understand how they are used.

    Args:
      path_priority_groups: Can be either None, or a list of file path
        strings (corresponding to a list of groups, where each group has
        a single file path), or a list of a list of file path strings
        (i.e. a list of groups).
    """
    self._path_priority_groups = self._ParseLocalPaths(path_priority_groups)

  def GetLocalPath(self):
    """Look for a local file, and return its path.

    Looks for the first group which has at least one existing file path. Then
    returns the most-recent of these files.

    Returns:
      Local file path, if found, or None otherwise.
    """
    for priority_group in self._path_priority_groups:
      priority_group = [g for g in priority_group if os.path.exists(g)]
      if not priority_group:
        continue
      return max(priority_group, key=lambda path: os.stat(path).st_mtime)
    return None

  def IsPathInLocalPaths(self, path):
    """Returns true if |path| is in one of this instance's file path lists."""
    return any(
        path in priority_group for priority_group in self._path_priority_groups)

  def Update(self, local_path_info):
    """Update this object from the content of another LocalPathInfo instance.

    Any file path from |local_path_info| that is not already contained in the
    current instance will be added into new groups to it.

    Args:
      local_path_info: Another LocalPathInfo instance, or None.
    """
    if not local_path_info:
      return
    for priority_group in local_path_info._path_priority_groups:
      group_list = []
      for path in priority_group:
        if not self.IsPathInLocalPaths(path):
          group_list.append(path)
      if group_list:
        self._path_priority_groups.append(group_list)

  @staticmethod
  def _ParseLocalPaths(local_paths):
    if not local_paths:
      return []
    return [[e] if isinstance(e, six.string_types) else e for e in local_paths]
