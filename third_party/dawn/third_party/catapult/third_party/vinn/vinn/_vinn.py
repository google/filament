#!/usr/bin/env python
# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import argparse
import logging
import os
import platform
import shutil
import stat
import subprocess
import sys
import time
import threading
import re
import json
import tempfile
import six
from six.moves import range


_V8_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), os.path.pardir, 'third_party',
                 'v8'))

_JS_PARSER_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), os.path.pardir, 'third_party',
                 'parse5', 'parse5.js'))


_BOOTSTRAP_JS_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), 'd8_bootstrap.js'))

_BASE64_COMPAT_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), 'base64_compat.js'))

_PATH_UTILS_JS_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), 'path_utils.js'))

_HTML_IMPORTS_LOADER_JS_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), 'html_imports_loader.js'))

_HTML_TO_JS_GENERATOR_JS_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), 'html_to_js_generator.js'))


_BOOTSTRAP_JS_CONTENT = None
_NUM_TRIALS = 3


def _ValidateSourcePaths(source_paths):
  if source_paths is None:
    return
  for x in source_paths:
    assert os.path.exists(x)
    assert os.path.isdir(x)
    assert os.path.isabs(x)


def _EscapeJsString(s):
  assert isinstance(s, str)
  return json.dumps(s)

def _RenderTemplateStringForJsSource(source, template, replacement_string):
  return source.replace(template, _EscapeJsString(replacement_string))


def _GetBootStrapJsContent(source_paths):
  assert isinstance(source_paths, list)
  global _BOOTSTRAP_JS_CONTENT
  if not _BOOTSTRAP_JS_CONTENT:
    with open(_BOOTSTRAP_JS_DIR, 'r') as f:
      _BOOTSTRAP_JS_CONTENT = f.read()

  bsc = _BOOTSTRAP_JS_CONTENT


  # Ensure that source paths are unique.
  source_paths = list(set(source_paths))
  source_paths_string = '[%s]' % (
      ','.join(_EscapeJsString(s) for s in source_paths))
  bsc = bsc.replace('<%source_paths%>', source_paths_string)
  bsc = _RenderTemplateStringForJsSource(
      bsc, '<%current_working_directory%>', os.getcwd())
  bsc = _RenderTemplateStringForJsSource(
      bsc, '<%path_utils_js_path%>', _PATH_UTILS_JS_DIR)
  bsc = _RenderTemplateStringForJsSource(
    bsc, '<%html_imports_loader_js_path%>', _HTML_IMPORTS_LOADER_JS_DIR)
  bsc = _RenderTemplateStringForJsSource(
      bsc, '<%html_to_js_generator_js_path%>', _HTML_TO_JS_GENERATOR_JS_DIR)
  bsc = _RenderTemplateStringForJsSource(
      bsc, '<%js_parser_path%>', _JS_PARSER_DIR)
  bsc = _RenderTemplateStringForJsSource(
      bsc, '<%base64_compat_path%>', _BASE64_COMPAT_DIR)
  bsc += '\n//@ sourceURL=%s\n' % _BOOTSTRAP_JS_DIR
  return bsc


def _IsValidJsOrHTMLFile(parser, js_file_arg):
  if not os.path.exists(js_file_arg):
    parser.error('The file %s does not exist' % js_file_arg)
  _, extension = os.path.splitext(js_file_arg)
  if extension not in ('.js', '.html'):
    parser.error('Input must be a JavaScript or HTML file')
  return js_file_arg


def _GetD8BinaryPathForPlatform():
  def _D8Path(*paths):
    """Join paths and make it executable."""
    assert isinstance(paths, tuple)
    exe = os.path.join(_V8_DIR, *paths)
    st = os.stat(exe)
    if not st.st_mode & stat.S_IEXEC:
      os.chmod(exe, st.st_mode | stat.S_IEXEC)
    return exe

  if platform.system() == 'Linux' and platform.machine() == 'x86_64':
    return _D8Path('linux', 'x86_64', 'd8')
  elif platform.system() == 'Linux' and platform.machine() == 'aarch64':
    return _D8Path('linux', 'arm', 'd8')
  elif platform.system() == 'Linux' and platform.machine() == 'armv7l':
    return _D8Path('linux', 'arm', 'd8')
  elif platform.system() == 'Linux' and platform.machine() == 'mips':
    return _D8Path('linux', 'mips', 'd8')
  elif platform.system() == 'Linux' and platform.machine() == 'mips64':
    return _D8Path('linux', 'mips64', 'd8')
  elif platform.system() == 'Darwin' and platform.machine() == 'x86_64':
    return _D8Path('mac', 'x86_64', 'd8')
  elif platform.system() == 'Darwin' and platform.machine() == 'arm64':
    return _D8Path('mac', 'arm', 'd8')
  elif platform.system() == 'Windows' and platform.machine() == 'AMD64':
    return _D8Path('win', 'AMD64', 'd8.exe')
  else:
    raise NotImplementedError(
        'd8 binary for this platform (%s) and architecture (%s) is not yet'
        ' supported' % (platform.system(), platform.machine()))


# Speculative change to workaround a failure on Windows: speculation is that the
# script attempts to remove a file before the process using the file has
# completely terminated. So the function here attempts to retry a few times with
# a second timeout between retries. More details at https://crbug.com/946012
# TODO(sadrul): delete this speculative change since it didn't work.
def _RemoveTreeWithRetry(tree, retry=3):
  for count in range(retry):
    try:
      shutil.rmtree(tree)
      return
    except:
      if count == retry - 1:
        raise
      logging.warning('Removing %s failed. Retrying in 1 second ...' % tree)
      time.sleep(1)


class RunResult(object):
  def __init__(self, returncode, stdout):
    self.returncode = returncode
    self.stdout = stdout


def ExecuteFile(file_path, source_paths=None, js_args=None, v8_args=None,
                stdout=subprocess.PIPE, stdin=subprocess.PIPE):
  """Execute JavaScript program in |file_path|.

  Args:
    file_path: string file_path that contains path the .js or .html file to be
      executed.
    source_paths: the list of absolute paths containing code. All the imports
    js_args: a list of string arguments to sent to the JS program.

  Args stdout & stdin are the same as _RunFileWithD8.

  Returns:
     The string output from running the JS program.
  """
  res = RunFile(file_path, source_paths, js_args, v8_args, None, stdout, stdin)
  return res.stdout


def RunFile(file_path, source_paths=None, js_args=None, v8_args=None,
            timeout=None, stdout=subprocess.PIPE, stdin=subprocess.PIPE):
  """Runs JavaScript program in |file_path|.

  Args are same as ExecuteFile.

  Returns:
     A RunResult containing the program's output.
  """
  assert os.path.isfile(file_path)
  _ValidateSourcePaths(source_paths)

  _, extension = os.path.splitext(file_path)
  if not extension in ('.html', '.js'):
    raise ValueError('Can only execute .js or .html file. File %s has '
                     'unsupported file type: %s' % (file_path, extension))
  if source_paths is None:
    source_paths = [os.path.dirname(file_path)]

  abs_file_path_str = _EscapeJsString(os.path.abspath(file_path))

  for trial in range(_NUM_TRIALS):
    try:
      temp_dir = tempfile.mkdtemp()
      temp_bootstrap_file = os.path.join(temp_dir, '_tmp_bootstrap.js')
      with open(temp_bootstrap_file, 'w') as f:
        f.write(_GetBootStrapJsContent(source_paths))
        if extension == '.html':
          f.write('\nHTMLImportsLoader.loadHTMLFile(%s, %s);' %
                  (abs_file_path_str, abs_file_path_str))
        else:
          f.write('\nHTMLImportsLoader.loadFile(%s);' % abs_file_path_str)
      result = _RunFileWithD8(temp_bootstrap_file, js_args, v8_args, timeout,
                              stdout, stdin)
    except:
      # Save the exception.
      t, v, tb = sys.exc_info()
      try:
        _RemoveTreeWithRetry(temp_dir)
      except:
        logging.error('Failed to remove temp dir %s.', temp_dir)
      if 'Error reading' in str(v):  # Handle crbug.com/953365
        if trial == _NUM_TRIALS - 1:
          logging.error(
              'Failed to run file with D8 after %s tries.', _NUM_TRIALS)
          six.reraise(t, v, tb)
        logging.warn('Hit error %s. Retrying after sleeping.', v)
        time.sleep(10)
        continue
      # Re-raise original exception.
      six.reraise(t, v, tb)
    _RemoveTreeWithRetry(temp_dir)
    break
  return result


def ExecuteJsString(js_string, source_paths=None, js_args=None, v8_args=None,
                     original_file_name=None, stdout=subprocess.PIPE,
                     stdin=subprocess.PIPE):
  res = RunJsString(js_string, source_paths, js_args, v8_args,
                    original_file_name, stdout, stdin)
  return res.stdout


def RunJsString(js_string, source_paths=None, js_args=None, v8_args=None,
                original_file_name=None, stdout=subprocess.PIPE,
                stdin=subprocess.PIPE):
  _ValidateSourcePaths(source_paths)

  try:
    temp_dir = tempfile.mkdtemp()
    if original_file_name:
      name = os.path.basename(original_file_name)
      name, _ = os.path.splitext(name)
      temp_file = os.path.join(temp_dir, '%s.js' % name)
    else:
      temp_file = os.path.join(temp_dir, 'temp_program.js')
    with open(temp_file, 'w') as f:
      f.write(js_string)
    result = RunFile(temp_file, source_paths, js_args, v8_args, None, stdout,
                     stdin)
  except:
    # Save the exception.
    t, v, tb = sys.exc_info()
    try:
      _RemoveTreeWithRetry(temp_dir)
    except:
      logging.error('Failed to remove temp dir %s.', temp_dir)
    # Re-raise original exception.
    six.reraise(t, v, tb)
  _RemoveTreeWithRetry(temp_dir)
  return result


def _KillProcess(process, name, reason):
  # kill() does not close the handle to the process. On Windows, a process
  # will live until you delete all handles to that subprocess, so
  # ps_util.ListAllSubprocesses will find this subprocess if
  # we haven't garbage-collected the handle yet. poll() should close the
  # handle once the process dies.
  logging.warn('Killing process %s because %s.', name, reason)
  process.kill()
  time.sleep(.01)
  for _ in range(100):
    if process.poll() is None:
      time.sleep(.1)
      continue
    break
  else:
    logging.warn('process %s is still running after we '
                 'attempted to kill it.', name)


def _RunFileWithD8(js_file_path, js_args, v8_args, timeout, stdout, stdin):
  """ Execute the js_files with v8 engine and return the output of the program.

  Args:
    js_file_path: the string path of the js file to be run.
    js_args: a list of arguments to passed to the |js_file_path| program.
    v8_args: extra arguments to pass into d8. (for the full list of these
      options, run d8 --help)
    timeout: how many seconds to wait for d8 to finish. If None or 0 then
      this will wait indefinitely.
    stdout: where to pipe the stdout of the executed program to. If
      subprocess.PIPE is used, stdout will be returned in RunResult.out.
      Otherwise RunResult.out is None
    stdin: specify the executed program's input.
  """
  if v8_args is None:
    v8_args = []
  assert isinstance(v8_args, list)
  args = [_GetD8BinaryPathForPlatform()] + v8_args
  args.append(os.path.abspath(js_file_path))
  full_js_args = [args[0]]
  if js_args:
    full_js_args += js_args

  args += ['--'] + full_js_args

  # Set stderr=None since d8 doesn't write into stderr anyway.
  sp = subprocess.Popen(args, stdout=stdout, stderr=None, stdin=stdin)
  if timeout:
    deadline = time.time() + timeout
    timeout_thread = threading.Timer(timeout, _KillProcess, args=(
        sp, 'd8', 'it timed out'))
    timeout_thread.start()
  out, _ = sp.communicate()
  if timeout:
    timeout_thread.cancel()

  # On Windows, d8's print() method add the carriage return characters \r to
  # newline, which make the output different from d8 on posix. We remove the
  # extra \r's  to make the output consistent with posix platforms.
  if platform.system() == 'Windows' and out:
    out = re.sub(b'\r+\n', b'\n', six.ensure_binary(out))

  # d8 uses returncode 1 to indicate an uncaught exception, but
  # _RunFileWithD8 needs to distingiush between that and quit(1).
  #
  # To fix this, d8_bootstrap.js monkeypatches D8's quit function to
  # adds 1 to an intentioned nonzero quit. So, now, we have to undo this
  # logic here in order to raise/return the right thing.
  returncode = sp.returncode
  if returncode == 0:
    return RunResult(0, out)
  elif returncode == 1:
    if out:
      raise RuntimeError(
        'Exception raised when executing %s:\n%s' % (js_file_path, out))
    else:
      raise RuntimeError(
        'Exception raised when executing %s. '
        '(Error stack is dumped into stdout)' % js_file_path)
  else:
    return RunResult(returncode - 1, out)


def main():
  parser = argparse.ArgumentParser(
      description='Run JavaScript file with v8 engine')
  parser.add_argument('file_name', help='input file', metavar='FILE',
                      type=lambda f: _IsValidJsOrHTMLFile(parser, f))
  parser.add_argument('--js_args', help='arguments for the js program',
                      nargs='+')
  parser.add_argument('--source_paths', help='search path for the js program',
                      nargs='+', type=str)

  args = parser.parse_args()
  if args.source_paths:
    args.source_paths = [os.path.abspath(x) for x in args.source_paths]
  else:
    args.source_paths = [os.path.abspath(os.path.dirname(args.file_name))]
    logging.warning(
      '--source_paths is not specified. Use %s for search path.' %
      args.source_paths)
  res = RunFile(args.file_name, source_paths=args.source_paths,
                js_args=args.js_args, timeout=None, stdout=sys.stdout,
                stdin=sys.stdin)
  return res.returncode
