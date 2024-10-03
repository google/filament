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

from utils import execute, ArgParseImpl

from parse_test_json import parse_test_config_from_path
import sys
import os

def run_test(gltf_viewer, pixel_test, output_dir, opengl_lib=None, vk_icd=None):
  assert os.path.isdir(output_dir)
  assert os.access(gltf_viewer, os.X_OK)

  for test in pixel_test.tests:
    test_json_path = f'{output_dir}/{test.name}_simplified.json'

    with open(test_json_path, 'w') as f:
      f.write(f'[{test.to_filament_format()}]')

    for backend in pixel_test.backends:
      env = None
      if backend == 'opengl' and opengl_lib and os.path.isdir(opengl_lib):
        env = {'LD_LIBRARY_PATH': opengl_lib}

      for model in test.models:
        model_path = pixel_test.models[model]
        out_name = f'{test.name}_{model}_{backend}'
        execute(f'{gltf_viewer} -a {backend} --batch={test_json_path} -e {model_path} --headless',
                env=env, capture_output=False)
        execute(f'mv -f {test.name}0.ppm {output_dir}/{out_name}.ppm', capture_output=False)
        execute(f'mv -f {test.name}0.json {output_dir}/{test.name}.json', capture_output=False)

if __name__ == "__main__":
  parser = ArgParseImpl()
  parser.add_argument('--test', help='Configuration of the test', required=True)
  parser.add_argument('--gltf_viewer', help='Path to the gltf_viewer', required=True)
  parser.add_argument('--output_dir', help='Output Directory', required=True)
  parser.add_argument('--opengl_lib', help='Path to the folder containing OpenGL driver lib (for LD_LIBRARY_PATH)')
  parser.add_argument('--vk_icd', help='Path to VK ICD file')

  args, _ = parser.parse_known_args(sys.argv[1:])
  test = parse_test_config_from_path(args.test)
  run_test(args.gltf_viewer, test, args.output_dir, opengl_lib=args.opengl_lib, vk_icd=args.vk_icd)
