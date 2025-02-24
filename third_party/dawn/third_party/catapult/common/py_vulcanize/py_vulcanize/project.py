# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
import collections
import os

try:
  from six import StringIO
except ImportError:
  from io import StringIO

from py_vulcanize import resource_loader
import six


def _FindAllFilesRecursive(source_paths):
  all_filenames = set()
  for source_path in source_paths:
    for dirpath, _, filenames in os.walk(source_path):
      for f in filenames:
        if f.startswith('.'):
          continue
        x = os.path.abspath(os.path.join(dirpath, f))
        all_filenames.add(x)
  return all_filenames


class AbsFilenameList(object):

  def __init__(self, willDirtyCallback):
    self._willDirtyCallback = willDirtyCallback
    self._filenames = []
    self._filenames_set = set()

  def _WillBecomeDirty(self):
    if self._willDirtyCallback:
      self._willDirtyCallback()

  def append(self, filename):
    assert os.path.isabs(filename)
    self._WillBecomeDirty()
    self._filenames.append(filename)
    self._filenames_set.add(filename)

  def extend(self, iterable):
    self._WillBecomeDirty()
    for filename in iterable:
      assert os.path.isabs(filename)
      self._filenames.append(filename)
      self._filenames_set.add(filename)

  def appendRel(self, basedir, filename):
    assert os.path.isabs(basedir)
    self._WillBecomeDirty()
    n = os.path.abspath(os.path.join(basedir, filename))
    self._filenames.append(n)
    self._filenames_set.add(n)

  def extendRel(self, basedir, iterable):
    self._WillBecomeDirty()
    assert os.path.isabs(basedir)
    for filename in iterable:
      n = os.path.abspath(os.path.join(basedir, filename))
      self._filenames.append(n)
      self._filenames_set.add(n)

  def __contains__(self, x):
    return x in self._filenames_set

  def __len__(self):
    return self._filenames.__len__()

  def __iter__(self):
    return iter(self._filenames)

  def __repr__(self):
    return repr(self._filenames)

  def __str__(self):
    return str(self._filenames)


class Project(object):

  py_vulcanize_path = os.path.abspath(os.path.join(
      os.path.dirname(__file__), '..'))

  def __init__(self, source_paths=None):
    """
    source_paths: A list of top-level directories in which modules and raw
        scripts can be found. Module paths are relative to these directories.
    """
    self._loader = None
    self._frozen = False
    self.source_paths = AbsFilenameList(self._WillPartOfPathChange)

    if source_paths is not None:
      self.source_paths.extend(source_paths)

  def Freeze(self):
    self._frozen = True

  def _WillPartOfPathChange(self):
    if self._frozen:
      raise Exception('The project is frozen. You cannot edit it now')
    self._loader = None

  @staticmethod
  def FromDict(d):
    return Project(d['source_paths'])

  def AsDict(self):
    return {
        'source_paths': list(self.source_paths)
    }

  def __repr__(self):
    return "Project(%s)" % repr(self.source_paths)

  def AddSourcePath(self, path):
    self.source_paths.append(path)

  @property
  def loader(self):
    if self._loader is None:
      self._loader = resource_loader.ResourceLoader(self)
    return self._loader

  def ResetLoader(self):
    self._loader = None

  def _Load(self, filenames):
    return [self.loader.LoadModule(module_filename=filename) for
            filename in filenames]

  def LoadModule(self, module_name=None, module_filename=None):
    return self.loader.LoadModule(module_name=module_name,
                                  module_filename=module_filename)

  def CalcLoadSequenceForModuleNames(self, module_names,
                                     excluded_scripts=None):
    modules = [self.loader.LoadModule(module_name=name,
                                      excluded_scripts=excluded_scripts) for
               name in module_names]
    return self.CalcLoadSequenceForModules(modules)

  def CalcLoadSequenceForModules(self, modules):
    already_loaded_set = set()
    load_sequence = []
    for m in modules:
      m.ComputeLoadSequenceRecursive(load_sequence, already_loaded_set)
    return load_sequence

  def GetDepsGraphFromModuleNames(self, module_names):
    modules = [self.loader.LoadModule(module_name=name) for
               name in module_names]
    return self.GetDepsGraphFromModules(modules)

  def GetDepsGraphFromModules(self, modules):
    load_sequence = self.CalcLoadSequenceForModules(modules)
    g = _Graph()
    for m in load_sequence:
      g.AddModule(m)

      for dep in m.dependent_modules:
        g.AddEdge(m, dep.id)

    # FIXME: _GetGraph is not defined. Maybe `return g` is intended?
    return _GetGraph(load_sequence)

  def GetDominatorGraphForModulesNamed(self, module_names, load_sequence):
    modules = [self.loader.LoadModule(module_name=name)
               for name in module_names]
    return self.GetDominatorGraphForModules(modules, load_sequence)

  def GetDominatorGraphForModules(self, start_modules, load_sequence):
    modules_by_id = {}
    for m in load_sequence:
      modules_by_id[m.id] = m

    module_referrers = collections.defaultdict(list)
    for m in load_sequence:
      for dep in m.dependent_modules:
        module_referrers[dep].append(m)

    # Now start at the top module and reverse.
    visited = set()
    g = _Graph()

    pending = collections.deque()
    pending.extend(start_modules)
    while len(pending):
      cur = pending.pop()

      g.AddModule(cur)
      visited.add(cur)

      for out_dep in module_referrers[cur]:
        if out_dep in visited:
          continue
        g.AddEdge(out_dep, cur)
        visited.add(out_dep)
        pending.append(out_dep)

    # Visited -> Dot
    return g.GetDot()


class _Graph(object):

  def __init__(self):
    self.nodes = []
    self.edges = []

  def AddModule(self, m):
    f = StringIO()
    m.AppendJSContentsToFile(f, False, None)

    attrs = {
        'label': '%s (%i)' % (m.name, f.tell())
    }

    f.close()

    attr_items = ['%s="%s"' % (x, y) for x, y in six.iteritems(attrs)]
    node = 'M%i [%s];' % (m.id, ','.join(attr_items))
    self.nodes.append(node)

  def AddEdge(self, mFrom, mTo):
    edge = 'M%i -> M%i;' % (mFrom.id, mTo.id)
    self.edges.append(edge)

  def GetDot(self):
    return 'digraph deps {\n\n%s\n\n%s\n}\n' % (
        '\n'.join(self.nodes), '\n'.join(self.edges))
