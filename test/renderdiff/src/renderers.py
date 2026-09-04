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
import shlex
import concurrent.futures
import fnmatch
from dataclasses import dataclass, field
from utils import execute, mkdir_p, mv_f, important_print
from results import RESULT_OK, RESULT_FAILED

@dataclass
class RenderTestCase(abc.ABC):
    test_name: str
    backend: str
    output_dir: str

    @property
    @abc.abstractmethod
    def target_name(self) -> str:
        """Name of the target/model used in output filenames and working directories."""
        pass

    def get_renderer_spec(self, renderer: 'BaseRenderer') -> str:
        return f"{renderer.platform}-{self.backend}"

    def get_out_name(self, renderer: 'BaseRenderer') -> str:
        return f"{self.test_name}.{self.get_renderer_spec(renderer)}.{self.target_name}"

    def get_out_tif_name(self, renderer: 'BaseRenderer') -> str:
        return os.path.join(self.output_dir, f"{self.get_out_name(renderer)}.tif")

    def get_working_dir(self, renderer: 'BaseRenderer') -> str:
        return f"/tmp/renderdiff/{self.get_renderer_spec(renderer)}/{self.test_name}/{self.target_name}"

    def render(self, renderer: 'BaseRenderer') -> dict:
        out_name = self.get_out_name(renderer)
        working_dir = self.get_working_dir(renderer)
        mkdir_p(working_dir)

        important_print(f'Rendering {out_name}')
        env = renderer.get_env()
        out_tif_name = self.get_out_tif_name(renderer)

        result_code, result = self._execute_render(renderer, env, working_dir, out_tif_name)

        return {
            'name': out_name,
            'result': result,
            'result_code': result_code,
        }

    @abc.abstractmethod
    def _execute_render(self, renderer: 'BaseRenderer', env: dict, working_dir: str, out_tif_name: str) -> tuple[int, str]:
        """Subclass-specific execution command and artifact capture."""
        pass


@dataclass
class GltfRenderTestCase(RenderTestCase):
    model: str
    model_path: str
    test_json_path: str

    @property
    def target_name(self) -> str:
        return self.model

    def _execute_render(self, renderer: 'BaseRenderer', env: dict, working_dir: str, out_tif_name: str) -> tuple[int, str]:
        out_name = self.get_out_name(renderer)
        executable_abs = os.path.abspath(renderer.executable)
        cmd = f'{executable_abs} -a {self.backend} --batch={self.test_json_path} -e {self.model_path} --headless'
        out_code, output = execute(cmd, cwd=working_dir, env=env, capture_output=True)

        tif_src = f'{working_dir}/{self.test_name}0.tif'
        json_src = f'{working_dir}/{self.test_name}0.json'
        if out_code == 0 and os.path.exists(tif_src):
            result = RESULT_OK
            mv_f(tif_src, out_tif_name)
            if os.path.exists(json_src):
                mv_f(json_src, os.path.join(self.output_dir, f'{out_name}.json'))
            important_print(f'{out_name} rendering succeeded. output=\n{output}')
        else:
            result = RESULT_FAILED
            important_print(f'{out_name} rendering failed with error={out_code} output=\n{output}')

        return out_code, result


@dataclass
class SampleRenderTestCase(RenderTestCase):
    target: str
    executable: str
    warmup_frames: int = 10
    fixed_timestep: float = 0.0166667
    extra_args: list[str] = field(default_factory=list)

    @property
    def target_name(self) -> str:
        return self.target

    def _execute_render(self, renderer: 'BaseRenderer', env: dict, working_dir: str, out_tif_name: str) -> tuple[int, str]:
        out_name = self.get_out_name(renderer)
        sample_bin = os.path.join(os.path.dirname(os.path.abspath(renderer.executable)), self.executable)
        if not os.path.exists(sample_bin):
            important_print(f'{out_name} failed: executable not found at {sample_bin}')
            return 127, RESULT_FAILED

        if os.path.exists(out_tif_name):
            os.remove(out_tif_name)

        extra_args_str = " ".join(shlex.quote(arg) for arg in (self.extra_args or []))
        cmd = (
            f'{sample_bin} -a {self.backend} --headless '
            f'--screenshot={out_tif_name} --frames={self.warmup_frames} '
            f'--fixed-timestep={self.fixed_timestep}'
        )
        if extra_args_str:
            cmd += f' {extra_args_str}'

        out_code, output = execute(cmd, cwd=working_dir, env=env, capture_output=True)

        if out_code == 0 and os.path.exists(out_tif_name):
            result = RESULT_OK
            important_print(f'{out_name} rendering succeeded. output=\n{output}')
        else:
            result = RESULT_FAILED
            important_print(f'{out_name} rendering failed with error={out_code} output=\n{output}')

        return out_code, result


class BaseRenderer(abc.ABC):
    def __init__(self, platform: str, backend: str, executable: str):
        self.platform = platform
        self.backend = backend
        self.executable = executable

    @abc.abstractmethod
    def get_env(self) -> dict:
        """Returns the environment dictionary for subprocess execution."""
        pass

    @abc.abstractmethod
    def run_tests(self, test_config: 'RenderTestConfig', output_dir: str) -> list[dict]:
        """Executes a suite of tests and returns a list of results."""
        pass


class DesktopRenderer(BaseRenderer):
    def __init__(self, platform: str, backend: str, executable: str, num_threads: int = None, test_filter: str = None):
        super().__init__(platform, backend, executable)
        self.num_threads = num_threads
        self.test_filter = test_filter

    def get_env(self) -> dict:
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
        return test_case.render(self)

    def _create_test_cases(self, test, named_output_dir: str) -> list[RenderTestCase]:
        mkdir_p(named_output_dir)
        cases = []
        if test.is_gltf_test:
            test_json_path = os.path.abspath(f'{named_output_dir}/{test.name}.simplified.json')
            with open(test_json_path, 'w') as f:
                f.write(f'[{test.to_filament_format()}]')

            for model in test.gltf_test.models:
                model_path = os.path.abspath(test.gltf_test.models_map[model])
                cases.append(GltfRenderTestCase(
                    test_name=test.name,
                    backend=self.backend,
                    output_dir=named_output_dir,
                    model=model,
                    model_path=model_path,
                    test_json_path=test_json_path
                ))
        elif test.is_sample_test:
            cases.append(SampleRenderTestCase(
                test_name=test.name,
                backend=self.backend,
                output_dir=named_output_dir,
                target=test.sample_test.target,
                executable=test.sample_test.executable,
                warmup_frames=test.sample_test.warmup_frames,
                fixed_timestep=test.sample_test.fixed_timestep,
                extra_args=test.sample_test.args
            ))
        return cases

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

                test_cases = self._create_test_cases(test, named_output_dir)
                for test_case in test_cases:
                    test_out_name = test_case.get_out_name(self)
                    if self.test_filter and not fnmatch.fnmatch(test_out_name, self.test_filter):
                        print(f'Skipping {test_out_name} because it does not match filter')
                        continue

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
