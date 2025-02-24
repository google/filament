# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
import os
import re


def _AddToPathIfNeeded(path):
  if path not in sys.path:
    sys.path.insert(0, path)


def UpdateSysPathIfNeeded():
  for path in GetDependencyPaths():
    _AddToPathIfNeeded(path)


def GetDependencyPaths():
  # TODO(#3703): Separate the paths that are only used by the dev server into
  # another call.
  p = TracingProject()
  return [
      p.catapult_path,
      p.py_vulcanize_path,
      p.vinn_path,
      os.path.join(p.catapult_third_party_path, 'six'),
      os.path.join(p.catapult_third_party_path, 'webapp2'),
      os.path.join(p.catapult_path, 'common', 'py_utils'),
      os.path.join(p.tracing_third_party_path, 'symbols'),
  ]


def _FindAllFilesRecursive(source_paths):
  assert isinstance(source_paths, list)
  all_filenames = set()
  for source_path in source_paths:
    for dirpath, _, filenames in os.walk(source_path):
      for f in filenames:
        if f.startswith('.'):
          continue
        x = os.path.abspath(os.path.join(dirpath, f))
        all_filenames.add(x)
  return all_filenames

def _IsFilenameATest(x):
  if x.endswith('_test.js'):
    return True

  if x.endswith('_test.html'):
    return True

  if x.endswith('_unittest.js'):
    return True

  if x.endswith('_unittest.html'):
    return True

  # TODO(nduca): Add content test?
  return False


class TracingProject(object):
  catapult_path = os.path.abspath(
      os.path.join(os.path.dirname(__file__), os.path.pardir))

  tracing_root_path = os.path.join(catapult_path, 'tracing')
  trace_processor_root_path = os.path.join(catapult_path, 'trace_processor')
  common_root_path = os.path.join(catapult_path, 'common')
  tracing_src_path = os.path.join(tracing_root_path, 'tracing')
  extras_path = os.path.join(tracing_src_path, 'extras')
  ui_extras_path = os.path.join(tracing_src_path, 'ui', 'extras')

  catapult_third_party_path = os.path.join(catapult_path, 'third_party')
  polymer_path = os.path.join(catapult_third_party_path, 'polymer')

  tracing_third_party_path = os.path.join(tracing_root_path, 'third_party')
  py_vulcanize_path = os.path.join(common_root_path, 'py_vulcanize')
  vinn_path = os.path.join(catapult_third_party_path, 'vinn')

  jszip_path = os.path.join(tracing_third_party_path, 'jszip')
  pako_path = os.path.join(tracing_third_party_path, 'pako')
  jpegjs_path = os.path.join(tracing_third_party_path, 'jpeg-js')

  glmatrix_path = os.path.join(
      tracing_third_party_path, 'gl-matrix', 'dist')

  mannwhitneyu_path = os.path.join(
      tracing_third_party_path, 'mannwhitneyu')

  ui_path = os.path.join(tracing_src_path, 'ui')
  d3_path = os.path.join(tracing_third_party_path, 'd3')
  chai_path = os.path.join(tracing_third_party_path, 'chai')
  mocha_path = os.path.join(tracing_third_party_path, 'mocha')
  oboe_path = os.path.join(tracing_third_party_path, 'oboe')

  mre_path = os.path.join(tracing_src_path, 'mre')

  metrics_path = os.path.join(tracing_src_path, 'metrics')
  diagnostics_path = os.path.join(tracing_src_path, 'value', 'diagnostics')

  value_ui_path = os.path.join(tracing_src_path, 'value', 'ui')
  metrics_ui_path = os.path.join(tracing_src_path, 'metrics', 'ui')

  test_data_path = os.path.join(tracing_root_path, 'test_data')
  skp_data_path = os.path.join(tracing_root_path, 'skp_data')

  rjsmin_path = os.path.join(
      tracing_third_party_path, 'tvcm', 'third_party', 'rjsmin')
  rcssmin_path = os.path.join(
      tracing_third_party_path, 'tvcm', 'third_party', 'rcssmin')

  def __init__(self):
    self.source_paths = []
    self.source_paths.append(self.tracing_root_path)
    self.source_paths.append(self.polymer_path)
    self.source_paths.append(self.tracing_third_party_path)
    self.source_paths.append(self.mre_path)
    self.source_paths.append(self.jszip_path)
    self.source_paths.append(self.pako_path)
    self.source_paths.append(self.jpegjs_path)
    self.source_paths.append(self.glmatrix_path)
    self.source_paths.append(self.mannwhitneyu_path)
    self.source_paths.append(self.d3_path)
    self.source_paths.append(self.chai_path)
    self.source_paths.append(self.mocha_path)
    self.source_paths.append(self.oboe_path)

  def CreateVulcanizer(self):
    from py_vulcanize import project as project_module # pylint: disable=import-outside-toplevel
    return project_module.Project(self.source_paths)

  def IsD8CompatibleFile(self, filename):
    if filename.startswith(self.ui_path):
      return False

    if filename.startswith(self.value_ui_path):
      return False

    if filename.startswith(self.metrics_ui_path):
      return False

    return True

  def FindAllTestModuleRelPaths(self, pred=None):
    if pred is None:
      pred = lambda x: True

    all_filenames = _FindAllFilesRecursive([self.tracing_src_path])
    test_module_filenames = [x for x in all_filenames if
                             _IsFilenameATest(x) and pred(x)]
    test_module_filenames.sort()

    return [os.path.relpath(x, self.tracing_root_path)
            for x in test_module_filenames]

  def FindAllMetricsModuleRelPaths(self):
    all_filenames = _FindAllFilesRecursive([self.tracing_src_path])
    all_metrics_module_filenames = []
    for x in all_filenames:
      if x.startswith(self.metrics_path) and not _IsFilenameATest(x):
        all_metrics_module_filenames.append(x)
    all_metrics_module_filenames.sort()
    return [os.path.relpath(x, self.tracing_root_path)
            for x in all_metrics_module_filenames]

  def FindAllDiagnosticsModuleRelPaths(self):
    all_filenames = _FindAllFilesRecursive([self.tracing_src_path])
    all_diagnostics_module_filenames = []
    for x in all_filenames:
      if x.startswith(self.diagnostics_path) and not _IsFilenameATest(x):
        all_diagnostics_module_filenames.append(x)
    all_diagnostics_module_filenames.sort()
    return [os.path.relpath(x, self.tracing_root_path)
            for x in all_diagnostics_module_filenames]

  def FindAllD8TestModuleRelPaths(self):
    return self.FindAllTestModuleRelPaths(pred=self.IsD8CompatibleFile)

  def GetConfigNames(self):
    config_files = [
        os.path.join(self.ui_extras_path, x)
        for x in os.listdir(self.ui_extras_path)
        if x.endswith('_config.html')
    ]

    config_files = [x for x in config_files if os.path.isfile(x)]

    config_basenames = [os.path.basename(x) for x in config_files]
    config_names = [re.match('(.+)_config.html$', x).group(1)
                    for x in config_basenames]
    return config_names

  def GetDefaultConfigName(self):
    assert 'full' in self.GetConfigNames()
    return 'full'

  def AddConfigNameOptionToParser(self, parser):
    choices = self.GetConfigNames()
    parser.add_argument(
        '--config', dest='config_name',
        choices=choices, default=self.GetDefaultConfigName(),
        help='Picks a browser config. Valid choices: %s' % ', '.join(choices))
    return choices

  def GetModuleNameForConfigName(self, config_name):
    return 'tracing.ui.extras.%s_config' % config_name
