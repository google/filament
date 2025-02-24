# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
import unittest

from py_vulcanize import fake_fs
from py_vulcanize import generate
from py_vulcanize import html_generation_controller
from py_vulcanize import html_module
from py_vulcanize import parse_html_deps
from py_vulcanize import project as project_module
from py_vulcanize import resource
from py_vulcanize import resource_loader as resource_loader
import functools
import six



class ResourceWithFakeContents(resource.Resource):

  def __init__(self, toplevel_dir, absolute_path, fake_contents):
    """A resource with explicitly provided contents.

    If the resource does not exist, then pass fake_contents=None. This will
    cause accessing the resource contents to raise an exception mimicking the
    behavior of regular resources."""
    super(ResourceWithFakeContents, self).__init__(toplevel_dir, absolute_path)
    self._fake_contents = fake_contents

  @property
  def contents(self):
    if self._fake_contents is None:
      raise Exception('File not found')
    return self._fake_contents


class FakeLoader(object):

  def __init__(self, source_paths, initial_filenames_and_contents=None):
    self._source_paths = source_paths
    self._file_contents = {}
    if initial_filenames_and_contents:
      for k, v in six.iteritems(initial_filenames_and_contents):
        self._file_contents[k] = v

  def FindResourceGivenAbsolutePath(self, absolute_path):
    candidate_paths = []
    for source_path in self._source_paths:
      if absolute_path.startswith(source_path):
        candidate_paths.append(source_path)
    if len(candidate_paths) == 0:
      return None

    # Sort by length. Longest match wins.
    sorted(candidate_paths,
           key=functools.cmp_to_key(lambda x, y: len(x) - len(y)), reverse=True)

    longest_candidate = candidate_paths[-1]

    return ResourceWithFakeContents(
        longest_candidate, absolute_path,
        self._file_contents.get(absolute_path, None))

  def FindResourceGivenRelativePath(self, relative_path):
    absolute_path = None
    for script_path in self._source_paths:
      absolute_path = os.path.join(script_path, relative_path)
      if absolute_path in self._file_contents:
        return ResourceWithFakeContents(script_path, absolute_path,
                                        self._file_contents[absolute_path])
    return None


class ParseTests(unittest.TestCase):

  def testValidExternalScriptReferenceToRawScript(self):
    parse_results = parse_html_deps.HTMLModuleParserResults("""<!DOCTYPE html>
      <script src="../foo.js">
      """)

    file_contents = {}
    file_contents[os.path.normpath('/tmp/a/foo.js')] = """
'i am just some raw script';
"""

    metadata = html_module.Parse(
        FakeLoader([os.path.normpath('/tmp')], file_contents),
        'a.b.start',
        '/tmp/a/b/',
        is_component=False,
        parser_results=parse_results)
    self.assertEquals([], metadata.dependent_module_names)
    self.assertEquals(
        ['a/foo.js'], metadata.dependent_raw_script_relative_paths)

  def testExternalScriptReferenceToModuleOutsideScriptPath(self):
    parse_results = parse_html_deps.HTMLModuleParserResults("""<!DOCTYPE html>
      <script src="/foo.js">
      """)

    file_contents = {}
    file_contents[os.path.normpath('/foo.js')] = ''

    def DoIt():
      html_module.Parse(FakeLoader([os.path.normpath('/tmp')], file_contents),
                        'a.b.start',
                        '/tmp/a/b/',
                        is_component=False,
                        parser_results=parse_results)
    self.assertRaises(Exception, DoIt)

  def testExternalScriptReferenceToFileThatDoesntExist(self):
    parse_results = parse_html_deps.HTMLModuleParserResults("""<!DOCTYPE html>
      <script src="/foo.js">
      """)

    file_contents = {}

    def DoIt():
      html_module.Parse(FakeLoader([os.path.normpath('/tmp')], file_contents),
                        'a.b.start',
                        '/tmp/a/b/',
                        is_component=False,
                        parser_results=parse_results)
    self.assertRaises(Exception, DoIt)

  def testValidImportOfModule(self):
    parse_results = parse_html_deps.HTMLModuleParserResults("""<!DOCTYPE html>
      <link rel="import" href="../foo.html">
      """)

    file_contents = {}
    file_contents[os.path.normpath('/tmp/a/foo.html')] = """
"""

    metadata = html_module.Parse(
        FakeLoader([os.path.normpath('/tmp')], file_contents),
        'a.b.start',
        '/tmp/a/b/',
        is_component=False,
        parser_results=parse_results)
    self.assertEquals(['a.foo'], metadata.dependent_module_names)

  def testStyleSheetImport(self):
    parse_results = parse_html_deps.HTMLModuleParserResults("""<!DOCTYPE html>
      <link rel="stylesheet" href="../foo.css">
      """)

    file_contents = {}
    file_contents[os.path.normpath('/tmp/a/foo.css')] = """
"""
    metadata = html_module.Parse(
        FakeLoader([os.path.normpath('/tmp')], file_contents),
        'a.b.start',
        '/tmp/a/b/',
        is_component=False,
        parser_results=parse_results)
    self.assertEquals([], metadata.dependent_module_names)
    self.assertEquals(['a.foo'], metadata.style_sheet_names)

  def testUsingAbsoluteHref(self):
    parse_results = parse_html_deps.HTMLModuleParserResults("""<!DOCTYPE html>
      <script src="/foo.js">
      """)

    file_contents = {}
    file_contents[os.path.normpath('/src/foo.js')] = ''

    metadata = html_module.Parse(
        FakeLoader([os.path.normpath("/tmp"), os.path.normpath("/src")],
                   file_contents),
        "a.b.start",
        "/tmp/a/b/",
        is_component=False,
        parser_results=parse_results)
    self.assertEquals(['foo.js'], metadata.dependent_raw_script_relative_paths)


class HTMLModuleTests(unittest.TestCase):

  def testBasicModuleGeneration(self):
    file_contents = {}
    file_contents[os.path.normpath('/tmp/a/b/start.html')] = """
<!DOCTYPE html>
<link rel="import" href="/widget.html">
<link rel="stylesheet" href="../common.css">
<script src="/raw_script.js"></script>
<script src="/excluded_script.js"></script>
<dom-module id="start">
  <template>
  </template>
  <script>
    'use strict';
    console.log('inline script for start.html got written');
  </script>
</dom-module>
"""
    file_contents[os.path.normpath('/py_vulcanize/py_vulcanize.html')] = """<!DOCTYPE html>
"""
    file_contents[os.path.normpath('/components/widget.html')] = """
<!DOCTYPE html>
<link rel="import" href="/py_vulcanize.html">
<widget name="widget.html"></widget>
<script>
'use strict';
console.log('inline script for widget.html');
</script>
"""
    file_contents[os.path.normpath('/tmp/a/common.css')] = """
/* /tmp/a/common.css was written */
"""
    file_contents[os.path.normpath('/raw/raw_script.js')] = """
console.log('/raw/raw_script.js was written');
"""
    file_contents[os.path.normpath(
        '/raw/components/polymer/polymer.min.js')] = """
"""

    with fake_fs.FakeFS(file_contents):
      project = project_module.Project(
          [os.path.normpath('/py_vulcanize/'),
           os.path.normpath('/tmp/'),
           os.path.normpath('/components/'),
           os.path.normpath('/raw/')])
      loader = resource_loader.ResourceLoader(project)
      a_b_start_module = loader.LoadModule(
          module_name='a.b.start', excluded_scripts=['\/excluded_script.js'])
      load_sequence = project.CalcLoadSequenceForModules([a_b_start_module])

      # Check load sequence names.
      load_sequence_names = [x.name for x in load_sequence]
      self.assertEquals(['py_vulcanize',
                         'widget',
                         'a.b.start'], load_sequence_names)

      # Check module_deps on a_b_start_module
      def HasDependentModule(module, name):
        return [x for x in module.dependent_modules
                if x.name == name]
      assert HasDependentModule(a_b_start_module, 'widget')

      # Check JS generation.
      js = generate.GenerateJS(load_sequence)
      assert 'inline script for start.html' in js
      assert 'inline script for widget.html' in js
      assert '/raw/raw_script.js' in js
      assert 'excluded_script.js' not in js

      # Check HTML generation.
      html = generate.GenerateStandaloneHTMLAsString(
          load_sequence, title='', flattened_js_url='/blah.js')
      assert '<dom-module id="start">' in html
      assert 'inline script for widget.html' not in html
      assert 'common.css' in html

  def testPolymerConversion(self):
    file_contents = {}
    file_contents[os.path.normpath('/tmp/a/b/my_component.html')] = """
<!DOCTYPE html>
<dom-module id="my-component">
  <template>
  </template>
  <script>
    'use strict';
    Polymer ( {
      is: "my-component"
    });
  </script>
</dom-module>
"""
    with fake_fs.FakeFS(file_contents):
      project = project_module.Project([
          os.path.normpath('/py_vulcanize/'), os.path.normpath('/tmp/')])
      loader = resource_loader.ResourceLoader(project)
      my_component = loader.LoadModule(module_name='a.b.my_component')

      f = six.StringIO()
      my_component.AppendJSContentsToFile(
          f,
          use_include_tags_for_scripts=False,
          dir_for_include_tag_root=None)
      js = f.getvalue().rstrip()
      expected_js = """
    'use strict';
    Polymer ( {
      is: "my-component"
    });
""".rstrip()
      self.assertEquals(expected_js, js)

  def testInlineStylesheetURLs(self):
    file_contents = {}
    file_contents[os.path.normpath('/tmp/a/b/my_component.html')] = """
<!DOCTYPE html>
<style>
.some-rule {
    background-image: url('../something.jpg');
}
</style>
"""
    file_contents[os.path.normpath('/tmp/a/something.jpg')] = b'jpgdata'
    with fake_fs.FakeFS(file_contents):
      project = project_module.Project([
          os.path.normpath('/py_vulcanize/'), os.path.normpath('/tmp/')])
      loader = resource_loader.ResourceLoader(project)
      my_component = loader.LoadModule(module_name='a.b.my_component')

      computed_deps = []
      my_component.AppendDirectlyDependentFilenamesTo(computed_deps)
      self.assertEquals(set(computed_deps),
                        set([os.path.normpath('/tmp/a/b/my_component.html'),
                             os.path.normpath('/tmp/a/something.jpg')]))

      f = six.StringIO()
      ctl = html_generation_controller.HTMLGenerationController()
      my_component.AppendHTMLContentsToFile(f, ctl)
      html = f.getvalue().rstrip()
      # FIXME: This is apparently not used.
      expected_html = """
.some-rule {
    background-image: url(data:image/jpg;base64,anBnZGF0YQ==);
}
""".rstrip()
