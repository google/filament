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
import shutil

from utils import ArgParseImpl, important_print

import test_config
from results import RESULT_OK

from renderers import RendererFactory

if __name__ == "__main__":
  parser = ArgParseImpl()
  parser.add_argument('--test', help='Configuration of the test', required=True)
  parser.add_argument('--platform', help='Platform to render on (e.g. desktop)', default='desktop')
  parser.add_argument('--backend', help='Backend to render with (e.g. vulkan)', required=True)
  parser.add_argument('--executable', help='Path to the executable (e.g. gltf_viewer)', required=True)
  parser.add_argument('--output_dir', help='Output Directory', required=True)
  parser.add_argument('--test_filter', help='Filter for the tests to run')
  parser.add_argument('--num_threads', help='Number of threads to use for rendering', type=int)

  args, _ = parser.parse_known_args(sys.argv[1:])
  test = test_config.parse_from_path(args.test)

  renderer = RendererFactory.create(
      platform=args.platform,
      backend=args.backend,
      executable=args.executable,
      test_filter=args.test_filter,
      num_threads=args.num_threads
  )

  results = renderer.run_tests(test, args.output_dir)

  named_output_dir = os.path.join(args.output_dir, test.name)

  with open(os.path.join(named_output_dir, f'render_results_{args.backend}.json'), 'w') as f:
    f.write(json.dumps(results, indent=2))

  shutil.copy2(args.test, os.path.join(named_output_dir, 'test.json'))

  failed = [f"   {k['name']}" for k in results if k['result'] != RESULT_OK]
  success_count = len(results) - len(failed)
  important_print(f'Successfully rendered {success_count} / {len(results)} tests' +
                   ('\nFailed:\n' + ('\n'.join(failed)) if len(failed) > 0 else ''))

  if len(failed) > 0:
    exit(1)
