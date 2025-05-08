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
import yaml
import hashlib
import concurrent.futures
import re

from utils import execute, ArgParseImpl

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

def run_tidy(files):
  if len(files) == 0:
    return []
  files_str = ' '.join(files)
  hid = hashlib.md5(files_str.encode('utf-8')).hexdigest()
  _, _ = execute(f'clang-tidy --export-fixes=/tmp/{hid}.yaml --quiet --checks=-*,bugprone-exception-escape {files_str}')
  results = []
  with open(f'/tmp/{hid}.yaml', 'r') as f:
    data = yaml.safe_load(f)
    for d in data['Diagnostics']:
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
  'filament/**/*.h',
]

def exception_escape_test(src_globs=DEFAULT_SRC_GLOBS):
  files = []
  if not isinstance(src_globs, list):
    src_globs = [src_globs]
  for i in src_globs:
    files += glob.glob(i, recursive=True)

  num_workers = min(len(files), 5)  # Number of threads to spawn
  part_len = len(files) // num_workers
  workloads = []
  for i in range(num_workers):
    next = min(len(files), part_len)
    workloads.append(files[0:next])
    files = files[next:]

  all_results = []
  with concurrent.futures.ThreadPoolExecutor(max_workers=num_workers) as executor:
    future_to_worker_id = {executor.submit(run_tidy, workloads[i]): i for i in range(num_workers)}

    for future in concurrent.futures.as_completed(future_to_worker_id):
      worker_id = future_to_worker_id[future]
      try:
        all_results.extend(future.result())
      except Exception as exc:
          print(f"Main: Worker {worker_id} generated an exception: {exc}")
  test_name = 'code-correctness::exception-escape'
  failure_str_lines = []
  if len(all_results) > 0:
    all_results.sort(key=lambda x: x[0])
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
    [('exception_escape_globs', 'src_globs')],
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
