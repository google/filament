# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os

import tracing_project


FILE_GROUPS = ["tracing_css_files",
               "tracing_js_html_files",
               "tracing_img_files"]


def GetFileGroupFromFileName(filename):
  extension = os.path.splitext(filename)[1]
  return {
      '.css': 'tracing_css_files',
      '.html': 'tracing_js_html_files',
      '.js': 'tracing_js_html_files',
      '.png': 'tracing_img_files'
  }[extension]


def CheckListedFilesSorted(src_file, group_name, listed_files):
  sorted_files = sorted(listed_files)
  if sorted_files != listed_files:
    mismatch = ''
    for i, f in enumerate(listed_files):
      if f != sorted_files[i]:
        mismatch = f
        break
    what_is = '  ' + '\n  '.join(listed_files)
    what_should_be = '  ' + '\n  '.join(sorted_files)
    return '''In group {0} from file {1}, filenames aren't sorted.

First mismatch:
  {2}

Current listing:
{3}

Correct listing:
{4}\n\n'''.format(group_name, src_file, mismatch, what_is, what_should_be)

  return ''


def GetKnownFiles():
  project = tracing_project.TracingProject()

  vulcanizer = project.CreateVulcanizer()
  m = vulcanizer.loader.LoadModule(
      module_name='tracing.ui.extras.about_tracing.about_tracing')
  absolute_filenames = m.GetAllDependentFilenamesRecursive(
      include_raw_scripts=False)

  return list({os.path.relpath(f, project.tracing_root_path)
                   for f in absolute_filenames})


def CheckCommon(file_name, listed_files):
  known_files = GetKnownFiles()
  u = set(listed_files).union(set(known_files))
  i = set(listed_files).intersection(set(known_files))
  diff = list(u - i)

  if len(diff) == 0:
    return ''

  error = 'Entries in ' + file_name + ' do not match files on disk:\n'
  in_file_only = list(set(listed_files) - set(known_files))
  in_known_only = list(set(known_files) - set(listed_files))

  if len(in_file_only) > 0:
    error += '  In file only:\n    ' + '\n    '.join(sorted(in_file_only))
  if len(in_known_only) > 0:
    if len(in_file_only) > 0:
      error += '\n\n'
    error += '  On disk only:\n    ' + '\n    '.join(sorted(in_known_only))

  if in_file_only:
    error += (
        '\n\n'
        '  Note: only files actually used in about:tracing should\n'
        '  be listed in the build files. Try running \n'
        '       tracing/bin/update_gni\n'
        '  to update the files automatically.')

  return error
