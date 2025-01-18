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
