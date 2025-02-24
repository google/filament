# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import re

from py_vulcanize import js_utils
from py_vulcanize import module
from py_vulcanize import parse_html_deps
from py_vulcanize import style_sheet


def IsHTMLResourceTheModuleGivenConflictingResourceNames(
    js_resource, html_resource):  # pylint: disable=unused-argument
  return 'polymer-element' in html_resource.contents


class HTMLModule(module.Module):

  @property
  def _module_dir_name(self):
    return os.path.dirname(self.resource.absolute_path)

  def Parse(self, excluded_scripts):
    try:
      parser_results = parse_html_deps.HTMLModuleParser().Parse(self.contents)
    except Exception as ex:
      raise Exception('While parsing %s: %s' % (self.name, str(ex)))

    self.dependency_metadata = Parse(self.loader,
                                     self.name,
                                     self._module_dir_name,
                                     self.IsThirdPartyComponent(),
                                     parser_results,
                                     excluded_scripts)
    self._parser_results = parser_results
    self.scripts = parser_results.scripts

  def Load(self, excluded_scripts):
    super(HTMLModule, self).Load(excluded_scripts=excluded_scripts)

    reachable_names = set([m.name
                           for m in self.all_dependent_modules_recursive])
    if 'tr.exportTo' in self.contents:
      if 'tracing.base.base' not in reachable_names:
        raise Exception('%s: Does not have a dependency on base' %
                        os.path.relpath(self.resource.absolute_path))

    for script in self.scripts:
      if script.is_external:
        if excluded_scripts and any(re.match(pattern, script.src) for
            pattern in excluded_scripts):
          continue

        resource = _HRefToResource(self.loader, self.name, self._module_dir_name,
                                   script.src,
                                   tag_for_err_msg='<script src="%s">' % script.src)
        path = resource.unix_style_relative_path
        raw_script = self.loader.LoadRawScript(path)
        self.dependent_raw_scripts.append(raw_script)
        script.loaded_raw_script = raw_script

  def GetTVCMDepsModuleType(self):
    return 'py_vulcanize.HTML_MODULE_TYPE'

  def AppendHTMLContentsToFile(self, f, ctl, minify=False):
    super(HTMLModule, self).AppendHTMLContentsToFile(f, ctl)

    ctl.current_module = self
    try:
      for piece in self._parser_results.YieldHTMLInPieces(ctl, minify=minify):
        f.write(piece)
    finally:
      ctl.current_module = None

  def HRefToResource(self, href, tag_for_err_msg):
    return _HRefToResource(self.loader, self.name, self._module_dir_name,
                           href, tag_for_err_msg)

  def AppendDirectlyDependentFilenamesTo(
      self, dependent_filenames, include_raw_scripts=True):
    super(HTMLModule, self).AppendDirectlyDependentFilenamesTo(
        dependent_filenames, include_raw_scripts)
    for contents in self._parser_results.inline_stylesheets:
      module_dirname = os.path.dirname(self.resource.absolute_path)
      ss = style_sheet.ParsedStyleSheet(
          self.loader, module_dirname, contents)
      ss.AppendDirectlyDependentFilenamesTo(dependent_filenames)

def _HRefToResource(
    loader, module_name, module_dir_name, href, tag_for_err_msg):
  if href[0] == '/':
    resource = loader.FindResourceGivenRelativePath(
        os.path.normpath(href[1:]))
  else:
    abspath = os.path.normpath(os.path.join(module_dir_name,
                                            os.path.normpath(href)))
    resource = loader.FindResourceGivenAbsolutePath(abspath)

  if not resource:
    raise module.DepsException(
        'In %s, the %s cannot be loaded because '
        'it is not in the search path' % (module_name, tag_for_err_msg))
  try:
    resource.contents
  except:
    raise module.DepsException('In %s, %s points at a nonexistent file ' % (
        module_name, tag_for_err_msg))
  return resource


def Parse(loader, module_name, module_dir_name, is_component, parser_results,
          exclude_scripts=None):
  res = module.ModuleDependencyMetadata()
  if is_component:
    return res

  # External script references.
  for href in parser_results.scripts_external:
    if exclude_scripts and any(re.match(pattern, href) for
        pattern in exclude_scripts):
      continue

    resource = _HRefToResource(loader, module_name, module_dir_name,
                               href,
                               tag_for_err_msg='<script src="%s">' % href)
    res.dependent_raw_script_relative_paths.append(
        resource.unix_style_relative_path)

  # External imports. Mostly the same as <script>, but we know its a module.
  for href in parser_results.imports:
    if exclude_scripts and any(re.match(pattern, href) for
        pattern in exclude_scripts):
      continue

    if not href.endswith('.html'):
      raise Exception(
          'In %s, the <link rel="import" href="%s"> must point at a '
          'file with an html suffix' % (module_name, href))

    resource = _HRefToResource(
        loader, module_name, module_dir_name, href,
        tag_for_err_msg='<link rel="import" href="%s">' % href)
    res.dependent_module_names.append(resource.name)

  # Style sheets.
  for href in parser_results.stylesheets:
    resource = _HRefToResource(
        loader, module_name, module_dir_name, href,
        tag_for_err_msg='<link rel="stylesheet" href="%s">' % href)
    res.style_sheet_names.append(resource.name)

  return res
