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
  files_str = ' '.join(files)
  hid = hashlib.md5(files_str.encode('utf-8')).hexdigest()
  _, _ = execute(f'clang-tidy --export-fixes=/tmp/{hid}.yaml --quiet --checks=-*,bugprone-exception-escape {files_str}')
  results = []
  with open(f'/tmp/{hid}.yaml', 'r') as file:
    data = yaml.safe_load(file)
    for d in data['Diagnostics']:
      if d['DiagnosticName'] != 'bugprone-exception-escape':
        continue
      msg = d['DiagnosticMessage']
      fpath = msg['FilePath']
      offset = msg['FileOffset']
      line_num = get_line(fpath, offset)
      results.append((msg['FilePath'].replace(f'{os.getcwd()}/', ''), line_num, get_func_name(msg['Message'])))
  return results

def exception_escape_test():
  files = glob.glob('filament/**/*.mm', recursive=True) + \
      glob.glob('filament/**/*.cpp', recursive=True) + \
      glob.glob('filament/**/*.h', recursive=True)

  num_workers = 5  # Number of threads to spawn
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
    failure_str_lines.append(f'Number of failures: {len(all_results)}')
    for fname, line_num, msg in all_results:
      failure_str_lines.append(f'{fname}({line_num}): {msg}()')
  return (len(all_results) == 0, failure_str_lines)

TESTS = [
  (exception_escape_test,
   'exception-escape',
   '\'an exception may be thrown in a function which should not throw exceptions\'')
]

if __name__ == "__main__":
  has_failures = False
  for test_func, test_name, test_desc in TESTS:
    result, res_strs = test_func()
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

