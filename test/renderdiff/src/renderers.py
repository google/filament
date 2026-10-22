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
import signal
import sys
import concurrent.futures
import fnmatch
from dataclasses import dataclass, field
from utils import execute, mkdir_p, important_print
from results import RESULT_OK, RESULT_FAILED


def describe_exit_code(code: int) -> str:
    """Describes a process exit code, naming the signal when the process was killed by one.

    Only a crash leaves a core behind for report_crashes.sh to turn into a backtrace.
    """
    if code >= 0:
        return str(code)
    try:
        name = signal.Signals(-code).name
    except ValueError:
        name = 'unknown signal'
    return f'{code} (killed by {name}; look for a backtrace in out/renderdiff/crashes)'

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
        return os.path.abspath(os.path.join(self.output_dir, f"{self.get_out_name(renderer)}.tif"))

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
    # The test as a batch spec. Only the web renderer still drives gltf_viewer through --batch.
    test_json_path: str
    # The test as the contents of a --settings file, which the desktop renderer uses. It is written
    # to the working directory rather than the output directory, because update_golden.py treats
    # every JSON file in the output directory as a golden.
    settings_json: str = '{}'
    # These match the sample tests. Under --screenshot gltf_viewer loads the asset synchronously,
    # so the warmup frames all render the complete scene and the image does not depend on how
    # fast the machine decodes textures.
    warmup_frames: int = 10
    fixed_timestep: float = 0.0166667

    @property
    def target_name(self) -> str:
        return self.model

    def _execute_render(self, renderer: 'BaseRenderer', env: dict, working_dir: str, out_tif_name: str) -> tuple[int, str]:
        out_name = self.get_out_name(renderer)
        executable_abs = os.path.abspath(renderer.executable)

        settings_json_path = os.path.join(working_dir, f'{self.test_name}.settings.json')
        with open(settings_json_path, 'w') as f:
            f.write(self.settings_json)

        # gltf_viewer writes the effective settings next to the screenshot, with a .json extension.
        out_json_name = os.path.splitext(out_tif_name)[0] + '.json'
        for stale in (out_tif_name, out_json_name):
            if os.path.exists(stale):
                os.remove(stale)

        cmd = (
            f'{shlex.quote(executable_abs)} -a {self.backend} --headless '
            f'--settings={shlex.quote(settings_json_path)} '
            f'--screenshot={shlex.quote(out_tif_name)} --frames={self.warmup_frames} '
            f'--fixed-timestep={self.fixed_timestep} {shlex.quote(self.model_path)}'
        )
        out_code, output = execute(cmd, cwd=working_dir, env=env, capture_output=True)

        if out_code == 0 and os.path.exists(out_tif_name):
            result = RESULT_OK
            important_print(f'{out_name} rendering succeeded. model={self.model_path} '
                            f'output=\n{output}')
        else:
            result = RESULT_FAILED
            # gltf_viewer writes the settings during setup, before rendering anything, so a failed
            # render can leave them behind without an image. update_golden.py would treat that
            # file as a golden, so it is removed to match the old behavior of copying neither.
            if os.path.exists(out_json_name):
                os.remove(out_json_name)
            important_print(f'{out_name} rendering failed with '
                            f'error={describe_exit_code(out_code)} model={self.model_path} '
                            f'output=\n{output}')

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
            important_print(f'{out_name} rendering failed with '
                            f'error={describe_exit_code(out_code)} output=\n{output}')

        return out_code, result


# A render is a separate process that helps itself to the whole machine: the software drivers size
# their worker pools from the CPU count, and the samples run their own job system on top. What
# needs bounding is therefore how many renders are in flight, not the threads within one.
#
# ThreadPoolExecutor's default width, min(32, cpu_count + 4), meant twenty concurrent renders on
# the 16-core runner. Running a few at a time costs little wall time and keeps the machine out of
# the scheduling regime where a latent race is most likely to be lost. This is exposure reduction
# rather than a fix: load cannot break an invariant inside a driver process, only change how often
# it is observed.
_CPUS_PER_RENDER = 4
_MIN_CONCURRENT_RENDERS = 2
_MAX_CONCURRENT_RENDERS = 4


def default_concurrent_renders() -> int:
    """Number of renders to run at once when the caller does not ask for a specific count."""
    cpu_count = os.cpu_count() or 1
    return max(_MIN_CONCURRENT_RENDERS,
               min(_MAX_CONCURRENT_RENDERS, cpu_count // _CPUS_PER_RENDER))


class BaseRenderer(abc.ABC):
    def __init__(self, platform: str, backend: str, executable: str):
        self.platform = platform
        self.backend = backend
        self.executable = executable
        # Subclasses that support filtering overwrite this; _collect_test_cases reads it.
        self.test_filter = None

    @abc.abstractmethod
    def get_env(self) -> dict:
        """Returns the environment dictionary for subprocess execution."""
        pass

    @abc.abstractmethod
    def run_tests(self, test_config: 'RenderTestConfig', output_dir: str) -> list[dict]:
        """Executes a suite of tests and returns a list of results."""
        pass

    def _make_gltf_test_case(self, test, named_output_dir: str, model: str, model_path: str,
                             test_json_path: str) -> RenderTestCase:
        """Hook: how this renderer executes a gltf test."""
        return GltfRenderTestCase(
            test_name=test.name,
            backend=self.backend,
            output_dir=named_output_dir,
            model=model,
            model_path=model_path,
            test_json_path=test_json_path,
            settings_json=test.to_settings_format()
        )

    def _make_sample_test_case(self, test, named_output_dir: str) -> RenderTestCase:
        """Hook: how this renderer executes a sample test."""
        return SampleRenderTestCase(
            test_name=test.name,
            backend=self.backend,
            output_dir=named_output_dir,
            target=test.sample_test.target,
            executable=test.sample_test.executable,
            warmup_frames=test.sample_test.warmup_frames,
            fixed_timestep=test.sample_test.fixed_timestep,
            extra_args=test.sample_test.args
        )

    def _create_test_cases(self, test, named_output_dir: str) -> list[RenderTestCase]:
        mkdir_p(named_output_dir)
        cases = []
        if test.is_gltf_test:
            test_json_path = os.path.abspath(f'{named_output_dir}/{test.name}.simplified.json')
            with open(test_json_path, 'w') as f:
                f.write(f'[{test.to_filament_format()}]')

            for model in test.gltf_test.models:
                model_path = os.path.abspath(test.gltf_test.models_map[model])
                cases.append(self._make_gltf_test_case(
                    test, named_output_dir, model, model_path, test_json_path))
        elif test.is_sample_test:
            cases.append(self._make_sample_test_case(test, named_output_dir))
        return cases

    def _collect_test_cases(self, test_config, output_dir: str) -> list[RenderTestCase]:
        """Expands a suite into the test cases this renderer should actually run."""
        named_output_dir = os.path.abspath(os.path.join(output_dir, test_config.name))
        mkdir_p(named_output_dir)

        renderer_spec = f"{self.platform}-{self.backend}"
        collected = []
        for test in test_config.tests:
            if renderer_spec not in test.renderers:
                continue

            for test_case in self._create_test_cases(test, named_output_dir):
                test_out_name = test_case.get_out_name(self)
                if self.test_filter and not fnmatch.fnmatch(test_out_name, self.test_filter):
                    print(f'Skipping {test_out_name} because it does not match filter')
                    continue
                collected.append(test_case)
        return collected



class DesktopRenderer(BaseRenderer):
    def __init__(self, platform: str, backend: str, executable: str, num_threads: int = None, test_filter: str = None):
        super().__init__(platform, backend, executable)
        # num_threads is named for the pool it configures, but what it bounds is the number of
        # renders running at once.
        self.concurrent_renders = num_threads if num_threads else default_concurrent_renders()
        self.test_filter = test_filter

    def get_env(self) -> dict:
        env = os.environ.copy()
        # Under Linux, Dawn has no Metal or D3D backend to fall back on, so a webgpu render goes
        # through Vulkan and needs the same software ICD the vulkan renderer uses; without it Dawn
        # enumerates no adapters and every webgpu test aborts. macOS is deliberately left alone,
        # because Dawn selects Metal there and that is the path macOS users actually run.
        needs_vk_icd = self.backend == 'vulkan' or (
                self.backend == 'webgpu' and sys.platform.startswith('linux'))
        if needs_vk_icd:
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

    def run_tests(self, test_config, output_dir: str) -> list[dict]:
        test_cases = self._collect_test_cases(test_config, output_dir)

        results = []
        print(f'Rendering {self.platform}-{self.backend} with up to '
              f'{self.concurrent_renders} concurrent renders')
        with concurrent.futures.ThreadPoolExecutor(max_workers=self.concurrent_renders) as executor:
            futures = [executor.submit(self.render_single_test, c) for c in test_cases]
            for future in concurrent.futures.as_completed(futures):
                results.append(future.result())

        return results

class RendererFactory:
    @staticmethod
    def create(platform: str, backend: str, executable: str, **kwargs) -> BaseRenderer:
        if platform == 'desktop':
            return DesktopRenderer(platform, backend, executable, **kwargs)
        elif platform == 'web':
            # Imported here, not at module scope: the web renderer needs playwright, and a
            # desktop-only run should not have to install it.
            from web_renderer import WebRenderer
            return WebRenderer(platform, backend, executable, **kwargs)
        else:
            # AndroidRenderer and others would be instantiated here.
            raise NotImplementedError(f"Platform '{platform}' is not fully implemented yet.")

