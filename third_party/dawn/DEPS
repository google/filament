use_relative_paths = True

gclient_gn_args_file = 'build/config/gclient_args.gni'

gclient_gn_args = [
  'build_with_chromium',
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
  'dawn_tintd': False, # Also fetches dependencies required for building tintd.
  'dawn_cmake_version': 'version:2@3.23.3',
  'dawn_cmake_win32_sha1': 'b106d66bcdc8a71ea2cdf5446091327bfdb1bcd7',
  'dawn_gn_version': 'git_revision:ec56d4d935a0e2ab9d52b88dd00c93ec51233055',
  # ninja CIPD package version.
  # https://chrome-infra-packages.appspot.com/p/infra/3pp/tools/ninja
  'dawn_ninja_version': 'version:3@1.12.1.chromium.4',
  'dawn_go_version': 'version:2@1.21.3',
  'dawn_node_version': 'version:2@20.11.0',

  # GN variable required by //testing that will be output in the gclient_args.gni
  'generate_location_tags': False,

  # Fetch clang-tidy into the same bin/ directory as our clang binary.
  'checkout_clang_tidy': False,

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
  'siso_version': 'git_revision:7bcc56206d234b3024df08ffd4ee6d40eea32623',

  # 'magic' text to tell depot_tools that git submodules should be accepted
  # but parity with DEPS file is expected.
  'SUBMODULE_MIGRATION': 'True',

  'fetch_cmake': False,

  # condition to allowlist deps to be synced in Cider. Allowlisting is needed
  # because not all deps are compatible with Cider. Once we migrate everything
  # to be compatible we can get rid of this allowlisting mecahnism and remove
  # this condition. Tracking bug for removing this condition: b/349365433
  'non_git_source': 'True',

  # Set to True by Chromium if syncing from a Chromium checkout.
  'build_with_chromium': False,

  # Version of Chromium the DEPS entries synced by scripts/roll_chromium_deps.py
  # were last synced to.
  'chromium_revision': 'c8fb3065f8298a1e27cb02493463609c90f556d9',
  # We never want to actually checkout Chromium, but we need a fake DEPS entry
  # in order for the Chromium -> Dawn DEPS autoroller to work.
  'checkout_placeholder_chromium': False,

  # Checkout mesa 3D graphics library and related dependencies
  # Not used by Dawn/Tint directly, but is used as part of an
  #  experiment in the fuzzing stack.
  'checkout_mesa': False,

  # Checkout //tools/code_coverage from Chromium and fetches the
  # prebuilt versions of llvm-cov and llvm-profdata.
  'checkout_clang_coverage_tools': False,
}

deps = {
  'buildtools': {
    'url': '{chromium_git}/chromium/src/buildtools@11cc2bd83053cb790b7516aa3eb3f3935fb05a0e',
    'condition': 'dawn_standalone',
  },
  'third_party/clang-format/script': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/clang/tools/clang-format.git@08cce2b81a4d9801ebeaa8cdb43b241475a4d521',
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
    'url': '{chromium_git}/chromium/tools/depot_tools.git@bac00505a8c117dd48d465ee853c595981dd5e62',
    'condition': 'dawn_standalone',
  },

  'third_party/libc++/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libcxx.git@7ab65651aed6802d2599dcb7a73b1f82d5179d05',
    'condition': 'dawn_standalone',
  },

  'third_party/libc++abi/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libcxxabi.git@8f11bb1d4438d0239d0dfc1bd9456a9f31629dda',
    'condition': 'dawn_standalone',
  },

  # Required by libc++
  'third_party/llvm-libc/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libc.git@95fabdf43412fa079e11842600edb2766b5d2d46',
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
  'url': '{chromium_git}/chromium/src/build@2316930a88b15a036fa5f72e2b2c2ba08e2904a0',
    'condition': 'dawn_standalone',
  },
  'tools/clang': {
  'url': '{chromium_git}/chromium/src/tools/clang@772ec0f7db794d7c9bc4df748935ff13aaed36d7',
    'condition': 'dawn_standalone',
  },
  'tools/memory': {
    'url': '{chromium_git}/chromium/src/tools/memory@b635f27e932356a2e29450e5cfa544cdcc9ea6bb',
    'condition': 'dawn_standalone',
  },
  'tools/valgrind': {
    'url': '{chromium_git}/chromium/src/tools/valgrind@da34b95fdbf2032df6cda5f3828c2ba421592644',
    'condition': 'dawn_standalone',
  },
  'tools/win': {
    'url': Var('chromium_git') + '/chromium/src/tools/win@bfff5550ec231441c314c64878d860fdb6fdf174',
    'condition': 'checkout_win and not build_with_chromium',
  },
  'tools/code_coverage': {
    'url': '{chromium_git}/chromium/src/tools/code_coverage@f9e72c2a3ad1da4aeb22e754241269901d5be0e5',
    'condition': 'dawn_standalone and checkout_clang_coverage_tools',
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
    'url': '{chromium_git}/chromium/src/tools/mb@dfa6825f1ac6f78747be94bc3aa4193bf0b7faa6',
    'condition': 'dawn_standalone',
  },

  # Testing, GTest and GMock
  'testing': {
    'url': '{chromium_git}/chromium/src/testing@7e85557d0963a5c940ac992cb1c5666cd6af34dd',
    'condition': 'dawn_standalone',
  },
  'third_party/libFuzzer/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/compiler-rt/lib/fuzzer.git' + '@' + 'bea408a6e01f0f7e6c82a43121fe3af4506c932e',
    'condition': 'dawn_standalone',
  },
  'third_party/googletest': {
    'url': '{chromium_git}/external/github.com/google/googletest@4fe3307fb2d9f86d19777c7eb0e4809e9694dde7',
    'condition': 'dawn_standalone',
  },
  # This is a dependency of //testing
  'third_party/catapult': {
    'url': '{chromium_git}/catapult.git@5a34891efa6e41c8aca8842386b8ee528963ffdf',
    'condition': 'dawn_standalone',
  },
  'third_party/google_benchmark/src': {
    'url': '{chromium_git}/external/github.com/google/benchmark.git' + '@' + '8abf1e701fbd88c8170f48fe0558247e2e5f8e7d',
    'condition': 'dawn_standalone',
  },

  # Required for fuzzer + mesa experiment
  'third_party/mesa/src': {
    'url': '{chromium_git}/external/gitlab.freedesktop.org/mesa/mesa/@2e683eb7385c54f872acc47b371210d2282bc103',
    'condition': 'dawn_standalone and checkout_mesa',
  },
  'third_party/meson/src': {
    'url': '{chromium_git}/external/github.com/mesonbuild/meson@d389906a136c2aac9820ded0f38d1e25ef25fb9a',
    'condition': 'dawn_standalone and checkout_mesa',
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
    'url': '{chromium_git}/external/github.com/glfw/glfw@b00e6a8a88ad1b60c0a045e696301deb92c9a13e',
  },

  'third_party/vulkan_memory_allocator': {
    'url': '{chromium_git}/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator@cb0597213b0fcb999caa9ed08c2f88dc45eb7d50',
    'condition': 'dawn_standalone',
  },

  'third_party/angle': {
    'url': '{chromium_git}/angle/angle@fd958ac6533eb789e56801b558c9de15af17792c',
    'condition': 'dawn_standalone',
  },

  'third_party/swiftshader': {
    'url': '{swiftshader_git}/SwiftShader@89556131bf9d48af3c5c9fbb9a3322e706da89a3',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-deps': {
    'url': '{chromium_git}/vulkan-deps@e6dcb3f62da7ed41dd80cfe762e531d7d89e4ec5',
    'condition': 'dawn_standalone',
  },

  'third_party/glslang/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/glslang@dcf1aaa6fd7dc2081f17aa0a4f1590a76473d961',
    'condition': 'dawn_standalone',
  },

  'third_party/spirv-headers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Headers@869266ad9e6050197d87cf0a22aab59abf7ad008',
    'condition': 'dawn_standalone',
  },

  'third_party/spirv-tools/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Tools@a52828d672ab1622c308d0a6fc99630b79561609',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-headers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Headers@f6a6f7ab165cedbfa2a7d0c93fe27a2d01ce09c8',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-loader/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Loader@1bf213b2a90181553fff35aeb6fa5c468dcfd35d',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-tools/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Tools@7c46da2b39036a80ce088576d5794bf39e667f56',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-utility-libraries/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Utility-Libraries@5f62a4c3c686d3d3d1b3973a4397dd4d4768f3ad',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-validation-layers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-ValidationLayers@2c76c8fd77eb3aaf0a77b65e8b4078196715d2a7',
    'condition': 'dawn_standalone',
  },

  'third_party/zlib': {
    'url': '{chromium_git}/chromium/src/third_party/zlib@b80f1d1e5256ac25f6aea3f31f13d458981cb1f9',
    'condition': 'dawn_standalone',
  },

  'third_party/abseil-cpp': {
    'url': '{chromium_git}/chromium/src/third_party/abseil-cpp@dd4432fb02583b7a5b2b4fbf2012f7053e286ce0',
    'condition': 'dawn_standalone',
  },

  # The third_party/dxc entry will be removed after we've rolled and
  # updated Chromium for the third_party/directx-shader-compiler/src entry
  'third_party/dxc': {
    'url': '{chromium_git}/external/github.com/microsoft/DirectXShaderCompiler@5f8d05f9b760ae2be0242cad8ed719d10058b98c',
  },
  'third_party/directx-shader-compiler/src': {
    'url': '{chromium_git}/external/github.com/microsoft/DirectXShaderCompiler@5f8d05f9b760ae2be0242cad8ed719d10058b98c',
  },

  # The third_party/dxheaders entry will be removed after we've rolled and
  # updated Chromium for the third_party/directx-headers/src entry
  'third_party/dxheaders': {
    # The non-Windows build of DXC depends on DirectX-Headers, and at a specific commit (not ToT)
    'url': '{chromium_git}/external/github.com/microsoft/DirectX-Headers@980971e835876dc0cde415e8f9bc646e64667bf7',
    'condition': 'host_os != "win"',
  },
  'third_party/directx-headers/src': {
    # The non-Windows build of DXC depends on DirectX-Headers, and at a specific commit (not ToT)
    'url': '{chromium_git}/external/github.com/microsoft/DirectX-Headers@980971e835876dc0cde415e8f9bc646e64667bf7',
    'condition': 'host_os != "win"',
  },

  'third_party/OpenGL-Registry/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/OpenGL-Registry@9cb90ca4902d588bef3c830fbb1da484893bd5fb',
  },

  'third_party/EGL-Registry/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/EGL-Registry@3d7796b3721d93976b6bfe536aa97bbc4bce8667',
  },

  # WebGPU CTS - Used both by the dawn_node tests and transitively by Chromium.
  'third_party/webgpu-cts': {
    'url': '{chromium_git}/external/github.com/gpuweb/cts@09fdb847d90d0b5bfe57068ce2eb9283cb77fc7f',
    'condition': 'build_with_chromium or dawn_standalone',
  },

  # Dependencies required to build / run WebAssembly bindings
  'third_party/emsdk': {
    # Note: Always use an emsdk hash referring to a tagged release, just so
    # emsdk and emscripten are always in sync with an exact release.
    'url': '{chromium_git}/external/github.com/emscripten-core/emsdk.git@b4258c35121c8d0e12f53568ffb22236d7816723',
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
    'url': '{chromium_git}/external/github.com/gpuweb/gpuweb@b4b5752ff755fe33bf6a67fb6e5964ba9d40dcdc',
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
    'url': '{chromium_git}/external/github.com/webgpu-native/webgpu-headers@dc16b3e531cf4f31be54236d1a3e988ba5f295a2',
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
    'condition': 'checkout_linux and non_git_source',
  },
  'tools/golang/linux-arm64': {
    'packages': [{
      'package': 'infra/3pp/tools/go/linux-arm64',
      'version': Var('dawn_go_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'checkout_linux and non_git_source',
  },
  'tools/golang/mac-amd64': {
    'packages': [{
      'package': 'infra/3pp/tools/go/mac-amd64',
      'version': Var('dawn_go_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'checkout_mac and non_git_source',
  },
  'tools/golang/mac-arm64': {
    'packages': [{
      'package': 'infra/3pp/tools/go/mac-arm64',
      'version': Var('dawn_go_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'checkout_mac and non_git_source',
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
    'url': '{chromium_git}/chromium/src/third_party/protobuf@04bb12d7ca5e16c58b55012acb32f07c89dd4e2b',
    'condition': 'dawn_standalone',
  },

  'tools/protoc_wrapper': {
    'url': '{chromium_git}/chromium/src/tools/protoc_wrapper@3438d4183bfc7c0d6850e8b970204cc8189f0323',
    'condition': 'dawn_standalone',
  },

  'third_party/libprotobuf-mutator/src': {
    'url': '{chromium_git}/external/github.com/google/libprotobuf-mutator.git@7bf98f78a30b067e22420ff699348f084f802e12',
    'condition': 'dawn_standalone',
  },

  # Dependencies for tintd.
  'third_party/jsoncpp': {
    'url': '{chromium_git}/external/github.com/open-source-parsers/jsoncpp.git@42e892d96e47b1f6e29844cc705e148ec4856448',
    'condition': 'dawn_tintd',
  },

  'third_party/langsvr': {
    'url': '{github_git}/google/langsvr.git@303c526231a90049a3e384549720f3fbd453cf66',
    'condition': 'dawn_tintd',
  },

  # Dependencies for PartitionAlloc.
  # Doc: https://docs.google.com/document/d/1wz45t0alQthsIU9P7_rQcfQyqnrBMXzrOjSzdQo-V-A
  'third_party/partition_alloc': {
    'url': '{chromium_git}/chromium/src/base/allocator/partition_allocator.git@9e005cf3bb7d9514f416edb45363ee88c323d726',
    'condition': 'dawn_standalone',
  },

  # We never want to actually checkout Chromium, but we need a fake DEPS entry
  # in order for the Chromium -> Dawn DEPS autoroller to work. Note that this
  # only currently works due to an explicit exception in presubmit checks
  # https://source.chromium.org/chromium/chromium/tools/depot_tools/+/dac161882feaedaabcf99db47726a999e4834a13:presubmit_canned_checks.py;l=2139
  'third_party/placeholder_chromium': {
    'url': '{chromium_git}/chromium/src.git' + '@' + Var('chromium_revision'),
    'condition': 'checkout_placeholder_chromium',
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
