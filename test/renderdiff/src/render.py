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
import json
import glob
import shutil

from utils import execute, ArgParseImpl, mkdir_p, mv_f, important_print

import test_config
from golden_manager import GoldenManager
from image_diff import same_image
from results import RESULT_OK, RESULT_FAILED

def _render_test_config(gltf_viewer,
                        test_config,
                        output_dir,
                        opengl_lib=None,
                        vk_icd=None):
  assert os.path.isdir(output_dir), f"output directory {output_dir} does not exist"
  assert os.access(gltf_viewer, os.X_OK)

  named_output_dir = os.path.join(output_dir, test_config.name)
  mkdir_p(named_output_dir)

  results = []
  for test in test_config.tests:
    test_json_path = f'{named_output_dir}/{test.name}.simplified.json'

    with open(test_json_path, 'w') as f:
      f.write(f'[{test.to_filament_format()}]')

    for backend in test_config.backends:
      env = None
      if backend == 'opengl' and opengl_lib and os.path.isdir(opengl_lib):
        env = {
          'LD_LIBRARY_PATH': opengl_lib,

           # for macOS
          'DYLD_LIBRARY_PATH': opengl_lib,
        }

      for model in test.models:
        model_path = test_config.models[model]
        out_name = f'{test.name}.{backend}.{model}'
        test_desc = out_name

        important_print(f'Rendering {test_desc}')

        out_code, _ = execute(
          f'{gltf_viewer} -a {backend} --batch={test_json_path} -e {model_path} --headless',
          env=env, capture_output=False
        )

        result = ''
        if out_code == 0:
          result = RESULT_OK
          out_tif_basename = f'{out_name}.tif'
          out_tif_name = f'{named_output_dir}/{out_tif_basename}'
          mv_f(f'{test.name}0.tif', out_tif_name)
          mv_f(f'{test.name}0.json', f'{named_output_dir}/{test.name}.json')
        else:
          result = RESULT_FAILED
          important_print(f'{test_desc} rendering failed with error={out_code}')

        results.append({
          'name': out_name,
          'result': result,
          'result_code': out_code,
        })
  return named_output_dir, results

if __name__ == "__main__":
  parser = ArgParseImpl()
  parser.add_argument('--test', help='Configuration of the test', required=True)
  parser.add_argument('--gltf_viewer', help='Path to the gltf_viewer', required=True)
  parser.add_argument('--output_dir', help='Output Directory', required=True)
  parser.add_argument('--opengl_lib', help='Path to the folder containing OpenGL driver lib (for LD_LIBRARY_PATH)')
  parser.add_argument('--vk_icd', help='Path to VK ICD file')

  args, _ = parser.parse_known_args(sys.argv[1:])
  test = test_config.parse_from_path(args.test)

  output_dir, results = \
    _render_test_config(args.gltf_viewer,
                        test,
                        args.output_dir,
                        opengl_lib=args.opengl_lib,
                        vk_icd=args.vk_icd)

  with open(f'{output_dir}/render_results.json', 'w') as f:
    f.write(json.dumps(results, indent=2))

  shutil.copy2(args.test, f'{output_dir}/test.json')

  failed = [f"   {k['name']}" for k in results if k['result'] != RESULT_OK]
  success_count = len(results) - len(failed)
  important_print(f'Successfully rendered {success_count} / {len(results)} tests' +
                   ('\nFailed:\n' + ('\n'.join(failed)) if len(failed) > 0 else ''))

  if len(failed) > 0:
    exit(1)
