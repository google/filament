# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import collections
import os
import re

import tracing_project
from tracing_build import check_common


class _Token(object):

  def __init__(self, data, token_id=None):
    self.data = data
    if token_id:
      self.token_id = token_id
    else:
      self.token_id = 'plain'


class BuildFile(object):

  def __init__(self, text, file_groups):
    self._file_groups = file_groups
    self._tokens = list(self._Tokenize(text))

  def _Tokenize(self, text):
    rest = text
    token_regex = self._TokenRegex()
    while len(rest):
      m = token_regex.search(rest)
      if not m:
        # In `rest', we couldn't find a match.
        # So, lump the entire `rest' into a token
        # and stop producing any more tokens.
        yield _Token(rest)
        return
      min_index, end_index, matched_token = self._ProcessMatch(m)

      if min_index > 0:
        yield _Token(rest[:min_index])

      yield matched_token
      rest = rest[end_index:]

  def Update(self, files_by_group):
    for token in self._tokens:
      if token.token_id in files_by_group:
        token.data = self._GetReplacementListAsString(
            token.data,
            files_by_group[token.token_id])

  def Write(self, f):
    for token in self._tokens:
      f.write(token.data)

  def _ProcessMatch(self, match):
    raise NotImplementedError

  def _TokenRegex(self):
    raise NotImplementedError

  def _GetReplacementListAsString(self, existing_list_as_string, filelist):
    raise NotImplementedError


class GniFile(BuildFile):

  def _ProcessMatch(self, match):
    min_index = match.start(2)
    end_index = match.end(2)
    token = _Token(match.string[min_index:end_index],
                   token_id=match.groups()[0])
    return min_index, end_index, token

  def _TokenRegex(self):
    # regexp to match the following:
    #   file_group_name = [
    #     "path/to/one/file.extension",
    #     "another/file.ex",
    #   ]
    # In the match,
    # group 1 is : 'file_group_name'
    # group 2 is : ' "path/to/one/file.extension",\n  "another/file.ex",\n'
    regexp_str = r'(%s) = \[\n(.+?) +\],?\n' % '|'.join(self._file_groups)
    return re.compile(regexp_str, re.MULTILINE | re.DOTALL)

  def _GetReplacementListAsString(self, existing_list_as_string, filelist):
    list_entry = existing_list_as_string.splitlines()[0]
    prefix, _, suffix = list_entry.split('"')
    return ''.join(['"'.join([prefix, filename, suffix + '\n'])
                    for filename in filelist])


def _GroupFiles(file_name_to_group_name_func, filenames):
  file_groups = collections.defaultdict(lambda: [])
  for filename in filenames:
    file_groups[file_name_to_group_name_func(filename)].append(filename)
  for group in file_groups:
    file_groups[group].sort()
  return file_groups


def _UpdateBuildFile(filename, build_file_class):
  with open(filename, 'r') as f:
    build_file = build_file_class(f.read(), check_common.FILE_GROUPS)
  files_by_group = _GroupFiles(check_common.GetFileGroupFromFileName,
                               check_common.GetKnownFiles())
  build_file.Update(files_by_group)
  with open(filename, 'w') as f:
    build_file.Write(f)


def UpdateGni():
  tvp = tracing_project.TracingProject()
  _UpdateBuildFile(
      os.path.join(tvp.tracing_root_path, 'trace_viewer.gni'), GniFile)


def Update():
  UpdateGni()
