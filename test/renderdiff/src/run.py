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

import sys
import os

from utils import execute, ArgParseImpl
from parse_test_json import parse_test_config_from_path

def important_print(msg):
  lines = msg.split('\n')
  max_len = max([len(l) for l in lines])
  print('-' * (max_len + 8))
  for line in lines:
    diff = max_len - len(line)
    information = f'--- {line} ' + (' ' * diff) + '---'
    print(information)
  print('-' * (max_len + 8))

def render_test(gltf_viewer, test_config, output_dir,
                opengl_lib=None, vk_icd=None):
  assert os.path.isdir(output_dir), f"output directory {output_dir} does not exist"
  assert os.access(gltf_viewer, os.X_OK)

  named_output_dir = os.path.join(output_dir, test_config.name)
  execute(f'mkdir -p {named_output_dir}')

  results = []
  for test in test_config.tests:
    test_json_path = f'{named_output_dir}/{test.name}.simplified.json'

    with open(test_json_path, 'w') as f:
      f.write(f'[{test.to_filament_format()}]')

    for backend in test_config.backends:
      env = None
      if backend == 'opengl' and opengl_lib and os.path.isdir(opengl_lib):
        env = {'LD_LIBRARY_PATH': opengl_lib}

      for model in test.models:
        model_path = test_config.models[model]
        out_name = f'{test.name}.{backend}.{model}'
        test_desc = out_name

        important_print(f'Rendering {test_desc}')

        res, _ = execute(f'{gltf_viewer} -a {backend} --batch={test_json_path} -e {model_path} --headless',
                env=env, capture_output=False)

        if res == 0:
          execute(f'mv -f {test.name}0.tif {named_output_dir}/{out_name}.tif', capture_output=False)
          execute(f'mv -f {test.name}0.json {named_output_dir}/{test.name}.json', capture_output=False)
        else:
          important_print(f'{test_desc} failed with error={res}')
        print('')

        results.append((out_name, res))
  return results

GOLDENS_DIR = 'renderdiff_goldens'

# We pull the goldens from the filament-assets repo
def pull_goldens(output_dir):
  assert os.path.isdir(output_dir), f"output directory {output_dir} does not exist"
  golden_dir = os.path.join(output_dir, "golden")
  assets_dir = os.path.join(output_dir, "filament-assets")

  if not os.path.exists(assets_dir):
    execute('git clone --depth 1 git@github.com:google/filament-assets.git', cwd=output_dir)
  else:
    execute('git fetch', cwd=assets_dir)
    execute('git checkout main ', cwd=assets_dir)
    execute('git rebase', cwd=assets_dir)

  if os.path.exists(golden_dir):
    execute('rm -f goldens/*', cwd=output_dir)
  execute(f'cp filament-assets/{GOLDENS_DIR}/* goldens', cwd=output_dir)

def push_goldens(output_dir, test_name, filter_func=lambda a:True):
  for test in test_config.tests:
    for backend in test_config.backends:
      for model in test.models:
        pass

if __name__ == "__main__":
  parser = ArgParseImpl()
  parser.add_argument('--test', help='Configuration of the test', required=True)
  parser.add_argument('--gltf_viewer', help='Path to the gltf_viewer', required=True)
  parser.add_argument('--output_dir', help='Output Directory', required=True)
  parser.add_argument('--opengl_lib', help='Path to the folder containing OpenGL driver lib (for LD_LIBRARY_PATH)')
  parser.add_argument('--vk_icd', help='Path to VK ICD file')

  args, _ = parser.parse_known_args(sys.argv[1:])
  test = parse_test_config_from_path(args.test)
  render_result = render_test(args.gltf_viewer, test, args.output_dir, opengl_lib=args.opengl_lib, vk_icd=args.vk_icd)

  failed = [f'   {tname}' for tname, res in render_result if res != 0]
  success_count = len(render_result) - len(failed  )
  important_print(f'Successfully rendered {success_count} / {len(render_result)}' +
                  ('\nFailed:\n' + ('\n'.join(failed)) if len(failed) > 0 else ''))
