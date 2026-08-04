# Copyright (C) 2026 The Android Open Source Project
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

import abc
import os
import concurrent.futures
import fnmatch
from dataclasses import dataclass
from utils import execute, mkdir_p, mv_f, important_print
from results import RESULT_OK, RESULT_FAILED

@dataclass
class RenderTestCase:
    test_name: str
    backend: str
    model: str
    model_path: str
    test_json_path: str
    output_dir: str

class BaseRenderer(abc.ABC):
    def __init__(self, platform: str, backend: str, executable: str):
        self.platform = platform
        self.backend = backend
        self.executable = executable

    @abc.abstractmethod
    def run_tests(self, test_config: 'RenderTestConfig', output_dir: str) -> list[dict]:
        """Executes a suite of tests and returns a list of results."""
        pass

class DesktopRenderer(BaseRenderer):
    def __init__(self, platform: str, backend: str, executable: str, num_threads: int = None, test_filter: str = None):
        super().__init__(platform, backend, executable)
        self.num_threads = num_threads
        self.test_filter = test_filter

    def _get_env(self) -> dict:
        env = os.environ.copy()
        if self.backend == 'vulkan':
            vk_icd = os.environ.get('FILAMENT_VK_ICD')
            if vk_icd and os.path.exists(vk_icd):
                env.update({
                    'VK_ICD_FILENAMES': vk_icd,
                    'VK_DRIVER_FILES': vk_icd,
                })
        elif self.backend == 'opengl':
            opengl_lib = os.environ.get('FILAMENT_OPENGL_LIB')
            if opengl_lib and os.path.isdir(opengl_lib):
                env.update({
                    'LD_LIBRARY_PATH': opengl_lib,
                    'DYLD_LIBRARY_PATH': opengl_lib,
                })
        return env

    def render_single_test(self, test_case: RenderTestCase) -> dict:
        env = self._get_env()
        ext = '.tif'
        renderer_spec = f"{self.platform}-{self.backend}"
        out_name = f'{test_case.test_name}.{renderer_spec}.{test_case.model}'
        test_desc = out_name

        working_dir = f'/tmp/renderdiff/{renderer_spec}/{test_case.model}'
        mkdir_p(working_dir)

        important_print(f'Rendering {test_desc}')

        executable_abs = os.path.abspath(self.executable)
        out_code, output = execute(
            f'{executable_abs} -a {self.backend} --batch={test_case.test_json_path} -e {test_case.model_path} --headless',
            cwd=working_dir,
            env=env, capture_output=True
        )

        result = ''
        if out_code == 0:
            result = RESULT_OK
            out_tif_basename = f'{out_name}{ext}'
            out_tif_name = f'{test_case.output_dir}/{out_tif_basename}'
            mv_f(f'{working_dir}/{test_case.test_name}0.tif', out_tif_name)
            mv_f(f'{working_dir}/{test_case.test_name}0.json', f'{test_case.output_dir}/{test_case.test_name}.json')
            important_print(f'{test_desc} rendering succeeded. output=\n{output}')
        else:
            result = RESULT_FAILED
            important_print(f'{test_desc} rendering failed with error={out_code}output=\n{output}')

        return {
            'name': out_name,
            'result': result,
            'result_code': out_code,
        }

    def run_tests(self, test_config, output_dir: str) -> list[dict]:
        named_output_dir = os.path.join(output_dir, test_config.name)
        mkdir_p(named_output_dir)

        results = []
        with concurrent.futures.ThreadPoolExecutor(max_workers=self.num_threads) as executor:
            futures = []
            for test in test_config.tests:
                renderer_spec = f"{self.platform}-{self.backend}"
                if renderer_spec not in test.renderers:
                    continue

                test_json_path = os.path.abspath(f'{named_output_dir}/{test.name}.simplified.json')

                with open(test_json_path, 'w') as f:
                    f.write(f'[{test.to_filament_format()}]')

                for model in test.models:
                    renderer_spec = f"{self.platform}-{self.backend}"
                    test_name = f'{test.name}.{renderer_spec}.{model}'
                    if self.test_filter and not fnmatch.fnmatch(test_name, self.test_filter):
                        print(f'Skipping {test_name} because it does not match filter')
                        continue
                    model_path = os.path.abspath(test_config.models[model])

                    test_case = RenderTestCase(
                        test_name=test.name,
                        backend=self.backend,
                        model=model,
                        model_path=model_path,
                        test_json_path=test_json_path,
                        output_dir=named_output_dir
                    )

                    futures.append(executor.submit(self.render_single_test, test_case))

            for future in concurrent.futures.as_completed(futures):
                results.append(future.result())

        return results

class RendererFactory:
    @staticmethod
    def create(platform: str, backend: str, executable: str, **kwargs) -> BaseRenderer:
        if platform == 'desktop':
            return DesktopRenderer(platform, backend, executable, **kwargs)
        else:
            # AndroidRenderer and others would be instantiated here.
            raise NotImplementedError(f"Platform '{platform}' is not fully implemented yet.")
