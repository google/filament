#!/usr/bin/python3
#
# Copyright 2022 The Draco Authors
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
"""Tests installations of the Draco library.

Builds the library in shared and static configurations on the current host
system, and then confirms that a simple test application can link in both
configurations.
"""

import argparse
import multiprocessing
import os
import pathlib
import shlex
import shutil
import subprocess
import sys

# CMake executable.
CMAKE = shutil.which('cmake')

# List of generators available in the current CMake executable.
CMAKE_AVAILABLE_GENERATORS = []

# List of variable defs to be passed through to CMake via its -D argument.
CMAKE_DEFINES = []

# CMake builds use the specified generator.
CMAKE_GENERATOR = None

# Enable the transcoder before running tests (sets DRACO_TRANSCODER_SUPPORTED
# and builds transcoder support dependencies).
ENABLE_TRANSCODER = False

# The Draco tree that this script uses.
DRACO_SOURCES_PATH = os.path.abspath(os.path.join('..', '..', '..', '..'))

# Path to this script and the rest of the test project files.
TEST_SOURCES_PATH = os.path.dirname(os.path.abspath(__file__))

# The Draco build directories.
DRACO_SHARED_BUILD_PATH = os.path.join(TEST_SOURCES_PATH, '_draco_build_shared')
DRACO_STATIC_BUILD_PATH = os.path.join(TEST_SOURCES_PATH, '_draco_build_static')

# The Draco install roots.
DRACO_SHARED_INSTALL_PATH = os.path.join(TEST_SOURCES_PATH,
                                         '_draco_install_shared')
DRACO_STATIC_INSTALL_PATH = os.path.join(TEST_SOURCES_PATH,
                                         '_draco_install_static')

DRACO_SHARED_INSTALL_BIN_PATH = os.path.join(DRACO_SHARED_INSTALL_PATH, 'bin')

DRACO_SHARED_INSTALL_LIB_PATH = os.path.join(DRACO_SHARED_INSTALL_PATH, 'lib')

# Argument for -j when using make, or -m when using Visual Studio. Number of
# build jobs.
NUM_PROCESSES = multiprocessing.cpu_count() - 1

# The test project build directories.
TEST_SHARED_BUILD_PATH = os.path.join(TEST_SOURCES_PATH, '_test_build_shared')
TEST_STATIC_BUILD_PATH = os.path.join(TEST_SOURCES_PATH, '_test_build_static')

# The test project install directories.
TEST_SHARED_INSTALL_PATH = os.path.join(TEST_SOURCES_PATH,
                                        '_test_install_shared')
TEST_STATIC_INSTALL_PATH = os.path.join(TEST_SOURCES_PATH,
                                        '_test_install_static')

# Show configuration and build output.
VERBOSE = False


def cmake_get_available_generators():
  """Returns list of generators available in current CMake executable."""
  result = run_process_and_capture_output(f'{CMAKE} --help')

  if result[0] != 0:
    raise Exception(f'cmake --help failed, exit code: {result[0]}\n{result[1]}')

  help_text = result[1].splitlines()
  generators_start_index = help_text.index('Generators') + 3
  generators_text = help_text[generators_start_index::]

  generators = []
  for gen in generators_text:
    gen = gen.split('=')[0].strip().replace('* ', '')

    if gen and gen[0] != '=':
      generators.append(gen)

  return generators


def cmake_get_generator():
  """Returns the CMake generator from CMakeCache.txt in the current dir."""
  cmake_cache_file_path = os.path.join(os.getcwd(), 'CMakeCache.txt')
  cmake_cache_text = ''
  with open(cmake_cache_file_path, 'r') as cmake_cache_file:
    cmake_cache_text = cmake_cache_file.read()

  if not cmake_cache_text:
    raise FileNotFoundError(f'{cmake_cache_file_path} missing or empty.')

  generator = ''
  for line in cmake_cache_text.splitlines():
    if line.startswith('CMAKE_GENERATOR:INTERNAL='):
      generator = line.split('=')[1]

  return generator


def run_process_and_capture_output(cmd, env=None):
  """Runs |cmd| as a child process.

  Returns process exit code and output. Streams process output to stdout when
  VERBOSE is true.

  Args:
    cmd: String containing the command to execute.
    env: Optional dict of environment variables.

  Returns:
    Tuple of exit code and output.
  """
  if not cmd:
    raise ValueError('run_process_and_capture_output requires cmd argument.')

  if os.name == 'posix':
    # On posix systems subprocess.Popen will treat |cmd| as the program name
    # when it is passed as a string. Unconditionally split the command so
    # callers don't need to care about this detail.
    cmd = shlex.split(cmd)

  proc = subprocess.Popen(
      cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=env)

  if VERBOSE:
    print('COMMAND output:')

  stdout = ''
  for line in iter(proc.stdout.readline, b''):
    decoded_line = line.decode('utf-8')
    if VERBOSE:
      sys.stdout.write(decoded_line)
      sys.stdout.flush()
    stdout += decoded_line

  # Wait for the process to exit so that the exit code is available.
  proc.wait()
  return [proc.returncode, stdout]


def create_output_directories():
  """Creates the build output directores for the test."""
  pathlib.Path(DRACO_SHARED_BUILD_PATH).mkdir(parents=True, exist_ok=True)
  pathlib.Path(DRACO_STATIC_BUILD_PATH).mkdir(parents=True, exist_ok=True)
  pathlib.Path(TEST_SHARED_BUILD_PATH).mkdir(parents=True, exist_ok=True)
  pathlib.Path(TEST_STATIC_BUILD_PATH).mkdir(parents=True, exist_ok=True)


def cleanup():
  """Removes the build output directories from the test."""
  shutil.rmtree(DRACO_SHARED_BUILD_PATH)
  shutil.rmtree(DRACO_STATIC_BUILD_PATH)
  shutil.rmtree(DRACO_SHARED_INSTALL_PATH)
  shutil.rmtree(DRACO_STATIC_INSTALL_PATH)
  shutil.rmtree(TEST_SHARED_BUILD_PATH)
  shutil.rmtree(TEST_STATIC_BUILD_PATH)
  shutil.rmtree(TEST_SHARED_INSTALL_PATH)
  shutil.rmtree(TEST_STATIC_INSTALL_PATH)


def cmake_configure(source_path, cmake_args=None):
  """Configures a CMake build."""
  command = f'{CMAKE} {source_path}'

  if CMAKE_GENERATOR:
    if ' ' in CMAKE_GENERATOR:
      command += f' -G "{CMAKE_GENERATOR}"'
    else:
      command += f' -G {CMAKE_GENERATOR}'

  if cmake_args:
    for arg in cmake_args:
      command += f' {arg}'

  if CMAKE_DEFINES:
    for arg in CMAKE_DEFINES:
      command += f' -D{arg}'

  if VERBOSE:
    print(f'CONFIGURE command:\n{command}')

  result = run_process_and_capture_output(command)

  if result[0] != 0:
    raise Exception(f'CONFIGURE failed!\nexit_code: {result[0]}\n{result[1]}')


def cmake_build(cmake_args=None, build_args=None):
  """Runs a CMake build."""
  command = f'{CMAKE} --build .'

  if cmake_args:
    for arg in cmake_args:
      command += f' {arg}'

  if not build_args:
    build_args = []

  generator = cmake_get_generator()
  if generator.endswith('Makefiles'):
    build_args.append(f'-j {NUM_PROCESSES}')
  elif generator.startswith('Visual'):
    build_args.append(f'-m:{NUM_PROCESSES}')

  if build_args:
    command += ' --'
    for arg in build_args:
      command += f' {arg}'

  if VERBOSE:
    print(f'BUILD command:\n{command}')

  result = run_process_and_capture_output(f'{command}')

  if result[0] != 0:
    raise Exception(f'BUILD failed!\nexit_code: {result[0]}\n{result[1]}')


def run_install_check(install_path):
  """Runs the install_check program."""
  cmd = os.path.join(install_path, 'bin', 'install_check')
  if VERBOSE:
    print(f'RUN command: {cmd}')

  result = run_process_and_capture_output(
      cmd,
      # On Windows, add location of draco.dll into PATH env var
      {'PATH': DRACO_SHARED_INSTALL_BIN_PATH + os.pathsep + os.environ['PATH']},
    )
  if result[0] != 0:
    raise Exception(
        f'install_check run failed!\nexit_code: {result[0]}\n{result[1]}')


def build_and_install_transcoder_dependencies():
  """Builds and installs Draco dependencies for transcoder enabled builds."""
  orig_dir = os.getcwd()

  # The Eigen CMake build in the release Draco has pinned is, to put it mildly,
  # user unfriendly. Instead of wasting time trying to integrate it here, just
  # shutil.copytree() everything in $eigen_submodule_path to
  # $CMAKE_INSTALL_PREFIX/include/Eigen.
  # Eigen claims to be header-only, so this should be adequate for Draco's
  # needs here.
  eigen_submodule_path = os.path.join(
      DRACO_SOURCES_PATH, 'third_party', 'eigen', 'Eigen')

  # "Install" Eigen for the shared install root.
  eigen_install_path = os.path.join(
      DRACO_SHARED_INSTALL_PATH, 'include', 'Eigen')
  shutil.copytree(src=eigen_submodule_path, dst=eigen_install_path)

  # "Install" Eigen for the static install root.
  eigen_install_path = os.path.join(
      DRACO_STATIC_INSTALL_PATH, 'include', 'Eigen')
  shutil.copytree(src=eigen_submodule_path, dst=eigen_install_path)

  # Build and install gulrak/filesystem for shared and static configurations.
  # Note that this is basically running gulrak/filesystem's CMake build as an
  # install script.
  fs_submodule_path = os.path.join(
      DRACO_SOURCES_PATH, 'third_party', 'filesystem')

  # Install gulrak/filesystem in the shared draco install root.
  fs_shared_build = os.path.join(DRACO_SHARED_BUILD_PATH, '_fs')
  pathlib.Path(fs_shared_build).mkdir(parents=True, exist_ok=True)
  os.chdir(fs_shared_build)
  cmake_args = []
  cmake_args.append(f'-DCMAKE_INSTALL_PREFIX={DRACO_SHARED_INSTALL_PATH}')
  cmake_args.append('-DBUILD_SHARED_LIBS=ON')
  cmake_args.append('-DGHC_FILESYSTEM_BUILD_TESTING=OFF')
  cmake_args.append('-DGHC_FILESYSTEM_BUILD_EXAMPLES=OFF')
  cmake_configure(source_path=fs_submodule_path, cmake_args=cmake_args)
  cmake_build(cmake_args=['--target install'])

  # Install gulrak/filesystem in the shared draco install root.
  fs_static_build = os.path.join(DRACO_STATIC_BUILD_PATH, '_fs')
  pathlib.Path(fs_static_build).mkdir(parents=True, exist_ok=True)
  os.chdir(fs_static_build)
  cmake_args = []
  cmake_args.append(f'-DCMAKE_INSTALL_PREFIX={DRACO_SHARED_INSTALL_PATH}')
  cmake_args.append('-DBUILD_SHARED_LIBS=OFF')
  cmake_args.append('-DGHC_FILESYSTEM_BUILD_TESTING=OFF')
  cmake_args.append('-DGHC_FILESYSTEM_BUILD_EXAMPLES=OFF')
  cmake_configure(source_path=fs_submodule_path, cmake_args=cmake_args)
  cmake_build(cmake_args=['--target install'])

  # Build and install TinyGLTF for shared and static configurations.
  # Note, as above, that this is basically running TinyGLTF's CMake build as an
  # install script.
  tinygltf_submodule_path = os.path.join(
      DRACO_SOURCES_PATH, 'third_party', 'tinygltf')

  # Install TinyGLTF in the shared draco install root.
  tinygltf_shared_build = os.path.join(DRACO_SHARED_BUILD_PATH, '_TinyGLTF')
  pathlib.Path(tinygltf_shared_build).mkdir(parents=True, exist_ok=True)
  os.chdir(tinygltf_shared_build)
  cmake_args = []
  cmake_args.append(f'-DCMAKE_INSTALL_PREFIX={DRACO_SHARED_INSTALL_PATH}')
  cmake_args.append('-DTINYGLTF_BUILD_EXAMPLES=OFF')
  cmake_configure(source_path=tinygltf_submodule_path, cmake_args=cmake_args)
  cmake_build(cmake_args=['--target install'])

  # Install TinyGLTF in the static draco install root.
  tinygltf_static_build = os.path.join(DRACO_STATIC_BUILD_PATH, '_TinyGLTF')
  pathlib.Path(tinygltf_static_build).mkdir(parents=True, exist_ok=True)
  os.chdir(tinygltf_static_build)
  cmake_args = []
  cmake_args.append(f'-DCMAKE_INSTALL_PREFIX={DRACO_STATIC_INSTALL_PATH}')
  cmake_args.append('-DTINYGLTF_BUILD_EXAMPLES=OFF')
  cmake_configure(source_path=tinygltf_submodule_path, cmake_args=cmake_args)
  cmake_build(cmake_args=['--target install'])

  os.chdir(orig_dir)


def build_and_install_draco():
  """Builds Draco in shared and static configurations."""
  orig_dir = os.getcwd()

  if ENABLE_TRANSCODER:
    build_and_install_transcoder_dependencies()

  # Build and install Draco in shared library config for the current host
  # machine.
  os.chdir(DRACO_SHARED_BUILD_PATH)
  cmake_args = []
  cmake_args.append(f'-DCMAKE_INSTALL_PREFIX={DRACO_SHARED_INSTALL_PATH}')
  cmake_args.append('-DBUILD_SHARED_LIBS=ON')
  if ENABLE_TRANSCODER:
    cmake_args.append('-DDRACO_TRANSCODER_SUPPORTED=ON')
  cmake_configure(source_path=DRACO_SOURCES_PATH, cmake_args=cmake_args)
  cmake_build(cmake_args=['--target install'])

  # Build and install Draco in the static config for the current host machine.
  os.chdir(DRACO_STATIC_BUILD_PATH)
  cmake_args = []
  cmake_args.append(f'-DCMAKE_INSTALL_PREFIX={DRACO_STATIC_INSTALL_PATH}')
  cmake_args.append('-DBUILD_SHARED_LIBS=OFF')
  if ENABLE_TRANSCODER:
    cmake_args.append('-DDRACO_TRANSCODER_SUPPORTED=ON')
  cmake_configure(source_path=DRACO_SOURCES_PATH, cmake_args=cmake_args)
  cmake_build(cmake_args=['--target install'])

  os.chdir(orig_dir)


def build_test_project():
  """Builds the test application in shared and static configurations."""
  orig_dir = os.getcwd()

  # Configure the test project against draco shared and build it.
  os.chdir(TEST_SHARED_BUILD_PATH)
  cmake_args = []
  cmake_args.append(f'-DCMAKE_INSTALL_PREFIX={TEST_SHARED_INSTALL_PATH}')
  cmake_args.append(f'-DCMAKE_PREFIX_PATH={DRACO_SHARED_INSTALL_PATH}')
  cmake_args.append(f'-DCMAKE_INSTALL_RPATH={DRACO_SHARED_INSTALL_LIB_PATH}')
  cmake_configure(source_path=f'{TEST_SOURCES_PATH}', cmake_args=cmake_args)
  cmake_build(cmake_args=['--target install'])
  run_install_check(TEST_SHARED_INSTALL_PATH)

  # Configure the test project against draco static and build it.
  os.chdir(TEST_STATIC_BUILD_PATH)
  cmake_args = []
  cmake_args.append(f'-DCMAKE_INSTALL_PREFIX={TEST_STATIC_INSTALL_PATH}')
  cmake_args.append(f'-DCMAKE_PREFIX_PATH={DRACO_STATIC_INSTALL_PATH}')
  cmake_configure(source_path=f'{TEST_SOURCES_PATH}', cmake_args=cmake_args)
  cmake_build(cmake_args=['--target install'])
  run_install_check(TEST_STATIC_INSTALL_PATH)

  os.chdir(orig_dir)


def test_draco_install():
  create_output_directories()
  build_and_install_draco()
  build_test_project()
  cleanup()


if __name__ == '__main__':
  CMAKE_AVAILABLE_GENERATORS = cmake_get_available_generators()

  parser = argparse.ArgumentParser()
  parser.add_argument(
      '-G', '--generator', help='CMake builds use the specified generator.')
  parser.add_argument(
      '-D', '--cmake_define',
      action='append',
      help='Passes argument through to CMake as a CMake variable via cmake -D.')
  parser.add_argument(
      '-t', '--with_transcoder',
      action='store_true',
      help='Run tests with Draco transcoder support enabled.')
  parser.add_argument(
      '-v',
      '--verbose',
      action='store_true',
      help='Show configuration and build output.')
  args = parser.parse_args()

  if args.cmake_define:
    CMAKE_DEFINES = args.cmake_define
  if args.generator:
    CMAKE_GENERATOR = args.generator
  if args.verbose:
    VERBOSE = True
  if args.with_transcoder:
    ENABLE_TRANSCODER = True

  if VERBOSE:
    print(f'CMAKE={CMAKE}')
    print(f'CMAKE_DEFINES={CMAKE_DEFINES}')
    print(f'CMAKE_GENERATOR={CMAKE_GENERATOR}')
    print(f'CMAKE_AVAILABLE_GENERATORS={CMAKE_AVAILABLE_GENERATORS}')
    print(f'ENABLE_TRANSCODER={ENABLE_TRANSCODER}')
    print(f'DRACO_SOURCES_PATH={DRACO_SOURCES_PATH}')
    print(f'DRACO_SHARED_BUILD_PATH={DRACO_SHARED_BUILD_PATH}')
    print(f'DRACO_STATIC_BUILD_PATH={DRACO_STATIC_BUILD_PATH}')
    print(f'DRACO_SHARED_INSTALL_PATH={DRACO_SHARED_INSTALL_PATH}')
    print(f'DRACO_STATIC_INSTALL_PATH={DRACO_STATIC_INSTALL_PATH}')
    print(f'NUM_PROCESSES={NUM_PROCESSES}')
    print(f'TEST_SHARED_BUILD_PATH={TEST_SHARED_BUILD_PATH}')
    print(f'TEST_STATIC_BUILD_PATH={TEST_STATIC_BUILD_PATH}')
    print(f'TEST_SOURCES_PATH={TEST_SOURCES_PATH}')
    print(f'VERBOSE={VERBOSE}')

  if CMAKE_GENERATOR and CMAKE_GENERATOR not in CMAKE_AVAILABLE_GENERATORS:
    raise ValueError(f'CMake generator unavailable: {CMAKE_GENERATOR}.')

  test_draco_install()
