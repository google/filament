# Copyright (C) 2024 The Android Open Source Project
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

import subprocess
import os
import argparse
import sys
import pathlib

def execute(cmd,
            cwd=None,
            capture_output=True,
            stdin=None,
            env=None,
            raise_errors=False):
  in_env = os.environ
  in_env.update(env if env else {})
  home = os.environ['HOME']
  if f'{home}/bin' not in in_env['PATH']:
    in_env['PATH'] = in_env['PATH'] + f':{home}/bin'

  stdout = subprocess.PIPE if capture_output else sys.stdout
  stderr = subprocess.PIPE if capture_output else sys.stdout
  output = ''
  err_output = ''
  return_code = -1
  kwargs = {
      'cwd': cwd,
      'env': in_env,
      'stdout': stdout,
      'stderr': stderr,
      'stdin': stdin,
      'universal_newlines': True
  }
  if capture_output:
    process = subprocess.Popen(cmd.split(' '), **kwargs)
    output, err_output = process.communicate()
    return_code = process.returncode
  else:
    return_code = subprocess.call(cmd.split(' '), **kwargs)

  if return_code:
    # Error
    if raise_errors:
      raise subprocess.CalledProcessError(return_code, cmd)
  if output:
    if type(output) != str:
      try:
        output = output.decode('utf-8').strip()
      except UnicodeDecodeError as e:
        print('cannot decode ', output, file=sys.stderr)
  return return_code, (output if return_code == 0 else err_output)

class ArgParseImpl(argparse.ArgumentParser):
  def error(self, message):
    sys.stderr.write('error: %s\n' % message)
    self.print_help()
    sys.exit(1)


PROMPT_YES = 'y'
PROMPT_NO = 'n'
PROMPT_YES_NO = f'{PROMPT_YES}{PROMPT_NO}'

class GetCh:
  def __init__(self):
    pass

  def __call__(self):
    import sys, tty, termios
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    try:
      tty.setraw(sys.stdin.fileno())
      ch = sys.stdin.read(1)
    finally:
      termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
    return ch

getch = GetCh()

def prompt_helper(prompt_str, keys=PROMPT_YES_NO):
  while True:
    print(f'{prompt_str}: [' + ', '.join(keys) + '] => ', end='', flush=True)
    val = getch()
    print(val)
    if val in keys or ord(val) == 3: # If user pressed Ctrl+c
      if ord(val) == 3:
        exit(1)
    return val

def mkdir_p(path_str):
  pathlib.Path(path_str).mkdir(parents=True, exist_ok=True)

def mv_f(src_str, dst_str):
  src = pathlib.Path(src_str)
  src.replace(dst_str)

def important_print(msg):
  lines = msg.split('\n')
  max_len = max([len(l) for l in lines])
  print('-' * (max_len + 8))
  for line in lines:
    diff = max_len - len(line)
    information = f'--- {line} ' + (' ' * diff) + '---'
    print(information)
  print('-' * (max_len + 8))
