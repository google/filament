# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import base64
import os
import unittest

from py_vulcanize import project as project_module
from py_vulcanize import resource_loader
from py_vulcanize import fake_fs
from py_vulcanize import module


class StyleSheetUnittest(unittest.TestCase):

  def testImages(self):
    fs = fake_fs.FakeFS()
    fs.AddFile('/src/foo/x.css', """
.x .y {
    background-image: url(../images/bar.jpeg);
}
""")
    fs.AddFile('/src/images/bar.jpeg', b'hello world')
    with fs:
      project = project_module.Project([os.path.normpath('/src/')])
      loader = resource_loader.ResourceLoader(project)

      foo_x = loader.LoadStyleSheet('foo.x')
      self.assertEquals(1, len(foo_x.images))

      r0 = foo_x.images[0]
      self.assertEquals(os.path.normpath('/src/images/bar.jpeg'),
                        r0.absolute_path)

      inlined = foo_x.contents_with_inlined_images
      self.assertEquals("""
.x .y {
    background-image: url(data:image/jpeg;base64,%s);
}
""" % base64.standard_b64encode(b'hello world').decode('utf-8'), inlined)

  def testURLResolveFails(self):
    fs = fake_fs.FakeFS()
    fs.AddFile('/src/foo/x.css', """
.x .y {
    background-image: url(../images/missing.jpeg);
}
""")
    with fs:
      project = project_module.Project([os.path.normpath('/src')])
      loader = resource_loader.ResourceLoader(project)

      self.assertRaises(module.DepsException,
                        lambda: loader.LoadStyleSheet('foo.x'))

  def testImportsCauseFailure(self):
    fs = fake_fs.FakeFS()
    fs.AddFile('/src/foo/x.css', """
@import url(awesome.css);
""")
    with fs:
      project = project_module.Project([os.path.normpath('/src')])
      loader = resource_loader.ResourceLoader(project)

      self.assertRaises(Exception,
                        lambda: loader.LoadStyleSheet('foo.x'))
