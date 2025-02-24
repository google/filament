# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import json
import os
import re
import sys
import shutil
import tempfile
import unittest
import six
from six.moves import range

from unittest import mock

import vinn
from vinn import _vinn

def _EscapeJsString(s):
  return json.dumps(s)


class VinnUnittest(unittest.TestCase):

  @classmethod
  def setUpClass(cls):
    cls.test_data_dir = os.path.abspath(
        os.path.join(os.path.dirname(__file__), 'test_data'))

  def GetTestFilePath(self, file_name):
    return os.path.join(self.test_data_dir, file_name)

  def AssertHasNamedFrame(self, func_name, file_and_linum, exception_message):
    m = re.search('at %s.+\(.*%s.*\)' % (func_name, file_and_linum),
                  exception_message)
    if not m:
      sys.stderr.write('\n=============================================\n')
      msg = "Expected to find %s and %s" % (func_name, file_and_linum)
      sys.stderr.write('%s\n' % msg)
      sys.stderr.write('=========== Begin Exception Message =========\n')
      sys.stderr.write(exception_message)
      sys.stderr.write('=========== End Exception Message =========\n\n')
      self.fail(msg)

  def AssertHasFrame(self, file_and_linum, exception_message):
    m = re.search('at .*%s.*' % file_and_linum, exception_message)
    if not m:
      sys.stderr.write('\n=============================================\n')
      msg = "Expected to find %s" % file_and_linum
      sys.stderr.write('%s\n' % msg)
      sys.stderr.write('=========== Begin Exception Message =========\n')
      sys.stderr.write(exception_message)
      sys.stderr.write('=========== End Exception Message =========\n\n')
      self.fail(msg)

  def _GetUnescapedExceptionMessage(self, exception):
    if os.name == 'nt':
      return str(exception)
    else:
      return str(exception).encode().decode("unicode-escape")


  def testExecuteJsStringStdoutPiping(self):
    tmp_dir = tempfile.mkdtemp()
    try:
      temp_file_name = os.path.join(tmp_dir, 'out_file')
      with open(temp_file_name, 'w') as f:
        vinn.ExecuteJsString(
            'print("Hello w0rld");\n', stdout=f)
      with open(temp_file_name, 'r') as f:
        self.assertEquals(f.read(), 'Hello w0rld\n')
    finally:
      shutil.rmtree(tmp_dir)

  def testRunJsStringStdoutPiping(self):
    tmp_dir = tempfile.mkdtemp()
    try:
      temp_file_name = os.path.join(tmp_dir, 'out_file')
      with open(temp_file_name, 'w') as f:
        vinn.RunJsString(
            'print("Hello w0rld");\n', stdout=f)
      with open(temp_file_name, 'r') as f:
        self.assertEquals(f.read(), 'Hello w0rld\n')
    finally:
      shutil.rmtree(tmp_dir)

  def testExecuteFileStdoutPiping(self):
    file_path = self.GetTestFilePath('simple.js')
    tmp_dir = tempfile.mkdtemp()
    try:
      temp_file_name = os.path.join(tmp_dir, 'out_file')
      with open(temp_file_name, 'w') as f:
        vinn.ExecuteFile(file_path, stdout=f)
      with open(temp_file_name, 'r') as f:
        self.assertEquals(f.read(), 'Hello W0rld from simple.js\n')
    finally:
      shutil.rmtree(tmp_dir)

  def testRunFileStdoutPiping(self):
    file_path = self.GetTestFilePath('simple.js')
    tmp_dir = tempfile.mkdtemp()
    try:
      temp_file_name = os.path.join(tmp_dir, 'out_file')
      with open(temp_file_name, 'w') as f:
        vinn.RunFile(file_path, stdout=f)
      with open(temp_file_name, 'r') as f:
        self.assertEquals(f.read(), 'Hello W0rld from simple.js\n')
    finally:
      shutil.rmtree(tmp_dir)

  def testSimpleJsExecution(self):
    file_path = self.GetTestFilePath('print_file_content.js')
    dummy_test_path = self.GetTestFilePath('dummy_test_file')
    output = vinn.ExecuteFile(file_path, source_paths=[self.test_data_dir],
                                   js_args=[dummy_test_path]).decode('utf-8')
    self.assertIn(
        'This is file contains only data for testing.\n1 2 3 4', output)

  def testDuplicateSourcePaths(self):
    output = vinn.ExecuteJsString(
      "HTMLImportsLoader.loadHTML('/load_simple_html.html');",
      source_paths=[self.test_data_dir]*100).decode('utf-8')
    self.assertIn(
        'load_simple_html.html is loaded', output)

  def testJsFileLoadHtmlFile(self):
    file_path = self.GetTestFilePath('load_simple_html.js')
    output = vinn.ExecuteFile(file_path, source_paths=[self.test_data_dir]).decode('utf-8')
    expected_output = ('File foo.html is loaded\n'
                       'x = 1\n'
                       "File foo.html's second script is loaded\n"
                       'x = 2\n'
                       'load_simple_html.js is loaded\n')
    self.assertEquals(output, expected_output)

  def testJsFileLoadJsFile(self):
    file_path = self.GetTestFilePath('load_simple_js.js')
    output = vinn.ExecuteFile(file_path, source_paths=[self.test_data_dir]).decode('utf-8')
    expected_output = ('bar.js is loaded\n'
                       'load_simple_js.js is loaded\n')
    self.assertEquals(output, expected_output)

  def testHTMLFileLoadHTMLFile(self):
    file_path = self.GetTestFilePath('load_simple_html.html')
    output = vinn.ExecuteFile(
        file_path, source_paths=[self.test_data_dir]).decode('utf-8')
    expected_output = ('File foo.html is loaded\n'
                       'x = 1\n'
                       "File foo.html's second script is loaded\n"
                       'x = 2\n'
                       'bar.js is loaded\n'
                       'File load_simple_html.html is loaded\n')
    self.assertEquals(output, expected_output)

  def testQuit0Handling(self):
    file_path = self.GetTestFilePath('quit_0_test.js')
    res = vinn.RunFile(file_path, source_paths=[self.test_data_dir])
    self.assertEquals(res.returncode, 0)

  def testQuit1Handling(self):
    file_path = self.GetTestFilePath('quit_1_test.js')
    res = vinn.RunFile(file_path, source_paths=[self.test_data_dir])
    self.assertEquals(res.returncode, 1)

  def testQuit42Handling(self):
    file_path = self.GetTestFilePath('quit_42_test.js')
    res = vinn.RunFile(file_path, source_paths=[self.test_data_dir])
    self.assertEquals(res.returncode, 42)

  def testQuit274Handling(self):
    file_path = self.GetTestFilePath('quit_274_test.js')
    res = vinn.RunFile(file_path, source_paths=[self.test_data_dir])
    self.assertEquals(res.returncode, 238)

  def testErrorStackTraceJs(self):
    file_path = self.GetTestFilePath('error_stack_test.js')
    # error_stack_test.js imports load_simple_html.html
    # load_simple_html.html imports foo.html
    # foo.html imports error.js
    # error.js defines maybeRaiseException() method that can raise exception
    # foo.html defines maybeRaiseExceptionInFoo() method that calls
    # maybeRaiseException()
    # Finally, we call maybeRaiseExceptionInFoo() error_stack_test.js
    # Exception log should capture these method calls' stack trace.
    with self.assertRaises(RuntimeError) as context:
      vinn.ExecuteFile(file_path, source_paths=[self.test_data_dir])

    # Assert error stack trace contain src files' info.
    exception_message = self._GetUnescapedExceptionMessage(context.exception)
    self.assertIn(
        ('error.js:7: Error: Throw ERROR'), exception_message)
    self.assertIn(
        ("    throw new Error('Throw ERROR');"), exception_message)
    self.AssertHasNamedFrame('maybeRaiseException', 'error.js:7',
                             exception_message)
    self.AssertHasNamedFrame('global.maybeRaiseExceptionInFoo', 'foo.html:34',
                             exception_message)
    self.AssertHasFrame('error_stack_test.js:14', exception_message)

  def testErrorStackTraceHTML(self):
    file_path = self.GetTestFilePath('error_stack_test.html')
    # error_stack_test.html imports error_stack_test.js
    # error_stack_test.js imports load_simple_html.html
    # load_simple_html.html imports foo.html
    # foo.html imports error.js
    # error.js defines maybeRaiseException() method that can raise exception
    # foo.html defines maybeRaiseExceptionInFoo() method that calls
    # maybeRaiseException()
    # Finally, we call maybeRaiseExceptionInFoo() error_stack_test.js
    # Exception log should capture these method calls' stack trace.
    with self.assertRaises(RuntimeError) as context:
      vinn.ExecuteFile(file_path, source_paths=[self.test_data_dir])

    # Assert error stack trace contain src files' info.
    exception_message = self._GetUnescapedExceptionMessage(context.exception)
    self.assertIn(
        ('error.js:7: Error: Throw ERROR'), exception_message)
    self.assertIn(
        ("    throw new Error('Throw ERROR');"), exception_message)

    self.AssertHasNamedFrame('maybeRaiseException', 'error.js:7',
                             exception_message)
    self.AssertHasNamedFrame('global.maybeRaiseExceptionInFoo', 'foo.html:34',
                             exception_message)
    self.AssertHasFrame('error_stack_test.js:14', exception_message)
    self.AssertHasNamedFrame('eval', 'error_stack_test.html:22',
                             exception_message)

  def testStackTraceOfErroWhenLoadingHTML(self):
    file_path = self.GetTestFilePath('load_error.html')
    with self.assertRaises(RuntimeError) as context:
      vinn.ExecuteFile(file_path, source_paths=[self.test_data_dir])

    # Assert error stack trace contain src files' info.
    exception_message = self._GetUnescapedExceptionMessage(context.exception)

    self.assertIn('Error: /does_not_exist.html not found', exception_message)
    self.AssertHasNamedFrame('eval', 'load_error_2.html:21', exception_message)
    self.AssertHasNamedFrame('eval', 'load_error.html:23', exception_message)

  def testStackTraceOfErroWhenSyntaxErrorOccurs(self):
    file_path = self.GetTestFilePath('syntax_error.html')
    with self.assertRaises(RuntimeError) as context:
      vinn.ExecuteFile(file_path, source_paths=[self.test_data_dir])

    # Assert error stack trace contain src files' info.
    exception_message = self._GetUnescapedExceptionMessage(context.exception)

    self.assertIn('syntax_error.html:23: SyntaxError: Unexpected identifier',
                  exception_message)

  def testStackTraceOfErroWhenLoadingJS(self):
    file_path = self.GetTestFilePath('load_js_error.html')
    with self.assertRaises(RuntimeError) as context:
      vinn.ExecuteFile(file_path, source_paths=[self.test_data_dir])

    # Assert error stack trace contain src files' info.
    exception_message = self._GetUnescapedExceptionMessage(context.exception)

    self.assertIn('Error: /does_not_exist.js not found', exception_message)
    self.AssertHasNamedFrame('eval', 'load_js_error_2.html:20',
                             exception_message)
    self.AssertHasNamedFrame('eval', 'load_js_error.html:22',
                             exception_message)

  def testStrictError(self):
    file_path = self.GetTestFilePath('non_strict_error.html')
    with self.assertRaises(RuntimeError) as context:
      vinn.ExecuteFile(file_path, source_paths=[self.test_data_dir])

    # Assert error stack trace contain src files' info.
    exception_message = self._GetUnescapedExceptionMessage(context.exception)

    self.assertIn('non_defined_variable is not defined', exception_message)
    self.AssertHasNamedFrame('eval', 'non_strict_error.html:17',
                             exception_message)

  def testConsolePolyfill(self):
    self.assertEquals(
        vinn.ExecuteJsString(
            'console.log("hello", "world");').decode('utf-8'),
        'hello world\n')
    self.assertEquals(
        vinn.ExecuteJsString(
            'console.info("hello", "world");').decode('utf-8'),
        'Info: hello world\n')
    self.assertEquals(
        vinn.ExecuteJsString(
            'console.warn("hello", "world");').decode('utf-8'),
        'Warning: hello world\n')
    self.assertEquals(
        vinn.ExecuteJsString(
            'console.error("hello", "world");').decode('utf-8'),
        'Error: hello world\n')

  def testConsoleTimeEndAssertion(self):
    file_path = self.GetTestFilePath('console_time_test.js')
    try:
      vinn.ExecuteFile(file_path)
    except RuntimeError:
      self.fail()

  def testConsoleTime(self):
    self.assertEquals(
        vinn.ExecuteJsString('console.time("AA")').decode('utf-8'),
        '')

  @unittest.skip('crbug.com/1315752')
  def testConsoleTimeEndOutput(self):
    output = vinn.ExecuteJsString(
        'console.time("AA");console.timeEnd("AA")').decode('utf-8')
    m = re.search('\d+\.\d+', output)
    if not m:
      sys.stderr.write('\nExpected to find output of timer AA')
      self.fail()
    a_duration = float(m.group())
    self.assertTrue(a_duration > 0.0)
    output = vinn.ExecuteJsString("""console.time("BB");
                                     console.time("CC");
                                     console.timeEnd("CC");
                                     console.timeEnd("BB")""").decode('utf-8')
    m = re.findall('(\d+\.\d+)', output)
    if not m:
      sys.stderr.write('\nExpected to find output of timer\n')
      self.fail()
    c_duration = float(m[0])
    b_duration = float(m[1])
    self.assertTrue(b_duration > c_duration)

  def testRetries(self):
    file_path = self.GetTestFilePath('load_simple_js.js')
    def RaiseRuntimeError(*args, **kwargs):
      del args, kwargs
      raise RuntimeError('Error reading "blah blah"')
    with mock.patch('vinn._vinn._RunFileWithD8') as mock_d8_runner, \
        mock.patch('time.sleep'):
      mock_d8_runner.side_effect = RaiseRuntimeError
      with self.assertRaises(RuntimeError):
        vinn.ExecuteFile(file_path, source_paths=[self.test_data_dir])
      self.assertEqual(len(mock_d8_runner.mock_calls), _vinn._NUM_TRIALS)


class PathUtilUnittest(unittest.TestCase):
  def testPathUtil(self):
    path_util_js_test = os.path.abspath(os.path.join(
        os.path.dirname(__file__), 'path_utils_test.js'))
    path_utils_js_dir = os.path.abspath(
        os.path.join(os.path.dirname(__file__), 'path_utils.js'))

    test_loading_js = """
    load(%s);
    load(%s);
    runTests();
    """ % (_EscapeJsString(path_utils_js_dir),
           _EscapeJsString(path_util_js_test))

    res = vinn.RunJsString(test_loading_js)
    self.assertEquals(res.returncode, 0)


def _GetLineNumberOfSubstring(content, substring):
  """ Return the line number of |substring| in |content|."""
  index = content.index(substring)
  return content[:index].count('\n') + 1


def _GenerateLineByLineDiff(actual, expected):
  results = []
  expected_lines = expected.split('\n')
  actual_lines = actual.split('\n')
  max_num_lines = max(len(expected_lines), len(actual_lines))
  results.append('**Actual    : num lines =  %i' % len(actual_lines))
  results.append('**Expected  : num lines = %i' % len(expected_lines))

  for i in range(0, max_num_lines):
    expected_current_line = expected_lines[i] if i < len(expected_lines) else ''
    actual_current_line = actual_lines[i] if i < len(actual_lines) else ''
    if actual_current_line == expected_current_line:
      continue
    results.append('================= Line %s ======================' % (i + 1))
    results.append('**Actual    : %s' % repr(actual_current_line))
    results.append('**Expected  : %s' % repr(expected_current_line))
  return '\n'.join(results)


class HTMLGeneratorTest(unittest.TestCase):

  def AssertStringEquals(self, actual, expected):
    if actual != expected:
      message = 'Expected %s but got %s.\n' % (repr(expected), repr(actual))
      message += _GenerateLineByLineDiff(actual, expected)
      self.fail(message)

  def GetGeneratedJs(self, html_text):
    tmp_dir = tempfile.mkdtemp()
    try:
      temp_file_name = os.path.join(tmp_dir, 'test.html')
      with open(temp_file_name, 'w') as f:
        f.write(html_text)
      return vinn.ExecuteJsString(
          'write(generateJsFromHTML(read(%s)));' %
          _EscapeJsString(temp_file_name)).decode('utf-8')
    finally:
      shutil.rmtree(tmp_dir)

  def testGenerateJsForD8RunnerSimpleHTMLImport(self):
    html = '<link rel="import" href="/base/math.html">'
    expected_js = "global.HTMLImportsLoader.loadHTML('/base/math.html');"
    self.AssertStringEquals(self.GetGeneratedJs(html), expected_js)

  def testGenerateJSForD8RunnerImportMultilineHTMLImport(self):
    html = """
          <link rel="import"
          href="/base/math.html">"""
    expected_js = "\nglobal.HTMLImportsLoader.loadHTML('/base/math.html');"
    self.AssertStringEquals(self.GetGeneratedJs(html),
                            expected_js)

  def testGenerateJsForD8RunnerImportSimpleScriptWithSrc(self):
    html = '<script src="/base/math.js"></script>'
    expected_js = "global.HTMLImportsLoader.loadScript('/base/math.js');"
    self.AssertStringEquals(self.GetGeneratedJs(html),
                            expected_js)

  def testGenerateJsForD8RunnerImportMultilineScriptWithSrc(self):
    html = """<script
                  type="text/javascript"
                  src="/base/math.js">
                  </script>"""
    expected_js = """global.HTMLImportsLoader.loadScript('/base/math.js');


                  """
    self.AssertStringEquals(self.GetGeneratedJs(html),
                            expected_js)

  def testGenerateJsForD8RunnerWithMixedMultipleImport(self):
    html = """
<link rel="import" href="/base.html"><link rel="import" href="/base64.html">
<link rel="import"
            href="/base/math.html"><script
            type="text/javascript"

            src="/base/3d.js">
            </script>

<script src="/base/math.js"></script>

 <link rel="import"
  href="/base/random.html">
"""
    expected_js = ("""
global.HTMLImportsLoader.loadHTML('/base.html');global.HTMLImportsLoader.loadHTML('/base64.html');
global.HTMLImportsLoader.loadHTML('/base/math.html');
global.HTMLImportsLoader.loadScript('/base/3d.js');



            """ + """

global.HTMLImportsLoader.loadScript('/base/math.js');

global.HTMLImportsLoader.loadHTML('/base/random.html');""")
    self.AssertStringEquals(self.GetGeneratedJs(html),
                            expected_js)

  def testGenerateJsForD8RunnerImportWithSimpleContent(self):
    html = """<script>
               var html_lines = [
                '<script>',
                '< /script>',
               ];
    </script>
    """
    expected_js = """
               var html_lines = [
                '<script>',
                '< /script>',
               ];
    """
    self.AssertStringEquals(self.GetGeneratedJs(html),
                            expected_js)

  def testGenerateJsForD8RunnerImportWithEscapedScriptTag(self):
    html = """<script>
var s = ("<") + "<\/script>";
var x = 100;
    </script>
    """
    expected_js = """
var s = ("<") + "<\/script>";
var x = 100;
    """
    self.AssertStringEquals(self.GetGeneratedJs(html),
                            expected_js)

  def testGenerateJsForD8RunnerImportWithSrcAndSimpleContent(self):
    html = """<script
               src="/base.js">var html_lines = [
                '<script>',
                '< /script>',
               ];
    </script>
    """
    expected_js = """global.HTMLImportsLoader.loadScript('/base.js');
var html_lines = [
                '<script>',
                '< /script>',
               ];
    """
    self.AssertStringEquals(self.GetGeneratedJs(html),
                            expected_js)

  def testGenerateJsForD8RunnerImportComplex(self):
    html = """<!DOCTYPE html>
<!--
Copyright (c) 2014 The Chromium Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
-->
        <link rel="import" href="/base/math.html"><script>var x = 1;</script>
        <script src="/base/computer.js">
          var linux = os.system;  // line number of this is 9
        </script>
        <link rel="import" href="/base/physics.html">

        <script>
              var html_lines = [
                '<script>',
                '< /script>',
              ];
              function foo() {
                var y = [
                  1,
                  2,   // line number of this is 21
                  3,
                  4
                ];
              }
        </script>

         <link rel="import" href="/base/this_is_line_28.html">
         <script>
          var i = '<link rel="import" href="/base/math.html">';
         </script>
        """
    expected_js = """





global.HTMLImportsLoader.loadHTML('/base/math.html');var x = 1;
global.HTMLImportsLoader.loadScript('/base/computer.js');
          var linux = os.system;  // line number of this is 9
        """ + """
global.HTMLImportsLoader.loadHTML('/base/physics.html');


              var html_lines = [
                '<script>',
                '< /script>',
              ];
              function foo() {
                var y = [
                  1,
                  2,   // line number of this is 21
                  3,
                  4
                ];
              }
        """ + """

global.HTMLImportsLoader.loadHTML('/base/this_is_line_28.html');

          var i = '<link rel="import" href="/base/math.html">';
         """

    generated_js = self.GetGeneratedJs(html)
    self.AssertStringEquals(
        _GetLineNumberOfSubstring(generated_js, '// line number of this is 9'),
        9)
    self.AssertStringEquals(
        _GetLineNumberOfSubstring(generated_js, '// line number of this is 21'),
        21)
    self.AssertStringEquals(
        _GetLineNumberOfSubstring(generated_js, 'this_is_line_28.html'),
        28)
    self.AssertStringEquals(generated_js, expected_js)


class VinnV8ArgsTest(unittest.TestCase):

  @classmethod
  def setUpClass(cls):
    cls.test_data_dir = os.path.abspath(
        os.path.join(os.path.dirname(__file__), 'test_data'))

  def GetTestFilePath(self, file_name):
    return os.path.join(self.test_data_dir, file_name)

  def setUp(self):
    self.patcher = mock.patch('vinn._vinn.subprocess.Popen')
    self.mock_popen = self.patcher.start()
    mock_rv = mock.Mock()
    mock_rv.returncode = 0
    # Communicate() returns [stdout, stderr]
    mock_rv.communicate.return_value = ['', None]
    self.mock_popen.return_value = mock_rv

  def tearDown(self):
    mock.patch.stopall()

  def testRunJsStringWithV8Args(self):
    vinn.RunJsString('var x = 1;', v8_args=['--foo', '--bar=True'])
    v8_args = self.mock_popen.call_args[0][0]
    self.assertIn('--foo', v8_args)
    self.assertIn('--bar=True', v8_args)

  def testExecuteJsStringWithV8Args(self):
    vinn.ExecuteJsString('var x = 1;', v8_args=['--foo', '--bar=True'])
    v8_args = self.mock_popen.call_args[0][0]
    self.assertIn('--foo', v8_args)
    self.assertIn('--bar=True', v8_args)

  def testRunFileWithV8Args(self):
    file_path = self.GetTestFilePath('simple.js')
    vinn.RunFile(file_path, v8_args=['--foo', '--bar=True'])
    v8_args = self.mock_popen.call_args[0][0]
    self.assertIn('--foo', v8_args)

  def testExecuteFileWithV8Args(self):
    file_path = self.GetTestFilePath('simple.js')
    vinn.ExecuteFile(file_path, v8_args=['--foo', '--bar=True'])
    v8_args = self.mock_popen.call_args[0][0]
    self.assertIn('--foo', v8_args)
    self.assertIn('--bar=True', v8_args)
