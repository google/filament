# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import unittest

from py_vulcanize import generate
from py_vulcanize import fake_fs
from py_vulcanize import project as project_module


class GenerateTests(unittest.TestCase):

  def setUp(self):
    self.fs = fake_fs.FakeFS()
    self.fs.AddFile(
        '/x/foo/my_module.html',
        ('<!DOCTYPE html>\n'
         '<link rel="import" href="/foo/other_module.html">\n'))
    self.fs.AddFile(
        '/x/foo/other_module.html',
        ('<!DOCTYPE html>\n'
         '<script src="/foo/raw/raw_script.js"></script>\n'
         '<script>\n'
         '  \'use strict\';\n'
         '  HelloWorld();\n'
         '</script>\n'))
    self.fs.AddFile('/x/foo/raw/raw_script.js', '\n/* raw script */\n')
    self.fs.AddFile('/x/components/polymer/polymer.min.js', '\n')

    self.fs.AddFile('/x/foo/external_script.js', 'External()')
    self.fs.AddFile('/x/foo/inline_and_external_module.html',
                    ('<!DOCTYPE html>\n'
                    '<script>Script1()</script>'
                    '<script src=/foo/external_script.js></script>'
                    '<script>Script2()</script>'))

    self.project = project_module.Project([os.path.normpath('/x')])

  def testJSGeneration(self):
    with self.fs:
      load_sequence = self.project.CalcLoadSequenceForModuleNames(
          [os.path.normpath('foo.my_module')])
      generate.GenerateJS(load_sequence)

  def testHTMLGeneration(self):
    with self.fs:
      load_sequence = self.project.CalcLoadSequenceForModuleNames(
          [os.path.normpath('foo.my_module')])
      result = generate.GenerateStandaloneHTMLAsString(load_sequence)
      self.assertIn('HelloWorld();', result)

  def testExtraScriptWithWriteContentsFunc(self):
    with self.fs:
      load_sequence = self.project.CalcLoadSequenceForModuleNames(
          [os.path.normpath('foo.my_module')])

      class ExtraScript(generate.ExtraScript):

        def WriteToFile(self, f):
          f.write('<script>ExtraScript!</script>')

      result = generate.GenerateStandaloneHTMLAsString(
          load_sequence, title='Title', extra_scripts=[ExtraScript()])
      self.assertIn('ExtraScript', result)

  def testScriptOrdering(self):
    with self.fs:
      load_sequence = self.project.CalcLoadSequenceForModuleNames(
          [os.path.normpath('foo.inline_and_external_module')])
      result = generate.GenerateStandaloneHTMLAsString(load_sequence)
      script1_pos = result.index('Script1()')
      script2_pos = result.index('Script2()')
      external_pos = result.index('External()')
      self.assertTrue(script1_pos < external_pos < script2_pos)

  def testScriptOrderingWithIncludeTag(self):
    with self.fs:
      load_sequence = self.project.CalcLoadSequenceForModuleNames(
          [os.path.normpath('foo.inline_and_external_module')])
      result = generate.GenerateJS(load_sequence,
                                   use_include_tags_for_scripts = True,
      dir_for_include_tag_root='/x/')
      script1_pos = result.index('Script1()')
      script2_pos = result.index('Script2()')
      external_path = os.path.join('foo', 'external_script.js')
      external_pos = result.index('<include src="{0}">'.format(external_path))
      self.assertTrue(script1_pos < external_pos < script2_pos)
