# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""ResourceFinder is a helper class for finding resources given their name."""

from __future__ import absolute_import
import codecs
import functools
import os
import six

from py_vulcanize import module
from py_vulcanize import style_sheet as style_sheet_module
from py_vulcanize import resource as resource_module
from py_vulcanize import html_module
from py_vulcanize import strip_js_comments


class ResourceLoader(object):
  """Manges loading modules and their dependencies from files.

  Modules handle parsing and the construction of their individual dependency
  pointers. The loader deals with bookkeeping of what has been loaded, and
  mapping names to file resources.
  """

  def __init__(self, project):
    self.project = project
    self.stripped_js_by_filename = {}
    self.loaded_modules = {}
    self.loaded_raw_scripts = {}
    self.loaded_style_sheets = {}
    self.loaded_images = {}

  @property
  def source_paths(self):
    """A list of base directories to search for modules under."""
    return self.project.source_paths

  def FindResource(self, some_path, binary=False):
    """Finds a Resource for the given path.

    Args:
      some_path: A relative or absolute path to a file.

    Returns:
      A Resource or None.
    """
    if os.path.isabs(some_path):
      return self.FindResourceGivenAbsolutePath(some_path, binary)
    else:
      return self.FindResourceGivenRelativePath(some_path, binary)

  def FindResourceGivenAbsolutePath(self, absolute_path, binary=False):
    """Returns a Resource for the given absolute path."""
    candidate_paths = []
    for source_path in self.source_paths:
      if absolute_path.startswith(source_path):
        candidate_paths.append(source_path)
    if len(candidate_paths) == 0:
      return None

    # Sort by length. Longest match wins.
    sorted(candidate_paths,
           key=functools.cmp_to_key(lambda x, y: len(x) - len(y)), reverse=True)
    longest_candidate = candidate_paths[-1]
    return resource_module.Resource(longest_candidate, absolute_path, binary)

  def FindResourceGivenRelativePath(self, relative_path, binary=False):
    """Returns a Resource for the given relative path."""
    absolute_path = None
    for script_path in self.source_paths:
      absolute_path = os.path.join(script_path, relative_path)
      if os.path.exists(absolute_path):
        return resource_module.Resource(script_path, absolute_path, binary)
    return None

  def _FindResourceGivenNameAndSuffix(
        self, requested_name, extension, return_resource=False):
    """Searches for a file and reads its contents.

    Args:
      requested_name: The name of the resource that was requested.
      extension: The extension for this requested resource.

    Returns:
      A (path, contents) pair.
    """
    pathy_name = requested_name.replace('.', os.sep)
    filename = pathy_name + extension

    resource = self.FindResourceGivenRelativePath(filename)
    if return_resource:
      return resource
    if not resource:
      return None, None
    return _read_file(resource.absolute_path)

  def FindModuleResource(self, requested_module_name):
    """Finds a module javascript file and returns a Resource, or none."""
    js_resource = self._FindResourceGivenNameAndSuffix(
        requested_module_name, '.js', return_resource=True)
    html_resource = self._FindResourceGivenNameAndSuffix(
        requested_module_name, '.html', return_resource=True)
    if js_resource and html_resource:
      if html_module.IsHTMLResourceTheModuleGivenConflictingResourceNames(
          js_resource, html_resource):
        return html_resource
      return js_resource
    elif js_resource:
      return js_resource
    return html_resource

  def LoadModule(self, module_name=None, module_filename=None,
                 excluded_scripts=None):
    assert bool(module_name) ^ bool(module_filename), (
        'Must provide either module_name or module_filename.')
    if module_filename:
      resource = self.FindResource(module_filename)
      if not resource:
        raise Exception('Could not find %s in %s' % (
            module_filename, repr(self.source_paths)))
      module_name = resource.name
    else:
      resource = None  # Will be set if we end up needing to load.

    if module_name in self.loaded_modules:
      assert self.loaded_modules[module_name].contents
      return self.loaded_modules[module_name]

    if not resource:  # happens when module_name was given
      resource = self.FindModuleResource(module_name)
      if not resource:
        raise module.DepsException('No resource for module "%s"' % module_name)

    m = html_module.HTMLModule(self, module_name, resource)
    self.loaded_modules[module_name] = m

    # Fake it, this is probably either polymer.min.js or platform.js which are
    # actually .js files....
    if resource.absolute_path.endswith('.js'):
      return m

    m.Parse(excluded_scripts)
    m.Load(excluded_scripts)
    return m

  def LoadRawScript(self, relative_raw_script_path):
    resource = None
    for source_path in self.source_paths:
      possible_absolute_path = os.path.join(
          source_path, os.path.normpath(relative_raw_script_path))
      if os.path.exists(possible_absolute_path):
        resource = resource_module.Resource(
            source_path, possible_absolute_path)
        break
    if not resource:
      raise module.DepsException(
          'Could not find a file for raw script %s in %s' %
          (relative_raw_script_path, self.source_paths))
    assert relative_raw_script_path == resource.unix_style_relative_path, (
        'Expected %s == %s' % (relative_raw_script_path,
                               resource.unix_style_relative_path))

    if resource.absolute_path in self.loaded_raw_scripts:
      return self.loaded_raw_scripts[resource.absolute_path]

    raw_script = module.RawScript(resource)
    self.loaded_raw_scripts[resource.absolute_path] = raw_script
    return raw_script

  def LoadStyleSheet(self, name):
    if name in self.loaded_style_sheets:
      return self.loaded_style_sheets[name]

    resource = self._FindResourceGivenNameAndSuffix(
        name, '.css', return_resource=True)
    if not resource:
      raise module.DepsException(
          'Could not find a file for stylesheet %s' % name)

    style_sheet = style_sheet_module.StyleSheet(self, name, resource)
    style_sheet.load()
    self.loaded_style_sheets[name] = style_sheet
    return style_sheet

  def LoadImage(self, abs_path):
    if abs_path in self.loaded_images:
      return self.loaded_images[abs_path]

    if not os.path.exists(abs_path):
      raise module.DepsException("url('%s') did not exist" % abs_path)

    res = self.FindResourceGivenAbsolutePath(abs_path, binary=True)
    if res is None:
      raise module.DepsException("url('%s') was not in search path" % abs_path)

    image = style_sheet_module.Image(res)
    self.loaded_images[abs_path] = image
    return image

  def GetStrippedJSForFilename(self, filename, early_out_if_no_py_vulcanize):
    if filename in self.stripped_js_by_filename:
      return self.stripped_js_by_filename[filename]

    with open(filename, 'r') as f:
      contents = f.read(4096)
    if early_out_if_no_py_vulcanize and ('py_vulcanize' not in contents):
      return None

    s = strip_js_comments.StripJSComments(contents)
    self.stripped_js_by_filename[filename] = s
    return s


def _read_file(absolute_path):
  """Reads a file and returns a (path, contents) pair.

  Args:
    absolute_path: Absolute path to a file.

  Raises:
    Exception: The given file doesn't exist.
    IOError: There was a problem opening or reading the file.
  """
  if not os.path.exists(absolute_path):
    raise Exception('%s not found.' % absolute_path)
  f = codecs.open(absolute_path, mode='r', encoding='utf-8')
  contents = f.read()
  f.close()
  return absolute_path, contents
