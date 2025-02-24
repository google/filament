# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import os

from tracing.mre import corpus_driver
from tracing.mre import file_handle


def _GetFilesIn(basedir):
  data_files = []
  for dirpath, dirnames, filenames in os.walk(basedir, followlinks=True):
    new_dirnames = [d for d in dirnames if not d.startswith('.')]
    del dirnames[:]
    dirnames += new_dirnames
    for f in filenames:
      if f.startswith('.'):
        continue
      if f == 'README.md':
        continue
      full_f = os.path.join(dirpath, f)
      rel_f = os.path.relpath(full_f, basedir)
      data_files.append(rel_f)

  data_files.sort()
  return data_files


def _DefaultUrlResover(abspath):
  return 'file:///%s' % abspath


class LocalDirectoryCorpusDriver(corpus_driver.CorpusDriver):

  def __init__(self, trace_directory, url_resolver=_DefaultUrlResover):
    self.directory = trace_directory
    self.url_resolver = url_resolver

  @staticmethod
  def CheckAndCreateInitArguments(parser, args):
    del args  # Unused by LocalDirectoryCorpusDriver.
    trace_dir = os.getcwd()
    if not os.path.exists(trace_dir):
      parser.error('Trace directory does not exist')
      return None
    return {'trace_directory': trace_dir}

  @staticmethod
  def AddArguments(parser):
    parser.add_argument(
        '--trace_directory',
        help='Local directory containing traces to process.')

  def GetTraceHandles(self):
    trace_handles = []

    files = _GetFilesIn(self.directory)
    for rel_filename in files:
      filename = os.path.join(self.directory, rel_filename)

      url = self.url_resolver(filename)
      if url is None:
        url = _DefaultUrlResover(filename)

      th = file_handle.URLFileHandle(url, 'file://' + filename)
      trace_handles.append(th)

    return trace_handles
