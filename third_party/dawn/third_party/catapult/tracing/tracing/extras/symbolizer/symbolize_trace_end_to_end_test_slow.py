#!/usr/bin/env python
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# pylint: disable=line-too-long
# To upload test files, use the command:
# <path_to_depot_tools>/upload_to_google_storage.py --bucket chrome-partner-telemetry <path_to_data_dir>/linux_trace_v2_breakpad_postsymbolization.json.gz
#
# To run this test suite, use ./tracing/bin/run_symbolizer_tests

from __future__ import print_function

import json
import os
import shutil
import sys
import tempfile
import unittest

from tracing.extras.symbolizer import symbolize_trace

_THIS_DIR_PATH = os.path.abspath(os.path.dirname(__file__))
_TRACING_DIR = os.path.abspath(
    os.path.join(_THIS_DIR_PATH,
                 os.path.pardir,
                 os.path.pardir,
                 os.path.pardir))
_PY_UTILS_PATH = os.path.abspath(os.path.join(
    _TRACING_DIR,
    os.path.pardir,
    'common',
    'py_utils'))
sys.path.append(_PY_UTILS_PATH)
import py_utils.cloud_storage as cloud_storage  # pylint: disable=wrong-import-position


def _DownloadFromCloudStorage(path):
  print('Downloading %s from gcs.' % (path))
  cloud_storage.GetIfChanged(path, cloud_storage.PARTNER_BUCKET)


class SymbolizeTraceEndToEndTest(unittest.TestCase):
  def _ValidateTrace(self, trace_path, expectations):
    with symbolize_trace.OpenTraceFile(trace_path, 'r') as trace_file:
      trace = symbolize_trace.Trace(json.load(trace_file))
    # Find the browser process.
    browser = None
    for process in trace.processes:
      if process.name == expectations['process']:
        browser = process
    self.assertTrue(browser)

    # Look for a frame with a symbolize name, and check that it has the right
    # parent.
    frames = browser.stack_frame_map.frame_by_id
    exact = expectations['frame_exact']
    found = False
    for _, frame in frames.items():
      if frame.name.strip() == exact['frame_name']:
        parent_id = frame.parent_id
        if frames[parent_id].name.strip() == exact['parent_name']:
          found = True
          break

    self.assertTrue(found)


  def _RunSymbolizationOnTrace(self, pre_symbolization, expectations,
                               extra_options):
    trace_presymbolization_path = os.path.join(
        _THIS_DIR_PATH, 'data', pre_symbolization)
    _DownloadFromCloudStorage(trace_presymbolization_path)
    self.assertTrue(os.path.exists(trace_presymbolization_path))

    temporary_fd, temporary_trace = tempfile.mkstemp(suffix='.json.gz')

    symbolization_options = ['--only-symbolize-chrome-symbols',
                             '--no-backup',
                             '--cloud-storage-bucket',
                             cloud_storage.PARTNER_BUCKET,
                             temporary_trace]

    symbolization_options.extend(extra_options)

    # On windows, a pre-built version of addr2line-pdb is provided.
    if sys.platform == 'win32':
      addr2line_path = os.path.join(
          _THIS_DIR_PATH, 'data', 'addr2line-pdb.exe')
      _DownloadFromCloudStorage(addr2line_path)
      self.assertTrue(os.path.exists(addr2line_path))
      symbolization_options += ['--addr2line-executable', addr2line_path]

    # Execute symbolization and compare results with the expected trace.
    try:
      shutil.copy(trace_presymbolization_path, temporary_trace)
      self.assertTrue(symbolize_trace.main(symbolization_options))
      self._ValidateTrace(temporary_trace, expectations)
    finally:
      os.close(temporary_fd)
      if os.path.exists(temporary_trace):
        os.remove(temporary_trace)


  def testMacv2(self):
    if sys.platform != 'darwin':
      return
    # The corresponding macOS Chrome symbols must be uploaded to
    # "gs://chrome-partner-telemetry/desktop-symbolizer-test/66.0.3334.0/mac64/Google Chrome.dSYM.tar.bz2"
    # since the waterfall bots do not have access to the chrome-unsigned bucket.
    expectations = {}
    expectations['process'] = 'Browser'
    expectations['frame_exact'] = {
        'parent_name': 'ProfileImpl::OnPrefsLoaded(Profile::CreateMode, bool)',
        'frame_name': 'ProfileImpl::OnLocaleReady()'
    }
    self._RunSymbolizationOnTrace(
        'mac_trace_v2_presymbolization.json.gz',
        expectations, [])

  def testMacv2Breakpad(self):
    # The corresponding macOS Chrome symbols must be uploaded to
    # "gs://chrome-partner-telemetry/desktop-symbolizer-test/66.0.3334.0/mac64/breakpad-info"
    # since the waterfall bots do not have access to the chrome-unsigned bucket.
    expectations = {}
    expectations['process'] = 'Browser'
    expectations['frame_exact'] = {
        'parent_name': 'ProfileImpl::OnPrefsLoaded(Profile::CreateMode, bool)',
        'frame_name': 'ProfileImpl::OnLocaleReady()'
    }
    self._RunSymbolizationOnTrace(
        'mac_trace_v2_presymbolization.json.gz',
        expectations, ['--use-breakpad-symbols'])

  def testWin64v1(self):
    if sys.platform != 'win32':
      return

    expectations = {}
    expectations['process'] = 'Browser'
    expectations['frame_exact'] = {
        'parent_name': 'ChromeMain',
        'frame_name': 'content::ContentMain'
    }
    # The corresponding Win64 Chrome symbols must be uploaded to
    # "gs://chrome-partner-telemetry/desktop-symbolizer-test/61.0.3130.0/"
    # "win64-pgo/chrome-win32-syms.zip"
    # and the corresponding executables to
    # "gs://chrome-partner-telemetry/desktop-symbolizer-test/61.0.3130.0/"
    # "win64-pgo/chrome-win64-pgo.zip"
    # since the waterfall bots do not have access to the chrome-unsigned bucket.
    self._RunSymbolizationOnTrace('windows_trace_v1_presymbolization.json.gz',
                                  expectations,
                                  [])

  def testWin64v2(self):
    if sys.platform != 'win32':
      return

    expectations = {}
    expectations['process'] = 'Browser'
    expectations['frame_exact'] = {
        'parent_name': 'base::MessagePumpWin::Run',
        'frame_name': 'base::MessagePumpForUI::DoRunLoop'
    }
    # The corresponding Win64 Chrome symbols must be uploaded to
    # "gs://chrome-partner-telemetry/desktop-symbolizer-test/61.0.3142.0/"
    # "win64-pgo/chrome-win32-syms.zip"
    # and the corresponding executables to
    # "gs://chrome-partner-telemetry/desktop-symbolizer-test/61.0.3142.0/"
    # "win64-pgo/chrome-win64-pgo.zip"
    # since the waterfall bots do not have access to the chrome-unsigned bucket.
    self._RunSymbolizationOnTrace('windows_trace_v2_presymbolization.json.gz',
                                  expectations,
                                  [])


  def testLinuxv2(self):
    # The corresponding Linux breakpad symbols must be uploaded to
    # "gs://chrome-partner-telemetry/desktop-symbolizer-test/64.0.3282.24/linux64/breakpad-info.zip"
    # since the waterfall bots do not have access to the chrome-unsigned bucket.
    expectations = {}
    expectations['process'] = 'Renderer'
    expectations['frame_exact'] = {
        'parent_name': 'cc::LayerTreeSettings::LayerTreeSettings(cc::LayerTreeSettings const&)',
        'frame_name': 'viz::ResourceSettings::ResourceSettings(viz::ResourceSettings const&)'
    }
    self._RunSymbolizationOnTrace(
        'linux_trace_v2_presymbolization.json.gz',
        expectations,
        ['--use-breakpad-symbols'])


if __name__ == '__main__':
  unittest.main()
