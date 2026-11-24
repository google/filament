# Copyright (C) 2025 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import os
import glob
import json
import yaml
import hashlib
import concurrent.futures
import re
import subprocess
import sys
import tempfile

from utils import ArgParseImpl

def get_line(file_name, offset):
  with open(f'{file_name}', 'rb') as file:
    bytes = file.read()[0:offset]
    f_str = bytes.decode('utf-8')
    return len(f_str.split('\n'))
  return -1

def get_func_name(msg):
  pattern = r"\'(.+)\'"
  res = re.findall(pattern, msg)
  if len(res) > 0:
    return res[0].replace("'", '')
  return msg

def run_tidy(file, fixes_dir):
  fixes_path = os.path.join(fixes_dir, hashlib.md5(file.encode('utf-8')).hexdigest() + '.yaml')
  subprocess.run(
      ['clang-tidy', f'--export-fixes={fixes_path}', '--quiet',
       '--header-filter=.*/filament/.*',
       '--checks=-*,bugprone-exception-escape', file],
      stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
  # clang-tidy only writes the fixes file when it has at least one diagnostic to report.
  if not os.path.exists(fixes_path):
    return []
  results = []
  with open(fixes_path, 'r') as f:
    data = yaml.safe_load(f) or {}
    for d in data.get('Diagnostics') or []:
      if d['DiagnosticName'] != 'bugprone-exception-escape':
        continue
      msg = d['DiagnosticMessage']
      fpath = msg['FilePath']
      offset = msg['FileOffset']
      line_num = get_line(fpath, offset)
      results.append((msg['FilePath'].replace(f'{os.getcwd()}/', ''), line_num, get_func_name(msg['Message'])))
  return results

DEFAULT_SRC_GLOBS=[
  'filament/**/*.mm',
  'filament/**/*.cpp',
]

def can_analyze(path):
  # Objective-C++ and the Metal backend include Apple framework headers, so they only compile,
  # and can only be analyzed, on an Apple host.
  if sys.platform == 'darwin':
    return True
  return not path.endswith('.mm') and '/metal/' not in path

def load_compiled_files(cwd):
  compdb_path = os.path.join(cwd, 'compile_commands.json')
  if not os.path.exists(compdb_path):
    return None
  try:
    with open(compdb_path, 'r') as f:
      entries = json.load(f)
    compiled = set()
    cwd_real = os.path.realpath(cwd)
    for entry in entries:
      fpath = entry.get('file')
      if not fpath:
        continue
      if not os.path.isabs(fpath):
        fpath = os.path.join(entry.get('directory', cwd), fpath)
      compiled.add(os.path.relpath(os.path.realpath(fpath), cwd_real))
    return compiled
  except Exception as exc:
    print(f'Warning: failed to read compile_commands.json: {exc}')
    return None

def load_changed_files(changed_files_file):
  with open(changed_files_file, 'r') as f:
    raw = f.read()
  sep = '\0' if '\0' in raw else '\n'
  return [os.path.normpath(p.strip()) for p in raw.split(sep) if p.strip()]

def exception_escape_test(src_globs=DEFAULT_SRC_GLOBS, jobs=None, changed_files_file=None):
  files = set()
  if not isinstance(src_globs, list):
    src_globs = [src_globs]
  for i in src_globs:
    files.update(os.path.normpath(p) for p in glob.glob(i, recursive=True))
  files = [f for f in files if can_analyze(f)]

  compiled_files = load_compiled_files(os.getcwd())
  if compiled_files is not None:
    files = [f for f in files if f in compiled_files]

  if changed_files_file:
    changed = [f for f in load_changed_files(changed_files_file)
               if f == 'filament' or f.startswith('filament/')]
    if not changed:
      print('Skipping exception-escape: no files under filament/ changed.')
      return (True, [])
    has_header_change = any(f.endswith(('.h', '.hpp', '.inc')) for f in changed)
    if not has_header_change:
      changed_set = set(changed)
      files = [f for f in files if f in changed_set]
      if not files:
        print('Skipping exception-escape: no active translation units under filament/ changed.')
        return (True, [])

  # One clang-tidy process per file, handed out as workers free up, keeps every core busy until
  # the queue drains. Largest files go first, since they tend to take longest and would otherwise
  # be the ones still running after everything else has finished.
  files.sort(key=lambda f: os.path.getsize(f), reverse=True)
  num_workers = max(1, int(jobs) if jobs else (os.cpu_count() or 1))

  all_results = set()
  with tempfile.TemporaryDirectory() as fixes_dir, \
      concurrent.futures.ThreadPoolExecutor(max_workers=num_workers) as executor:
    future_to_file = {executor.submit(run_tidy, f, fixes_dir): f for f in files}

    for future in concurrent.futures.as_completed(future_to_file):
      try:
        # A set, because a diagnostic in a header can be reported once for every file that
        # includes it.
        all_results.update(future.result())
      except Exception as exc:
          print(f"Main: Analyzing {future_to_file[future]} generated an exception: {exc}")
  test_name = 'code-correctness::exception-escape'
  failure_str_lines = []
  if len(all_results) > 0:
    all_results = sorted(all_results)
    failure_str_lines.append(f'Number of failures: {len(all_results)}')
    for fname, line_num, msg in all_results:
      failure_str_lines.append(f'{fname}({line_num}): {msg}()')
  return (len(all_results) == 0, failure_str_lines)

TESTS = [
  (
    exception_escape_test,
    # Test name
    'exception-escape',

    # Test description
    'An exception may be thrown in a function which should not throw exceptions. '
    'Consider adding \'NOLINT(bugprone-exception-escape)\' for valid suppression this check.',

    # Maps a command line argument to a function argument
    [('exception_escape_globs', 'src_globs'), ('jobs', 'jobs'),
     ('changed_files_file', 'changed_files_file')],
  )
]

if __name__ == "__main__":
  parser = argparse.ArgumentParser()

  for f, name, desc, args in TESTS:
    for cmd_arg_name, _ in args:
      parser.add_argument(f'--{cmd_arg_name}', required=False)

  args = parser.parse_args()

  has_failures = False
  for test_func, test_name, test_desc, arguments in TESTS:
    func_args = {}
    for cmdline_arg, func_arg in arguments:
      arg_val = getattr(args, cmdline_arg, None)
      if arg_val is not None:
        func_args[func_arg] = arg_val.split(',') if ',' in arg_val else arg_val

    result, res_strs = test_func(**func_args)

    ss = ' ' * 4
    if result:
      print(f'[{test_name}] PASSED')
    else:
      has_failures = True
      print(f'[{test_name}] FAILED')
      print(f'{ss}Description: \'{test_desc}\'')
      for s in res_strs:
        print(f'{ss}{s}')
  if has_failures:
    # TODO: Enable this when we've fixed all the exception-escape errors
    #exit(1)
    pass
