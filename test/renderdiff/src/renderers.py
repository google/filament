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
    @abc.abstractmethod
    def render(self, test_case: RenderTestCase) -> dict:
        pass

class NativeRenderer(BaseRenderer):
    def __init__(self, gltf_viewer: str):
        self.gltf_viewer = gltf_viewer

    def _get_env(self, test_case: RenderTestCase) -> dict:
        # We need to pass along the old environment because it might include vars from vulkansdk.
        return os.environ.copy()

    def render(self, test_case: RenderTestCase) -> dict:
        env = self._get_env(test_case)
        out_name = f'{test_case.test_name}.{test_case.backend}.{test_case.model}'
        test_desc = out_name

        working_dir = f'/tmp/renderdiff/{test_case.backend}/{test_case.model}'
        mkdir_p(working_dir)

        important_print(f'Rendering {test_desc}')

        out_code, output = execute(
            f'{self.gltf_viewer} -a {test_case.backend} --batch={test_case.test_json_path} -e {test_case.model_path} --headless',
            cwd=working_dir,
            env=env, capture_output=True
        )

        result = ''
        if out_code == 0:
            result = RESULT_OK
            out_tif_basename = f'{out_name}.tif'
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

class OpenGLRenderer(NativeRenderer):
    def __init__(self, gltf_viewer: str, opengl_lib: str = None):
        super().__init__(gltf_viewer)
        self.opengl_lib = opengl_lib

    def _get_env(self, test_case: RenderTestCase) -> dict:
        env = super()._get_env(test_case)
        if self.opengl_lib and os.path.isdir(self.opengl_lib):
            env.update({
                'LD_LIBRARY_PATH': self.opengl_lib,
                # for macOS
                'DYLD_LIBRARY_PATH': self.opengl_lib,
            })
        return env

class VulkanRenderer(NativeRenderer):
    def __init__(self, gltf_viewer: str, vk_icd: str = None):
        super().__init__(gltf_viewer)
        self.vk_icd = vk_icd

    def _get_env(self, test_case: RenderTestCase) -> dict:
        env = super()._get_env(test_case)
        if self.vk_icd and os.path.exists(self.vk_icd):
            env.update({
                'VK_ICD_FILENAMES': self.vk_icd,
                'VK_DRIVER_FILES': self.vk_icd,
            })
        return env

class WebGPUDesktopRenderer(NativeRenderer):
    pass
