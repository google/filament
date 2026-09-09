use_relative_paths = True

gclient_gn_args_file = 'build/config/gclient_args.gni'

gclient_gn_args = [
  'android_ndk_version',
  'build_with_chromium',
  'dawn_node',
  'dawn_wasm',
  'generate_location_tags',
]

git_dependencies = 'SYNC'

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'dawn_git': 'https://dawn.googlesource.com',
  'github_git': 'https://github.com',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'dawn_standalone': True,
  'dawn_node': False, # Also fetches dependencies required for building NodeJS bindings.
  'dawn_wasm': False, # Also fetches dependencies required for building WebAssembly.
  'dawn_cmake_version': 'version:2@3.23.3',
  'dawn_cmake_win32_sha1': 'b106d66bcdc8a71ea2cdf5446091327bfdb1bcd7',
  'dawn_gn_version': 'git_revision:58933a7cdbc90f70f2381f0c72e76d29be1d43a9',
  # ninja CIPD package version.
  # https://chrome-infra-packages.appspot.com/p/infra/3pp/tools/ninja
  'dawn_ninja_version': 'version:3@1.12.1.chromium.4',
  'dawn_go_version': 'version:3@1.26.6',
  'dawn_node_version': 'version:2@20.11.0',
  'agility_sdk_version': 'version:2@1.721.0-preview',
  'dawn_bazelisk_version': 'version:3@1.29.0',
  'dawn_llvm-dev_version': 'version:3@22.1.0',
  'dawn_zstd_version': 'Lf-seQJdussz81cZXnez5xIxEdkejDRxKumQpHbHMOUC',

  # GN variable required by //testing that will be output in the gclient_args.gni
  'generate_location_tags': False,

  # Fetch clang-tidy into the same bin/ directory as our clang binary,
  # and Chromium's tools/build for tricium_clang_tidy_script.py.
  'checkout_clang_tidy': False,

  # Fetch clangd into the same bin/ directory as our clang binary.
  'checkout_clangd': False,

  # Fetch configuration files required for the 'use_remoteexec' gn arg
  'download_remoteexec_cfg': False,
  # RBE instance to use for running remote builds
  'rbe_instance': 'projects/rbe-chrome-untrusted/instances/default_instance',
  # RBE project to download rewrapper config files for. Only needed if
  # different from the project used in 'rbe_instance'
  'rewrapper_cfg_project': '',
  # reclient CIPD package
  'reclient_package': 'infra/rbe/client/',
  # reclient CIPD package version
  'reclient_version': 're_client_version:0.185.0.db415f21-gomaip',
  # siso CIPD package version.
  'siso_version': 'git_revision:bc45e8f67ae0f37d337190ad64aa8bb440c791eb',

  # CPython 3 CIPD package version for Siso hermetic toolchain.
  'cpython3_version': 'version:3@3.11.9.chromium.38',

  # 'magic' text to tell depot_tools that git submodules should be accepted
  # but parity with DEPS file is expected.
  'SUBMODULE_MIGRATION': 'True',

  'fetch_cmake': False,
  'fetch_bazel': False,

  # condition to allowlist deps to be synced in Cider. Allowlisting is needed
  # because not all deps are compatible with Cider. Once we migrate everything
  # to be compatible we can get rid of this allowlisting mecahnism and remove
  # this condition. Tracking bug for removing this condition: b/349365433
  'non_git_source': 'True',

  # Set to True by Chromium if syncing from a Chromium checkout.
  'build_with_chromium': False,

  # Version of Chromium the DEPS entries synced by scripts/roll_chromium_deps.py
  # were last synced to.
  'chromium_revision': 'a0ec23691ebb97ecd19abb0cec31d254190dee55',
  # We never want to actually checkout Chromium, but we need a fake DEPS entry
  # in order for the Chromium -> Dawn DEPS autoroller to work.
  'checkout_placeholder_chromium': False,

  # Checkout mesa 3D graphics library and related dependencies
  # Not used by Dawn/Tint directly, but is used as part of extended fuzzing stack.
  'checkout_mesa': False,

  # Checkout //tools/code_coverage from Chromium and fetches the
  # prebuilt versions of llvm-cov and llvm-profdata.
  'checkout_clang_coverage_tools': False,

  # Checkout LiteRT-LM and its data dependencies.
  # Not actually depended on by Dawn/Tint, only used to run benchmark tests on them.
  'checkout_litert_lm': False,

  # Dependency versions for Android builds.
  # These should be all rolled at once to match the values in Chromium.
  'android_ndk_version': Str('2@30.0.15729638'),
  'android_sdk_build-tools_version': 'version_37.0.0',
  'android_sdk_emulator_version': '9lGp8nTUCRRWGMnI_96HcKfzjnxEJKUcfvfwmA3wXNkC',
  'android_sdk_platform-tools_version': 'qTD9QdBlBf3dyHsN1lJ0RH6AhHxR42Hmg2Ih-Vj4zIEC',
  'android_sdk_platforms_version': 'WhtP32Q46ZHdTmgCgdauM3ws_H9iPoGKEZ_cPggcQ6wC',
}

deps = {
  'buildtools': {
    'url': '{chromium_git}/chromium/src/buildtools@4277578aa9c45906e51ad33cac1a5a7ad5288010',
    'condition': 'dawn_standalone',
  },
  'third_party/clang-format/script': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/clang/tools/clang-format.git@70510081984cfcdb14a15b3e08dfe9776dc7ed37',
    'condition': 'dawn_standalone',
  },
  'buildtools/linux64': {
    'packages': [{
      'package': 'gn/gn/linux-${{arch}}',
      'version': Var('dawn_gn_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'dawn_standalone and host_os == "linux"',
  },
  'buildtools/mac': {
    'packages': [{
      'package': 'gn/gn/mac-${{arch}}',
      'version': Var('dawn_gn_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'dawn_standalone and host_os == "mac"',
  },
  'buildtools/win': {
    'packages': [{
      'package': 'gn/gn/windows-amd64',
      'version': Var('dawn_gn_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'dawn_standalone and host_os == "win"',
  },

  'third_party/depot_tools': {
    'url': '{chromium_git}/chromium/tools/depot_tools.git@5cac02a8eccc60a1ba2a0804fc0a598f24f6137a',
    'condition': 'dawn_standalone',
  },

  'third_party/libc++/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libcxx.git@97b436da4c33663581d394f4ee0a5977fc38c2f4',
    'condition': 'dawn_standalone',
  },

  'third_party/libc++abi/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libcxxabi.git@fc1897a2c12aa27e703c3ed48b62eba8abf4ce19',
    'condition': 'dawn_standalone',
  },

  # Required by libc++
  'third_party/llvm-libc/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libc.git@8bbe89bde10b5eb59c4071079d6e3086c9b74415',
    'condition': 'dawn_standalone',
  },

  # Required by //build on Linux
  'third_party/libdrm/src': {
    'url': '{chromium_git}/chromiumos/third_party/libdrm.git@369990d9660a387f618d0eedc341eb285016243b',
    'condition': 'dawn_standalone and host_os == "linux"',
  },

  # Dependencies required to use GN, and Clang in standalone.

  # The //build and //tools/* deps should all be updated in unison, as
  #  there are dependencies between them.
  'build': {
  'url': '{chromium_git}/chromium/src/build@d46d57431ce6185ce83aaa6c7924b836f7fc142a',
    'condition': 'dawn_standalone',
  },
  'tools/clang': {
  'url': '{chromium_git}/chromium/src/tools/clang@1a1bb2ccdb1b6a06037babf091253c77755d6479',
    'condition': 'dawn_standalone',
  },
  'tools/memory': {
    'url': '{chromium_git}/chromium/src/tools/memory@8d3182bb3c86948e9e27fbfe11d2134c7e29c898',
    'condition': 'dawn_standalone',
  },
  'tools/valgrind': {
    'url': '{chromium_git}/chromium/src/tools/valgrind@81110bd0de83ed91ed9e998b52e70717f66b69a6',
    'condition': 'dawn_standalone',
  },
  'tools/win': {
    'url': Var('chromium_git') + '/chromium/src/tools/win@13cb6e5d223dc49eadd082d3aef4c2a5b0e4c0a0',
    'condition': 'checkout_win and not build_with_chromium',
  },
  'tools/code_coverage': {
    'url': '{chromium_git}/chromium/src/tools/code_coverage@74d04576e893e08d9d16c99866c7d6696713d554',
    'condition': 'dawn_standalone and checkout_clang_coverage_tools',
  },

  # For run-tricium-clang-tidy.py
  'third_party/chromium-tools-build/src': {
    'url': '{chromium_git}/chromium/tools/build@b4b9ba1d20dfd1fe849f43222716b27b2840f8fa',
    'condition': 'dawn_standalone and checkout_clang_tidy',
  },

  # Linux sysroots for hermetic builds instead of relying on whatever is
  # available from the system used for compilation. Only applicable to
  # dawn_standalone since Chromium has its own sysroot copy.
  'build/linux/debian_bullseye_armhf-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and checkout_arm',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': 'b45a7f586a107380ca6141b00d74321922b41d6d327dc33e74a2f82fd454304c',
        'sha256sum': 'b45a7f586a107380ca6141b00d74321922b41d6d327dc33e74a2f82fd454304c',
        'size_bytes': 18374340,
        'generation': 1770327986819219,
      },
    ],
  },
  'build/linux/debian_bullseye_arm64-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and checkout_arm64',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': 'c7176a4c7aacbf46bda58a029f39f79a68008d3dee6518f154dcf5161a5486d8',
        'sha256sum': 'c7176a4c7aacbf46bda58a029f39f79a68008d3dee6518f154dcf5161a5486d8',
        'size_bytes': 18420984,
        'generation': 1770327978874031,
      },
    ],
  },
  'build/linux/debian_bullseye_i386-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and (checkout_x86 or checkout_x64)',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': '3de724b0d63478e1ae35f07b95d02261581a66e05c19aebe4e443d76179a565e',
        'sha256sum': '3de724b0d63478e1ae35f07b95d02261581a66e05c19aebe4e443d76179a565e',
        'size_bytes': 19768196,
        'generation': 1770327987132454,
      },
    ],
  },
  'build/linux/debian_bullseye_mipsel-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and checkout_mips',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': '82e930d6fa5d5ab1172cabc63b911ec800b182b4f8c14a273a89596541fe8658',
        'sha256sum': '82e930d6fa5d5ab1172cabc63b911ec800b182b4f8c14a273a89596541fe8658',
        'size_bytes': 18613672,
        'generation': 1770327971826284,
      },
    ],
  },
  'build/linux/debian_bullseye_mips64el-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and checkout_mips64',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': 'c847a32ae492aa14688be47fa696026e8dae8d9f4f589ec42fad29862bf311b3',
        'sha256sum': 'c847a32ae492aa14688be47fa696026e8dae8d9f4f589ec42fad29862bf311b3',
        'size_bytes': 19042256,
        'generation': 1770327970830699,
      },
    ],
  },
  'build/linux/debian_bullseye_amd64-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and checkout_x64',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': '52d61d4446ffebfaa3dda2cd02da4ab4876ff237853f46d273e7f9b666652e1d',
        'sha256sum': '52d61d4446ffebfaa3dda2cd02da4ab4876ff237853f46d273e7f9b666652e1d',
        'size_bytes': 19727236,
        'generation': 1770327973518330,
      },
    ],
  },

  # Used for Dawn-side GN arg definitions.
  'tools/mb': {
    'url': '{chromium_git}/chromium/src/tools/mb@4280c2f3ddc166978180f09659dd0133e41a1584',
    'condition': 'dawn_standalone',
  },

  # Testing, GTest and GMock
  'testing': {
    'url': '{chromium_git}/chromium/src/testing@52b41d5ba546fc7f487fc8b82e0a69d4636ee494',
    'condition': 'dawn_standalone',
  },
  'third_party/libFuzzer/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/compiler-rt/lib/fuzzer.git' + '@' + '5811dc57603eac1fa0e76addedb77f72f62bfa2d',
    'condition': 'dawn_standalone',
  },
  'third_party/googletest/src': {
    'url': '{chromium_git}/external/github.com/google/googletest@4fe3307fb2d9f86d19777c7eb0e4809e9694dde7',
    'condition': 'dawn_standalone',
  },
  # This is a dependency of //testing
  'third_party/catapult': {
    'url': '{chromium_git}/catapult.git@1d18f6e11082de030c45fd55b556d15e3aa628a8',
    'condition': 'dawn_standalone',
  },
  'third_party/google_benchmark/src': {
    'url': '{chromium_git}/external/github.com/google/benchmark.git' + '@' + '8abf1e701fbd88c8170f48fe0558247e2e5f8e7d',
    'condition': 'dawn_standalone',
  },
  'third_party/perfetto': {
    'url': '{chromium_git}/external/github.com/google/perfetto.git@15524260d4a9a6228e688cbbbfa4024161d0d9f3',
    'condition': 'dawn_standalone',
  },

  # Required for fuzzing Mesa via tint fuzzers
  'third_party/mesa/src': {
    'url': '{chromium_git}/external/gitlab.freedesktop.org/mesa/mesa/@2e683eb7385c54f872acc47b371210d2282bc103',
    'condition': 'checkout_mesa and host_os == "linux"',
  },
  'third_party/meson/src': {
    'url': '{chromium_git}/external/github.com/mesonbuild/meson@d389906a136c2aac9820ded0f38d1e25ef25fb9a',
    'condition': 'checkout_mesa and host_os == "linux"',
  },
  'third_party/llvm-dev': {
    'packages': [{
      'package': 'infra/3pp/tools/llvm-dev/linux-amd64',
      'version': Var('dawn_llvm-dev_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'checkout_mesa and host_os == "linux"',
  },
  'third_party/zstd': {
    'packages': [{
      'package': 'infra/3pp/static_libs/libzstd/linux-amd64',
      'version': Var('dawn_zstd_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'checkout_mesa and host_os == "linux"',
  },

  # Jinja2 and MarkupSafe for the code generator
  'third_party/jinja2': {
    'url': '{chromium_git}/chromium/src/third_party/jinja2@c3027d884967773057bf74b957e3fea87e5df4d7',
    'condition': 'dawn_standalone',
  },
  'third_party/markupsafe': {
    'url': '{chromium_git}/chromium/src/third_party/markupsafe@4256084ae14175d38a3ff7d739dca83ae49ccec6',
    'condition': 'dawn_standalone',
  },

  # GLFW for tests and samples
  'third_party/glfw3/src': {
    'url': '{chromium_git}/external/github.com/glfw/glfw@92dcf4ce74f2e2554a98fea09be7c705c17daa5a',
  },

  'third_party/vulkan_memory_allocator': {
    'url': '{chromium_git}/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator@7e55b011e16182fc349149abbd3aaf3b1db46421',
    'condition': 'dawn_standalone',
  },

  'third_party/angle': {
    'url': '{chromium_git}/angle/angle@6f066af1045bd2f675da35532aa3aedb5e2cb6a9',
    'condition': 'dawn_standalone',
  },

  'third_party/swiftshader': {
    'url': '{swiftshader_git}/SwiftShader@6b8d31709ad185dbd64e80865e830a9dbe8e7559',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-deps': {
    'url': '{chromium_git}/vulkan-deps@ab1905c50690dc629873b9081e9f54b821897e74',
    'condition': 'dawn_standalone',
  },

  'third_party/glslang/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/glslang@cc59a21628dc2adcb61bff2852bf78d944dc641f',
    'condition': 'dawn_standalone',
  },

  'third_party/spirv-headers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Headers@0d25db97cb9b8f725e4c95e4553001710e7fc39d',
    'condition': 'dawn_standalone',
  },

  'third_party/spirv-tools/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Tools@47c74f488bad1136559f382ce99e8e52d7a392cd',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-headers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Headers@b51f6b865c18fc5b33990d12f75e8dfd672cede6',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-loader/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Loader@eede24b3f9f789eabf1838391f3a84ce61238fb6',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-tools/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Tools@697e22d5d256b29e4d6cdd75b3cd42f1aa634113',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-utility-libraries/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Utility-Libraries@245b48c522b5375c0acd5377d52bef5e4917f31e',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-validation-layers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-ValidationLayers@a47ec18642d9564c01e3b4bb9e8a4ba79ee433e3',
    'condition': 'dawn_standalone',
  },

  'third_party/zlib': {
    'url': '{chromium_git}/chromium/src/third_party/zlib@0165316260f597826ab93d4580faebd56e4f002d',
    'condition': 'dawn_standalone',
  },

  'third_party/abseil-cpp': {
    'url': '{chromium_git}/chromium/src/third_party/abseil-cpp@3a994d3a4560406a1101ae4fb3a496e7c6604ca9',
    'condition': 'dawn_standalone',
  },

  'third_party/directx-shader-compiler/src': {
    'url': '{chromium_git}/external/github.com/microsoft/DirectXShaderCompiler@81c026b815e6aee2ad6558357fc7ef56f3901cd9',
  },

  'third_party/directx-headers/src': {
    # The non-Windows build of DXC depends on DirectX-Headers, and at a specific commit (not ToT)
    'url': '{chromium_git}/external/github.com/microsoft/DirectX-Headers@980971e835876dc0cde415e8f9bc646e64667bf7',
    'condition': 'host_os != "win"',
  },

  'third_party/agility-sdk/src': {
    'packages': [
      {
        'package': 'chromium/third_party/agility-sdk',
        'version': Var('agility_sdk_version'),
      },
    ],
    'condition': 'host_os == "win"',
    'dep_type': 'cipd',
  },

  'third_party/OpenGL-Registry/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/OpenGL-Registry@e8f7cd0e35ac8d6f5667a021ff83d04b1fec41ef',
  },

  'third_party/EGL-Registry/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/EGL-Registry@3d7796b3721d93976b6bfe536aa97bbc4bce8667',
  },

  # WebGPU CTS - Used both by the dawn_node tests and transitively by Chromium.
  'third_party/webgpu-cts': {
    'url': '{chromium_git}/external/github.com/gpuweb/cts@c354d28170af04797c053d223ff53dbdb9e17b8d',
    'condition': 'build_with_chromium or dawn_standalone',
  },

  # Dependencies required to build / run WebAssembly bindings
  'third_party/emsdk': {
    # Note: Always use an emsdk hash referring to a tagged release, just so
    # emsdk and emscripten are always in sync with an exact release.
    'url': '{chromium_git}/external/github.com/emscripten-core/emsdk.git@948c31acd3f369a5da276e33ab2ed57108c165e5',
    'condition': 'dawn_wasm',
  },

  # Dependencies required to build / run Dawn NodeJS bindings
  'third_party/node-api-headers': {
    'url': '{chromium_git}/external/github.com/nodejs/node-api-headers@d5cfe19da8b974ca35764dd1c73b91d57cd3c4ce',
    'condition': 'dawn_node',
  },
  'third_party/node-addon-api': {
    'url': '{chromium_git}/external/github.com/nodejs/node-addon-api@1e26dcb52829a74260ec262edb41fc22998669b6',
    'condition': 'dawn_node',
  },
  'third_party/gpuweb': {
    'url': '{chromium_git}/external/github.com/gpuweb/gpuweb@e95743d3940e0ff3c267ab55ced9ae6120c7d416',
    'condition': 'dawn_node',
  },

  # Node binaries, when dawn_node or dawn_wasm is enabled. Architectures are
  # listed out explicitly instead of using ${{platform}} because the
  # architecture that these are fetched on can differ from the architecture that
  # tests are ultimately run on, such as x64 vs. ARM64 Mac.
  'third_party/node/linux-amd64': {
    'packages': [
      {
        'package': 'infra/3pp/tools/nodejs/linux-amd64',
        'version': Var('dawn_node_version'),
      },
    ],
    'condition': 'checkout_linux and (dawn_node or dawn_wasm)',
    'dep_type': 'cipd',
  },
  'third_party/node/linux-arm64': {
    'packages': [
      {
        'package': 'infra/3pp/tools/nodejs/linux-arm64',
        'version': Var('dawn_node_version'),
      },
    ],
    'condition': 'checkout_linux and (dawn_node or dawn_wasm)',
    'dep_type': 'cipd',
  },
  'third_party/node/mac-amd64': {
    'packages': [
      {
        'package': 'infra/3pp/tools/nodejs/mac-amd64',
        'version': Var('dawn_node_version'),
      },
    ],
    'condition': 'checkout_mac and (dawn_node or dawn_wasm)',
    'dep_type': 'cipd',
  },
  'third_party/node/mac-arm64': {
    'packages': [
      {
        'package': 'infra/3pp/tools/nodejs/mac-arm64',
        'version': Var('dawn_node_version'),
      },
    ],
    'condition': 'checkout_mac and (dawn_node or dawn_wasm)',
    'dep_type': 'cipd',
  },
  'third_party/node/windows-amd64': {
    'packages': [
      {
        'package': 'infra/3pp/tools/nodejs/windows-amd64',
        'version': Var('dawn_node_version'),
      },
    ],
    'condition': 'checkout_win and (dawn_node or dawn_wasm)',
    'dep_type': 'cipd',
  },
  'third_party/node/windows-arm64': {
    'packages': [
      {
        'package': 'infra/3pp/tools/nodejs/windows-arm64',
        'version': Var('dawn_node_version'),
      },
    ],
    'condition': 'checkout_win and (dawn_node or dawn_wasm)',
    'dep_type': 'cipd',
  },

  # Upstream webgpu.h headers for testing purposes
  'third_party/webgpu-headers/src': {
    'url': '{chromium_git}/external/github.com/webgpu-native/webgpu-headers@01addc4ba8a2915a061b7095a6768b512071ab96',
  },

  # Like the Node dependency, architectures are listed out explicitly instead of
  # using ${{platform}} because the architecture that these are fetched on
  # can differ from the architecture that they are ultimately run on.
  'tools/golang/linux-amd64': {
    'packages': [{
      'package': 'infra/3pp/tools/go/linux-amd64',
      'version': Var('dawn_go_version'),
    }],
    'dep_type': 'cipd',
    'condition': '(checkout_android or checkout_linux) and non_git_source',
  },
  'tools/golang/linux-arm64': {
    'packages': [{
      'package': 'infra/3pp/tools/go/linux-arm64',
      'version': Var('dawn_go_version'),
    }],
    'dep_type': 'cipd',
    'condition': '(checkout_android or checkout_linux) and non_git_source',
  },
  'tools/golang/mac-amd64': {
    'packages': [{
      'package': 'infra/3pp/tools/go/mac-amd64',
      'version': Var('dawn_go_version'),
    }],
    'dep_type': 'cipd',
    'condition': '(checkout_ios or checkout_mac) and non_git_source',
  },
  'tools/golang/mac-arm64': {
    'packages': [{
      'package': 'infra/3pp/tools/go/mac-arm64',
      'version': Var('dawn_go_version'),
    }],
    'dep_type': 'cipd',
    'condition': '(checkout_ios or checkout_mac) and non_git_source',
  },
  'tools/golang/windows-amd64': {
    'packages': [{
      'package': 'infra/3pp/tools/go/windows-amd64',
      'version': Var('dawn_go_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'checkout_win and non_git_source',
  },
  'tools/golang/windows-arm64': {
    'packages': [{
      'package': 'infra/3pp/tools/go/windows-arm64',
      'version': Var('dawn_go_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'checkout_win and non_git_source',
  },

  'tools/cmake': {
    'condition': '(fetch_cmake or dawn_node)',
    'packages': [{
      'package': 'infra/3pp/tools/cmake/${{platform}}',
      'version': Var('dawn_cmake_version'),
    }],
    'dep_type': 'cipd',
  },

  'tools/bazelisk': {
    'condition': 'fetch_bazel or checkout_litert_lm',
    'packages': [{
      'package': 'infra/3pp/tools/bazelisk/${{platform}}',
      'version': Var('dawn_bazelisk_version'),
    }],
    'dep_type': 'cipd',
  },

  'third_party/ninja': {
    'packages': [
      {
        'package': 'infra/3pp/tools/ninja/${{platform}}',
        'version': Var('dawn_ninja_version'),
      }
    ],
    'dep_type': 'cipd',
  },
  'third_party/siso/cipd': {
    'packages': [
      {
        'package': 'build/siso/${{platform}}',
        'version': Var('siso_version'),
      }
    ],
    'condition': 'dawn_standalone and non_git_source',
    'dep_type': 'cipd',
  },

  # Always download Linux x64 package regardless of host OS for RBE workers.
  'third_party/cpython3/linux-amd64': {
    'packages': [
      {
        'package': 'infra/3pp/tools/cpython3/linux-amd64',
        'version': Var('cpython3_version'),
      }
    ],
    'condition': 'dawn_standalone and non_git_source',
    'dep_type': 'cipd',
  },
  # Host platform package.
  'third_party/cpython3/host': {
    'packages': [
      {
        'package': 'infra/3pp/tools/cpython3/${{platform}}',
        'version': Var('cpython3_version'),
      }
    ],
    'condition': 'dawn_standalone and non_git_source',
    'dep_type': 'cipd',
  },

  # RBE dependencies
  'buildtools/reclient': {
    'packages': [
      {
        'package': Var('reclient_package') + '${{platform}}',
        'version': Var('reclient_version'),
      }
    ],
    'condition': 'dawn_standalone and (host_cpu != "arm64" or host_os == "mac") and non_git_source',
    'dep_type': 'cipd',
  },

  # Misc dependencies inherited from Tint
  'third_party/protobuf': {
    'url': '{chromium_git}/chromium/src/third_party/protobuf@5f8c379d1fc89fe8eee16ae560dd5e514a4608da',
    'condition': 'dawn_standalone',
  },

  'tools/protoc_wrapper': {
    'url': '{chromium_git}/chromium/src/tools/protoc_wrapper@e9dbe1bf6a2a5d2d4973725874259eed587cf18d',
    'condition': 'dawn_standalone',
  },

  'third_party/libprotobuf-mutator/src': {
    'url': '{chromium_git}/external/github.com/google/libprotobuf-mutator.git@c1c950eae0440c3808f2b8bd7c57d0c6a42c1a90',
    'condition': 'dawn_standalone',
  },

  # Dependencies for PartitionAlloc.
  # Doc: https://docs.google.com/document/d/1wz45t0alQthsIU9P7_rQcfQyqnrBMXzrOjSzdQo-V-A
  'third_party/partition_alloc': {
    'url': '{chromium_git}/chromium/src/base/allocator/partition_allocator.git@584ccf1ce5c6b7bb78378d9bf2962a389db0413d',
    'condition': 'dawn_standalone',
  },

  # For Android builds in standalone.
  'third_party/android_sdk': {
    'url': Var('chromium_git') + '/chromium/src/third_party/android_sdk@c1f470e46a9339e80079350041b457c5593b6af1',
    'condition': 'checkout_android and dawn_standalone',
  },
  'third_party/android_sdk/public': {
    'packages': [
      {
        'package': 'chromium/third_party/android_sdk/public/build-tools/37.0.0/${{os}}',
        'version': Var('android_sdk_build-tools_version'),
      },
      {
        'package': 'chromium/third_party/android_sdk/public/emulator',
        'version': Var('android_sdk_emulator_version'),
      },
      {
        'package': 'chromium/third_party/android_sdk/public/platform-tools',
        'version': Var('android_sdk_platform-tools_version'),
      },
      {
        'package': 'chromium/third_party/android_sdk/public/platforms/android-37.0',
        'version': Var('android_sdk_platforms_version'),
      },
      {
        'package': 'chromium/third_party/android_sdk/public/cmdline-tools/linux',
        'version': 'vO477CZmV97fP5b9ZnNGZYK-23J4g_6FPPhhdSNvTfsC',
      },
    ],
    'condition': 'checkout_android and dawn_standalone',
    'dep_type': 'cipd',
  },
  'third_party/android_toolchain/ndk': {
    'packages': [
      {
        'package': 'chromium/third_party/android_toolchain/android_toolchain',
        'version': 'version:' + Var('android_ndk_version'),
      },
    ],
    'condition': 'checkout_android and dawn_standalone',
    'dep_type': 'cipd',
  },
  'third_party/jdk/current': {
    'packages': [
      {
        'package': 'chromium/third_party/jdk/linux-amd64',
        'version': 'Ac1SdNGLeE0dSBE6mDm4oq8NvcHCLDgYMWYZki0QkVAC',
      },
    ],
    'condition': 'checkout_android and dawn_standalone and non_git_source',
    'dep_type': 'cipd',
  },
  'third_party/android_build_tools': {
    'url': Var('chromium_git') + '/chromium/src/third_party/android_build_tools@92ec157f81da73642e4a921d67a8993d65f0f9a7',
    'condition': 'checkout_android and dawn_standalone',
  },
  'third_party/android_build_tools/aapt2/cipd': {
    'packages': [
      {
        'package': 'chromium/third_party/android_build_tools/aapt2',
        'version': '7tEuuB92wV8xh54fCO0bRk_6FS_7XtsBl9LB5Tf5d0AC',
      },
    ],
    'condition': 'checkout_android and dawn_standalone',
    'dep_type': 'cipd',
  },
  'third_party/android_build_tools/manifest_merger/cipd': {
    'packages': [
      {
        'package': 'chromium/third_party/android_build_tools/manifest_merger',
        'version': '9l5PKlY2DkqA4h5g7F1hXEIuTUH_B_zAYR_e58siHZgC',
      },
    ],
    'condition': 'checkout_android and dawn_standalone',
    'dep_type': 'cipd',
  },
  'third_party/cpu_features/src': {
    'url': Var('chromium_git') + '/external/github.com/google/cpu_features.git' + '@' + '81d13c49649f0714dd41fb56bb246398b6584085',
    'condition': 'checkout_android and dawn_standalone',
  },
  'third_party/ijar': {
    'url': Var('chromium_git') + '/chromium/src/third_party/ijar@94af60a05b33f9acb33477a8d969e48eb1c3029f',
    'condition': 'checkout_android and dawn_standalone',
  },
  'third_party/libunwind/src': {
    'url': Var('chromium_git') + '/external/github.com/llvm/llvm-project/libunwind.git@0790de42df7835816d95ce62c598f9fef5ef4e6c',
    'condition': 'checkout_android and dawn_standalone',
  },

  # We never want to actually checkout Chromium, but we need a fake DEPS entry
  # in order for the Chromium -> Dawn DEPS autoroller to work. Note that this
  # only currently works due to an explicit exception in presubmit checks
  # https://source.chromium.org/chromium/chromium/tools/depot_tools/+/dac161882feaedaabcf99db47726a999e4834a13:presubmit_canned_checks.py;l=2139
  'third_party/placeholder_chromium': {
    'url': '{chromium_git}/chromium/src.git' + '@' + Var('chromium_revision'),
    'condition': 'checkout_placeholder_chromium',
  },

  'third_party/litert-lm/src': {
    'url': '{chromium_git}/external/github.com/google-ai-edge/LiteRT-LM.git@df9bb057d50741d5a044de2c2ea4f97d72206949',
    'condition': 'checkout_litert_lm',
  },

  'third_party/litert-lm/data': {
    'packages': [
      {
        # TODO(crbug.com/527944617): Replace experimental CIPD dependency.
        'package': 'experimental/chouinard_at_google.com/litert_lm_benchmark_data',
        'version': '3gymze25YpJCADvh_zTc0MCNgeLcMPkh8xcfb_HjkAQC',
      }
    ],
    'dep_type': 'cipd',
    'condition': 'checkout_litert_lm',
  },
}

hooks = [
  {
    # Ensure that the DEPS'd "depot_tools" has its self-update capability
    # disabled.
    'name': 'disable_depot_tools_selfupdate',
    'pattern': '.',
    'condition': 'dawn_standalone',
    'action': [
        'vpython3',
        'third_party/depot_tools/update_depot_tools_toggle.py',
        '--disable',
    ],
  },

  # Pull the compilers and system libraries for hermetic builds
  {
    'name': 'sysroot_x86',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_linux and (checkout_x86 or checkout_x64)',
    'action': ['vpython3', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x86'],
  },
  {
    'name': 'sysroot_x64',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_linux and checkout_x64',
    'action': ['vpython3', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x64'],
  },
  {
    # Update the Mac toolchain if possible, this makes builders use "hermetic XCode" which is
    # is more consistent (only changes when rolling build/) and is cached.
    'name': 'mac_toolchain',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_mac',
    'action': ['vpython3', 'build/mac_toolchain.py'],
  },
  {
    # Create the required ranlib symlink for hermetic xcode
    'name': 'mac_ranlib_symlink',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_mac',
    'action': ['vpython3', 'tools/create_ranlib_symlink.py'],
  },
  {
    # Case-insensitivity for the Win SDK. Must run before win_toolchain below.
    'name': 'ciopfs_linux',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_win and host_os == "linux"',
    'action': [ 'vpython3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--bucket', 'chromium-browser-clang/ciopfs',
                '-s', 'build/ciopfs.sha1',
    ]
  },
  {
    # Update the Windows toolchain if necessary. Must run before 'clang' below.
    'name': 'win_toolchain',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_win',
    'action': ['vpython3', 'build/vs_toolchain.py', 'update', '--force'],
  },
  {
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    'action': ['vpython3', 'tools/clang/scripts/update.py',
               '--package=clang'],
    'condition': 'dawn_standalone',
  },
  {
    # This is also supposed to support the same set of platforms as 'clang'
    # above. LLVM ToT support isn't provided at the moment.
    'name': 'clang_tidy',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_clang_tidy',
    'action': ['vpython3', 'tools/clang/scripts/update.py',
               '--package=clang-tidy'],
  },
  {
    # This is also supposed to support the same set of platforms as 'clang'
    # above. LLVM ToT support isn't provided at the moment.
    'name': 'clangd',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_clangd',
    'action': ['vpython3', 'tools/clang/scripts/update.py',
               '--package=clangd'],
  },
  {
    'name': 'objdump',
    'pattern': '.',
    'action': ['vpython3', 'tools/clang/scripts/update.py',
               '--package=objdump'],
    'condition': 'dawn_standalone',
  },
  {
    'name': 'coverage_tools',
    'pattern': '.',
    'action': ['vpython3', 'tools/clang/scripts/update.py',
               '--package=coverage_tools'],
    'condition': 'dawn_standalone and checkout_clang_coverage_tools',
  },

  # Pull dsymutil binaries using checked-in hashes.
  {
    'name': 'dsymutil_mac_arm64',
    'pattern': '.',
    'condition': 'dawn_standalone and host_os == "mac" and host_cpu == "arm64"',
    'action': [ 'vpython3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--bucket', 'chromium-browser-clang',
                '-s', 'tools/clang/dsymutil/bin/dsymutil.arm64.sha1',
                '-o', 'tools/clang/dsymutil/bin/dsymutil',
    ],
  },
  {
    'name': 'dsymutil_mac_x64',
    'pattern': '.',
    'condition': 'dawn_standalone and host_os == "mac" and host_cpu == "x64"',
    'action': [ 'vpython3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--bucket', 'chromium-browser-clang',
                '-s', 'tools/clang/dsymutil/bin/dsymutil.x64.sha1',
                '-o', 'tools/clang/dsymutil/bin/dsymutil',
    ],
  },
  # Pull rc binaries using checked-in hashes.
  {
    'name': 'rc_win',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_win and host_os == "win"',
    'action': [ 'vpython3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--bucket', 'chromium-browser-clang/rc',
                '-s', 'build/toolchain/win/rc/win/rc.exe.sha1',
    ],
  },
  {
    'name': 'rc_linux',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_win and host_os == "linux"',
    'action': [ 'vpython3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--bucket', 'chromium-browser-clang/rc',
                '-s', 'build/toolchain/win/rc/linux64/rc.sha1',
    ],
  },
  {
    'name': 'rc_mac',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_win and host_os == "mac"',
    'action': [ 'vpython3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--bucket', 'chromium-browser-clang/rc',
                '-s', 'build/toolchain/win/rc/mac/rc.sha1',
    ],
  },

  # Update build/util/LASTCHANGE.
  {
    'name': 'lastchange',
    'pattern': '.',
    'condition': 'dawn_standalone',
    'action': ['vpython3', 'build/util/lastchange.py',
               '-o', 'build/util/LASTCHANGE'],
  },

  # Activate emsdk for WebAssembly builds
  {
    'name': 'activate_emsdk',
    'pattern': '.',
    'condition': 'dawn_wasm',
    'action': [ 'vpython3', 'tools/activate-emsdk' ],
  },

  # Configure remote exec cfg files
  {
    # Use luci_auth if on windows and using chrome-untrusted project
    'name': 'download_and_configure_reclient_cfgs',
    'pattern': '.',
    'condition': 'dawn_standalone and download_remoteexec_cfg and host_os == "win"',
    'action': ['vpython3',
               'buildtools/reclient_cfgs/configure_reclient_cfgs.py',
               '--rbe_instance',
               Var('rbe_instance'),
               '--reproxy_cfg_template',
               'reproxy.cfg.template',
               '--rewrapper_cfg_project',
               Var('rewrapper_cfg_project'),
               '--use_luci_auth_credshelper',
               '--quiet',
               ],
  },  {
    'name': 'download_and_configure_reclient_cfgs',
    'pattern': '.',
    'condition': 'dawn_standalone and download_remoteexec_cfg and not host_os == "win"',
    'action': ['vpython3',
               'buildtools/reclient_cfgs/configure_reclient_cfgs.py',
               '--rbe_instance',
               Var('rbe_instance'),
               '--reproxy_cfg_template',
               'reproxy.cfg.template',
               '--rewrapper_cfg_project',
               Var('rewrapper_cfg_project'),
               '--quiet',
               ],
  },
  {
    'name': 'configure_reclient_cfgs',
    'pattern': '.',
    'condition': 'dawn_standalone and not download_remoteexec_cfg',
    'action': ['vpython3',
               'buildtools/reclient_cfgs/configure_reclient_cfgs.py',
               '--rbe_instance',
               Var('rbe_instance'),
               '--reproxy_cfg_template',
               'reproxy.cfg.template',
               '--rewrapper_cfg_project',
               Var('rewrapper_cfg_project'),
               '--skip_remoteexec_cfg_fetch',
               '--quiet',
               ],
  },
  # Configure Siso for developer builds.
  {
    'name': 'configure_siso',
    'pattern': '.',
    'condition': 'dawn_standalone',
    'action': ['vpython3',
               'build/config/siso/configure_siso.py',
               '--rbe_instance',
               Var('rbe_instance'),
               ],
  },
]

recursedeps = [
  'buildtools',
]
