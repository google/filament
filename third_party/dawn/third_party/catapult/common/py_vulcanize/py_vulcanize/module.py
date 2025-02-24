# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""This module contains the Module class and other classes for resources.

The Module class represents a module in the trace viewer system. A module has
a name, and may require a variety of other resources, such as stylesheets,
template objects, raw JavaScript, or other modules.

Other resources include HTML templates, raw JavaScript files, and stylesheets.
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import codecs
import inspect
import os

from py_vulcanize import js_utils
import six


class DepsException(Exception):
  """Exceptions related to module dependency resolution."""

  def __init__(self, fmt, *args):
    from py_vulcanize import style_sheet as style_sheet_module
    context = []
    frame = inspect.currentframe()
    while frame:
      frame_locals = frame.f_locals

      module_name = None
      if 'self' in frame_locals:
        s = frame_locals['self']
        if isinstance(s, Module):
          module_name = s.name
        if isinstance(s, style_sheet_module.StyleSheet):
          module_name = s.name + '.css'
      if not module_name:
        if 'module' in frame_locals:
          module = frame_locals['module']
          if isinstance(s, Module):
            module_name = module.name
        elif 'm' in frame_locals:
          module = frame_locals['m']
          if isinstance(s, Module):
            module_name = module.name

      if module_name:
        if len(context):
          if context[-1] != module_name:
            context.append(module_name)
        else:
          context.append(module_name)

      frame = frame.f_back

    context.reverse()
    self.context = context
    context_str = '\n'.join('  %s' % x for x in context)
    Exception.__init__(
        self, 'While loading:\n%s\nGot: %s' % (context_str, (fmt % args)))


class ModuleDependencyMetadata(object):

  def __init__(self):
    self.dependent_module_names = []
    self.dependent_raw_script_relative_paths = []
    self.style_sheet_names = []

  def AppendMetdata(self, other):
    self.dependent_module_names += other.dependent_module_names
    self.dependent_raw_script_relative_paths += \
        other.dependent_raw_script_relative_paths
    self.style_sheet_names += other.style_sheet_names


_next_module_id = 1


class Module(object):
  """Represents a JavaScript module.

  Interesting properties include:
    name: Module name, may include a namespace, e.g. 'py_vulcanize.foo'.
    filename: The filename of the actual module.
    contents: The text contents of the module.
    dependent_modules: Other modules that this module depends on.

  In addition to these properties, a Module also contains lists of other
  resources that it depends on.
  """

  def __init__(self, loader, name, resource, load_resource=True):
    assert isinstance(name, six.string_types), 'Got %s instead' % repr(name)

    global _next_module_id
    self._id = _next_module_id
    _next_module_id += 1

    self.loader = loader
    self.name = name
    self.resource = resource

    if load_resource:
      f = codecs.open(self.filename, mode='r', encoding='utf-8')
      self.contents = f.read()
      f.close()
    else:
      self.contents = None

    # Dependency metadata, set up during Parse().
    self.dependency_metadata = None

    # Actual dependencies, set up during load().
    self.dependent_modules = []
    self.dependent_raw_scripts = []
    self.scripts = []
    self.style_sheets = []

    # Caches.
    self._all_dependent_modules_recursive = None

  def __repr__(self):
    return '%s(%s)' % (self.__class__.__name__, self.name)

  @property
  def id(self):
    return self._id

  @property
  def filename(self):
    return self.resource.absolute_path

  def IsThirdPartyComponent(self):
    """Checks whether this module is a third-party Polymer component."""
    if os.path.join('third_party', 'components') in self.filename:
      return True
    if os.path.join('third_party', 'polymer', 'components') in self.filename:
      return True
    return False

  def Parse(self, excluded_scripts):
    """Parses self.contents and fills in the module's dependency metadata."""
    raise NotImplementedError()

  def GetTVCMDepsModuleType(self):
    """Returns the py_vulcanize.setModuleInfo type for this module"""
    raise NotImplementedError()

  def AppendJSContentsToFile(self,
                             f,
                             use_include_tags_for_scripts,
                             dir_for_include_tag_root):
    """Appends the js for this module to the provided file."""
    for script in self.scripts:
      script.AppendJSContentsToFile(f, use_include_tags_for_scripts,
                                    dir_for_include_tag_root)

  def AppendHTMLContentsToFile(self, f, ctl, minify=False):
    """Appends the HTML for this module [without links] to the provided file."""
    pass

  def Load(self, excluded_scripts=None):
    """Loads the sub-resources that this module depends on from its dependency
    metadata.

    Raises:
      DepsException: There was a problem finding one of the dependencies.
      Exception: There was a problem parsing a module that this one depends on.
    """
    assert self.name, 'Module name must be set before dep resolution.'
    assert self.filename, 'Module filename must be set before dep resolution.'
    assert self.name in self.loader.loaded_modules, (
        'Module must be registered in resource loader before loading.')

    metadata = self.dependency_metadata
    for name in metadata.dependent_module_names:
      module = self.loader.LoadModule(module_name=name,
                                      excluded_scripts=excluded_scripts)
      self.dependent_modules.append(module)

    for name in metadata.style_sheet_names:
      style_sheet = self.loader.LoadStyleSheet(name)
      self.style_sheets.append(style_sheet)

  @property
  def all_dependent_modules_recursive(self):
    if self._all_dependent_modules_recursive:
      return self._all_dependent_modules_recursive

    self._all_dependent_modules_recursive = set(self.dependent_modules)
    for dependent_module in self.dependent_modules:
      self._all_dependent_modules_recursive.update(
          dependent_module.all_dependent_modules_recursive)
    return self._all_dependent_modules_recursive

  def ComputeLoadSequenceRecursive(self, load_sequence, already_loaded_set,
                                   depth=0):
    """Recursively builds up a load sequence list.

    Args:
      load_sequence: A list which will be incrementally built up.
      already_loaded_set: A set of modules that has already been added to the
          load sequence list.
      depth: The depth of recursion. If it too deep, that indicates a loop.
    """
    if depth > 32:
      raise Exception('Include loop detected on %s', self.name)
    for dependent_module in self.dependent_modules:
      if dependent_module.name in already_loaded_set:
        continue
      dependent_module.ComputeLoadSequenceRecursive(
          load_sequence, already_loaded_set, depth + 1)
    if self.name not in already_loaded_set:
      already_loaded_set.add(self.name)
      load_sequence.append(self)

  def GetAllDependentFilenamesRecursive(self, include_raw_scripts=True):
    dependent_filenames = []

    visited_modules = set()

    def Get(module):
      module.AppendDirectlyDependentFilenamesTo(
          dependent_filenames, include_raw_scripts)
      visited_modules.add(module)
      for m in module.dependent_modules:
        if m in visited_modules:
          continue
        Get(m)

    Get(self)
    return dependent_filenames

  def AppendDirectlyDependentFilenamesTo(
      self, dependent_filenames, include_raw_scripts=True):
    dependent_filenames.append(self.resource.absolute_path)
    if include_raw_scripts:
      for raw_script in self.dependent_raw_scripts:
        dependent_filenames.append(raw_script.resource.absolute_path)
    for style_sheet in self.style_sheets:
      style_sheet.AppendDirectlyDependentFilenamesTo(dependent_filenames)


class RawScript(object):
  """Represents a raw script resource referenced by a module via the
  py_vulcanize.requireRawScript(xxx) directive."""

  def __init__(self, resource):
    self.resource = resource

  @property
  def filename(self):
    return self.resource.absolute_path

  @property
  def contents(self):
    return self.resource.contents

  def __repr__(self):
    return 'RawScript(%s)' % self.filename
