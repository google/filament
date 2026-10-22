# Copyright 2026 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Usage:
#   1. Get Chromium depot_tools, and put it in your path.
#      See https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up
#
#   2. From the SPIRV-Tools source tree root:
#
#      # Get additional build tooling, using the info in the DEPS file.
#      cp utils/standalone.gclient .
#      gclient sync --gclientfile=standalone.gclient
#
#   3. One of the tools downloaded in the previous step is the binary for 'gn'.
#      Put it on your path.
#
#      # This is for Linux amd64 environments:
#      PATH=$(pwd)/build/linux64:$PATH
#
#   3. Create a build tree with Ninja build recipes.
#
#      gn gen out/Debug
#
#      # Or build a release build:
#      gn gen out/Release --args="is_debug=false"
#
#   4. Use ninja to build the project
#
#      ninja -C out/Debug

use_relative_paths = True

gclient_gn_args_file = 'build/config/gclient_args.gni'

# Emit this vars for use in .gn and .gni files
gclient_gn_args = [
  # Needed by //build/toolchain/linux/BUILD.gn
  # which was needed by 'action' in ./BUILD.gn
  'build_with_chromium',
  # Needed by //testing/test.gni
  'generate_location_tags',
]

git_dependencies = 'SYNC'

vars = {
  # Repo sources
  'chromium_git': 'https://chromium.googlesource.com',
  'github': 'https://github.com',

  # Building context
  'build_with_chromium': False,
  'spirv_tools_standalone': True,
  'generate_location_tags': False,

  # Commits for GN-related tooling
  'spirv_tools_gn_version': 'git_revision:ec56d4d935a0e2ab9d52b88dd00c93ec51233055',
  'spirv_tools_build_version': '2316930a88b15a036fa5f72e2b2c2ba08e2904a0',

  # Commits for SPIRV-Tools dependencies

  'abseil_revision': 'ba427a3dd37a1948aaca5cda75191cc4b88b4fa8',
  'effcee_revision': 'f8e8a164822d4f65e757bff66bc00e1567959aa0',
  'googletest_revision': '49495eacfdbda3f4b6ba219923fedbb2e3f99376',
  # Use a recent protobuf, which can depend on abseil
  'protobuf_revision': '35cd01f9fe9afbeea38cc7b979a3b6bfcde82c03',
  're2_revision': '972a15cedd008d846f1a39b2e88ce48d7f166cbd',
  'spirv_headers_revision': 'f0bf307f7c49d26484db596185cece53c37701fc',
  'mimalloc_revision': 'fc1e2acbced0b3e893da1a1375e02ac159d0423f',

  # SPIRV-Tools standalone GN-only dependencies
  'chromium_testing_version': '555e7546214837372345fef14e25eed42ff2ea07',
}

deps = {
  'external/abseil_cpp':
      Var('github') + '/abseil/abseil-cpp.git@' + Var('abseil_revision'),

  'external/effcee':
      Var('github') + '/google/effcee.git@' + Var('effcee_revision'),

  'external/googletest':
      Var('github') + '/google/googletest.git@' + Var('googletest_revision'),

  'external/protobuf':
      Var('github') + '/protocolbuffers/protobuf.git@' + Var('protobuf_revision'),

  'external/re2':
      Var('github') + '/google/re2.git@' + Var('re2_revision'),

  'external/spirv-headers':
      Var('github') +  '/KhronosGroup/SPIRV-Headers.git@' +
          Var('spirv_headers_revision'),

  'external/mimalloc':
      Var('github') + '/microsoft/mimalloc.git@' + Var('mimalloc_revision'),

  # Get buildtools.
  # We need the 'gn' binary.
  'buildtools': {
    'url': Var('chromium_git') + '/chromium/src/buildtools@11cc2bd83053cb790b7516aa3eb3f3935fb05a0e',
    'condition': 'spirv_tools_standalone',
  },
  'buildtools/linux64': {
    'packages': [{
      'package': 'gn/gn/linux-${{arch}}',
      'version': Var('spirv_tools_gn_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'spirv_tools_standalone and host_os == "linux"',
  },
  'buildtools/mac': {
    'packages': [{
      'package': 'gn/gn/mac-${{arch}}',
      'version': Var('spirv_tools_gn_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'spirv_tools_standalone and host_os == "mac"',
  },
  'buildtools/win': {
    'packages': [{
      'package': 'gn/gn/windows-amd64',
      'version': Var('spirv_tools_gn_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'spirv_tools_standalone and host_os == "win"',
  },

  # Get //build from Chromium.
  # It contains the generic //build/config/BUILDCONFIG.gn that sets up most rules
  # for C++ compilation.
  'build': {
  'url': Var('chromium_git') + '/chromium/src/build@' + Var('spirv_tools_build_version'),
    'condition': 'spirv_tools_standalone',
  },
  # For fuzzing
  'testing': {
    'url': Var('chromium_git') + '/chromium/src/testing@' + Var('chromium_testing_version'),
    'condition': 'spirv_tools_standalone',
  },
}

hooks = [
  # The Chromium BUILDCONFIG.gn has a false dependency on the sysroots.
  # Pull the compilers and system libraries for hermetic builds
  {
    'name': 'sysroot_x86',
    'pattern': '.',
    'condition': 'spirv_tools_standalone and checkout_linux and checkout_x86',
    'action': ['vpython3', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x86'],
  },
  {
    'name': 'sysroot_x64',
    'pattern': '.',
    'condition': 'spirv_tools_standalone and checkout_linux and checkout_x64',
    'action': ['vpython3', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x64'],
  },
]


recursedeps = [
  'buildtools',
]
