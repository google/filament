#!/usr/bin/env python
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Tests for the module module, which contains Module and related classes."""

from __future__ import absolute_import
import os
import unittest

from py_vulcanize import fake_fs
from py_vulcanize import module
from py_vulcanize import resource_loader
from py_vulcanize import project as project_module


class ModuleIntegrationTests(unittest.TestCase):

  def test_module(self):
    fs = fake_fs.FakeFS()
    fs.AddFile('/src/x.html', """
<!DOCTYPE html>
<link rel="import" href="/y.html">
<link rel="import" href="/z.html">
<script>
'use strict';
</script>
""")
    fs.AddFile('/src/y.html', """
<!DOCTYPE html>
<link rel="import" href="/z.html">
""")
    fs.AddFile('/src/z.html', """
<!DOCTYPE html>
""")
    fs.AddFile('/src/py_vulcanize.html', '<!DOCTYPE html>')
    with fs:
      project = project_module.Project([os.path.normpath('/src/')])
      loader = resource_loader.ResourceLoader(project)
      x_module = loader.LoadModule('x')

      self.assertEquals([loader.loaded_modules['y'],
                         loader.loaded_modules['z']],
                        x_module.dependent_modules)

      already_loaded_set = set()
      load_sequence = []
      x_module.ComputeLoadSequenceRecursive(load_sequence, already_loaded_set)

      self.assertEquals([loader.loaded_modules['z'],
                         loader.loaded_modules['y'],
                         x_module],
                        load_sequence)

  def testBasic(self):
    fs = fake_fs.FakeFS()
    fs.AddFile('/x/src/my_module.html', """
<!DOCTYPE html>
<link rel="import" href="/py_vulcanize/foo.html">
});
""")
    fs.AddFile('/x/py_vulcanize/foo.html', """
<!DOCTYPE html>
});
""")
    project = project_module.Project([os.path.normpath('/x')])
    loader = resource_loader.ResourceLoader(project)
    with fs:
      my_module = loader.LoadModule(module_name='src.my_module')
      dep_names = [x.name for x in my_module.dependent_modules]
      self.assertEquals(['py_vulcanize.foo'], dep_names)

  def testDepsExceptionContext(self):
    fs = fake_fs.FakeFS()
    fs.AddFile('/x/src/my_module.html', """
<!DOCTYPE html>
<link rel="import" href="/py_vulcanize/foo.html">
""")
    fs.AddFile('/x/py_vulcanize/foo.html', """
<!DOCTYPE html>
<link rel="import" href="missing.html">
""")
    project = project_module.Project([os.path.normpath('/x')])
    loader = resource_loader.ResourceLoader(project)
    with fs:
      exc = None
      try:
        loader.LoadModule(module_name='src.my_module')
        assert False, 'Expected an exception'
      except module.DepsException as e:
        exc = e
      self.assertEquals(
          ['src.my_module', 'py_vulcanize.foo'],
          exc.context)

  def testGetAllDependentFilenamesRecursive(self):
    fs = fake_fs.FakeFS()
    fs.AddFile('/x/y/z/foo.html', """
<!DOCTYPE html>
<link rel="import" href="/z/foo2.html">
<link rel="stylesheet" href="/z/foo.css">
<script src="/bar.js"></script>
""")
    fs.AddFile('/x/y/z/foo.css', """
.x .y {
    background-image: url(foo.jpeg);
}
""")
    fs.AddFile('/x/y/z/foo.jpeg', '')
    fs.AddFile('/x/y/z/foo2.html', """
<!DOCTYPE html>
""")
    fs.AddFile('/x/raw/bar.js', 'hello')
    project = project_module.Project([
        os.path.normpath('/x/y'), os.path.normpath('/x/raw/')])
    loader = resource_loader.ResourceLoader(project)
    with fs:
      my_module = loader.LoadModule(module_name='z.foo')
      self.assertEquals(1, len(my_module.dependent_raw_scripts))

      dependent_filenames = my_module.GetAllDependentFilenamesRecursive()
      self.assertEquals(
          [
              os.path.normpath('/x/y/z/foo.html'),
              os.path.normpath('/x/raw/bar.js'),
              os.path.normpath('/x/y/z/foo.css'),
              os.path.normpath('/x/y/z/foo.jpeg'),
              os.path.normpath('/x/y/z/foo2.html'),
          ],
          dependent_filenames)
