use_relative_paths = True

gclient_gn_args_file = 'build/config/gclient_args.gni'

gclient_gn_args = [
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
  'dawn_gn_version': 'git_revision:487f8353f15456474437df32bb186187b0940b45',
  # ninja CIPD package version.
  # https://chrome-infra-packages.appspot.com/p/infra/3pp/tools/ninja
  'dawn_ninja_version': 'version:3@1.12.1.chromium.4',
  'dawn_go_version': 'version:2@1.21.3',

  'node_darwin_arm64_sha': '864780996d3be6c9aca03f371a4bd672728f0a75',
  'node_darwin_x64_sha': '85ccc2202fd4f1615a443248c01a866ae227ba78',
  'node_linux_x64_sha': '46795170ff5df9831955f163f6966abde581c8af',
  'node_win_x64_sha': '2cb36010af52bc5e2a2d1e3675c10361c80d8f8d',

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
  'reclient_version': 're_client_version:0.176.0.8c46330a-gomaip',
  # siso CIPD package version.
  'siso_version': 'git_revision:9e4e007a51fdfd51e809d2817a3d6bbd3ec3b648',

  # 'magic' text to tell depot_tools that git submodules should be accepted
  # but parity with DEPS file is expected.
  'SUBMODULE_MIGRATION': 'True',

  'fetch_cmake': False,

  # condition to allowlist deps to be synced in Cider. Allowlisting is needed
  # because not all deps are compatible with Cider. Once we migrate everything
  # to be compatible we can get rid of this allowlisting mecahnism and remove
  # this condition. Tracking bug for removing this condition: b/349365433
  'non_git_source': 'True',
}

deps = {
  'buildtools': {
    'url': '{chromium_git}/chromium/src/buildtools@bb0dbc354cf9dd386f59a4db38564a21be756cd9',
    'condition': 'dawn_standalone',
  },
  'third_party/clang-format/script': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/clang/tools/clang-format.git@911fc51fb4657b50626a915f4a7509c463e4b169',
    'condition': 'dawn_standalone',
  },
  'buildtools/linux64': {
    'packages': [{
      'package': 'gn/gn/linux-amd64',
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
    'url': '{chromium_git}/chromium/tools/depot_tools.git@28358f4020ad58b86301a723807f8254326863c1',
    'condition': 'dawn_standalone',
  },

  'third_party/libc++/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libcxx.git@621455c85cae8e246fc9380159a68f775ffe385b',
    'condition': 'dawn_standalone',
  },

  'third_party/libc++abi/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libcxxabi.git@b6f2833c99549e5725055f6e9c795ca6e226afc0',
    'condition': 'dawn_standalone',
  },

  # Required by libc++
  'third_party/llvm-libc/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libc.git@630ea5577da4c334db1df16ff9472843f4a8aad2',
    'condition': 'dawn_standalone',
  },

  # Required by //build on Linux
  'third_party/libdrm/src': {
    'url': '{chromium_git}/chromiumos/third_party/libdrm.git@ad78bb591d02162d3b90890aa4d0a238b2a37cde',
    'condition': 'dawn_standalone and host_os == "linux"',
  },

  # Dependencies required to use GN, and Clang in standalone.

  # The //build and //tools/clang deps should all be updated in
  # unison, as there are dependencies between them.
  'build': {
    'url': '{chromium_git}/chromium/src/build@c734cf94f4e1501e80663319392cfbe0ce26dbb1',
    'condition': 'dawn_standalone',
  },
  'tools/clang': {
    'url': '{chromium_git}/chromium/src/tools/clang@7ade8a8f2a759b822022560e08d49b33a2e8496d',
    'condition': 'dawn_standalone',
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
        'object_name': 'e1ace9eea7f5f8906a5de665022abb745efb47ce4931ae774b58005adaf907e9',
        'sha256sum': 'e1ace9eea7f5f8906a5de665022abb745efb47ce4931ae774b58005adaf907e9',
        'size_bytes': 96825360,
        'generation': 1714159610727506,
      },
    ],
  },
  'build/linux/debian_bullseye_arm64-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and checkout_arm64',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': 'd303cf3faf7804c9dd24c9b6b167d0345d41d7fe4bfb7d34add3ab342f6a236c',
        'sha256sum': 'd303cf3faf7804c9dd24c9b6b167d0345d41d7fe4bfb7d34add3ab342f6a236c',
        'size_bytes': 103556332,
        'generation': 1714159596952688,
      },
    ],
  },
  'build/linux/debian_bullseye_i386-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and (checkout_x86 or checkout_x64)',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': '4300851707ad38b204e7f4912950c05ad51da0251ecc4e410de9b9fb94f7decf',
        'sha256sum': '4300851707ad38b204e7f4912950c05ad51da0251ecc4e410de9b9fb94f7decf',
        'size_bytes': 116515924,
        'generation': 1714159579525878,
      },
    ],
  },
  'build/linux/debian_bullseye_mipsel-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and checkout_mips',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': 'cc3202718a58541488e79b0333ce936a32227e07228f6b3c122d99ee45f83270',
        'sha256sum': 'cc3202718a58541488e79b0333ce936a32227e07228f6b3c122d99ee45f83270',
        'size_bytes': 93412776,
        'generation': 1714159559897107,
      },
    ],
  },
  'build/linux/debian_bullseye_mips64el-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and checkout_mips64',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': 'ee94d723b36d1e643820fe7ee2a8f45b3664b4c5d3c3379ebab39e474a2c9f86',
        'sha256sum': 'ee94d723b36d1e643820fe7ee2a8f45b3664b4c5d3c3379ebab39e474a2c9f86',
        'size_bytes': 97911708,
        'generation': 1714159538956875,
      },
    ],
  },
  'build/linux/debian_bullseye_amd64-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and checkout_x64',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': '5df5be9357b425cdd70d92d4697d07e7d55d7a923f037c22dc80a78e85842d2c',
        'sha256sum': '5df5be9357b425cdd70d92d4697d07e7d55d7a923f037c22dc80a78e85842d2c',
        'size_bytes': 123084324,
        'generation': 1714159395960299,
      },
    ],
  },

  # Used for Dawn-side GN arg definitions.
  'tools/mb': {
    'url': '{chromium_git}/chromium/src/tools/mb@6c50647ee969539f9371fafdeeb38d6b2c13dc34',
    'condition': 'dawn_standalone',
  },

  # Testing, GTest and GMock
  'testing': {
    'url': '{chromium_git}/chromium/src/testing@ae9705179f821d1dbd2b0a2ba7a6582faac7f86b',
    'condition': 'dawn_standalone',
  },
  'third_party/libFuzzer/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/compiler-rt/lib/fuzzer.git' + '@' + 'e31b99917861f891308269c36a32363b120126bb',
    'condition': 'dawn_standalone',
  },
  'third_party/googletest': {
    'url': '{chromium_git}/external/github.com/google/googletest@309dab8d4bbfcef0ef428762c6fec7172749de0f',
    'condition': 'dawn_standalone',
  },
  # This is a dependency of //testing
  'third_party/catapult': {
    'url': '{chromium_git}/catapult.git@b9db9201194440dc91d7f73d4c939a8488994f60',
    'condition': 'dawn_standalone',
  },
  'third_party/google_benchmark/src': {
    'url': '{chromium_git}/external/github.com/google/benchmark.git' + '@' + '761305ec3b33abf30e08d50eb829e19a802581cc',
    'condition': 'dawn_standalone',
  },

  # Jinja2 and MarkupSafe for the code generator
  'third_party/jinja2': {
    'url': '{chromium_git}/chromium/src/third_party/jinja2@e2d024354e11cc6b041b0cff032d73f0c7e43a07',
    'condition': 'dawn_standalone',
  },
  'third_party/markupsafe': {
    'url': '{chromium_git}/chromium/src/third_party/markupsafe@0bad08bb207bbfc1d6f3bbc82b9242b0c50e5794',
    'condition': 'dawn_standalone',
  },

  # GLFW for tests and samples
  'third_party/glfw': {
    'url': '{chromium_git}/external/github.com/glfw/glfw@b35641f4a3c62aa86a0b3c983d163bc0fe36026d',
  },

  'third_party/vulkan_memory_allocator': {
    'url': '{chromium_git}/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator@cb0597213b0fcb999caa9ed08c2f88dc45eb7d50',
    'condition': 'dawn_standalone',
  },

  'third_party/angle': {
    'url': '{chromium_git}/angle/angle@3643c21d9d0c159182c4680965a95d211beb7eec',
    'condition': 'dawn_standalone',
  },

  'third_party/swiftshader': {
    'url': '{swiftshader_git}/SwiftShader@0188d92714c3bc8ccfc3caf1d44c966d98f249e3',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-deps': {
    'url': '{chromium_git}/vulkan-deps@03f51f594fdd73cce50e3614f55ef5fe8f680417',
    'condition': 'dawn_standalone',
  },

  'third_party/glslang/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/glslang@eb77189a282b90e9856fc0ed5b08361a70025bec',
    'condition': 'dawn_standalone',
  },

  'third_party/spirv-cross/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Cross@b8fcf307f1f347089e3c46eb4451d27f32ebc8d3',
    'condition': 'dawn_standalone',
  },

  'third_party/spirv-headers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Headers@c8ad050fcb29e42a2f57d9f59e97488f465c436d',
    'condition': 'dawn_standalone',
  },

  'third_party/spirv-tools/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Tools@257a227fbadf8176ea386c7d8fb9b889cbf08640',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-headers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Headers@088a00d81d1fc30ff77aacf31485871aebec7cb2',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-loader/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Loader@f946876731972cb323b021b78d1921aa9244808b',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-tools/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Tools@105d6c1fede00c3a9055e5a531ebf3d99bac406e',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-utility-libraries/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Utility-Libraries@dc6f68172430999a96a209ef4700784917dab1a2',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-validation-layers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-ValidationLayers@aa1607f891cefd0a338b65bd8f2254e2c4fbf87c',
    'condition': 'dawn_standalone',
  },

  'third_party/zlib': {
    'url': '{chromium_git}/chromium/src/third_party/zlib@209717dd69cd62f24cbacc4758261ae2dd78cfac',
    'condition': 'dawn_standalone',
  },

  'third_party/abseil-cpp': {
    'url': '{chromium_git}/chromium/src/third_party/abseil-cpp@cae4b6a3990e1431caa09c7b2ed1c76d0dfeab17',
    'condition': 'dawn_standalone',
  },

  'third_party/dxc': {
    'url': '{chromium_git}/external/github.com/microsoft/DirectXShaderCompiler@a47537f0e2680441f547817a66c3b817200ac874',
  },

  'third_party/dxheaders': {
    # The non-Windows build of DXC depends on DirectX-Headers, and at a specific commit (not ToT)
    'url': '{chromium_git}/external/github.com/microsoft/DirectX-Headers@980971e835876dc0cde415e8f9bc646e64667bf7',
    'condition': 'host_os != "win"',
  },

  'third_party/khronos/OpenGL-Registry': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/OpenGL-Registry@5bae8738b23d06968e7c3a41308568120943ae77',
  },

  'third_party/khronos/EGL-Registry': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/EGL-Registry@7dea2ed79187cd13f76183c4b9100159b9e3e071',
  },

  # WebGPU CTS - not used directly by Dawn, only transitively by Chromium.
  'third_party/webgpu-cts': {
    'url': '{chromium_git}/external/github.com/gpuweb/cts@5b477670f53e5fefcf4bd829a2952013ef9d1953',
    'condition': 'build_with_chromium',
  },

  # Dependencies required to build / run WebAssembly bindings
  'third_party/emsdk': {
    # Note: Always use an emsdk hash referring to a tagged release, just so
    # emsdk and emscripten are always in sync with an exact release.
    'url': '{chromium_git}/external/github.com/emscripten-core/emsdk.git@419021fa040428bc69ef1559b325addb8e10211f',
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
    'url': '{chromium_git}/external/github.com/gpuweb/gpuweb@a2637f7b880c2556919cdb288fe89815e0ed1c41',
    'condition': 'dawn_node',
  },

  # Upstream webgpu.h headers for testing purposes
  'third_party/webgpu-headers/src': {
    'url': '{chromium_git}/external/github.com/webgpu-native/webgpu-headers@c8b371dd2ff8a2b028fdc0206af5958521181ba8',
  },

  'tools/golang': {
    'packages': [{
      'package': 'infra/3pp/tools/go/${{platform}}',
      'version': Var('dawn_go_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'non_git_source',
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
        'package': 'infra/build/siso/${{platform}}',
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
    'url': '{chromium_git}/chromium/src/third_party/protobuf@1a4051088b71355d44591172c474304331aaddad',
    'condition': 'dawn_standalone',
  },

  'tools/protoc_wrapper': {
    'url': '{chromium_git}/chromium/src/tools/protoc_wrapper@8ad6d21544b14c7f753852328d71861b363cc512',
    'condition': 'dawn_standalone',
  },

  'third_party/libprotobuf-mutator/src': {
    'url': '{chromium_git}/external/github.com/google/libprotobuf-mutator.git@7bf98f78a30b067e22420ff699348f084f802e12',
    'condition': 'dawn_standalone',
  },

  # Dependencies for tintd.
  'third_party/jsoncpp': {
    'url': '{github_git}/open-source-parsers/jsoncpp.git@69098a18b9af0c47549d9a271c054d13ca92b006',
    'condition': 'dawn_tintd',
  },

  'third_party/langsvr': {
    'url': '{github_git}/google/langsvr.git@303c526231a90049a3e384549720f3fbd453cf66',
    'condition': 'dawn_tintd',
  },

  # Dependencies for PartitionAlloc.
  # Doc: https://docs.google.com/document/d/1wz45t0alQthsIU9P7_rQcfQyqnrBMXzrOjSzdQo-V-A
  'third_party/partition_alloc': {
    'url': '{chromium_git}/chromium/src/base/allocator/partition_allocator.git@2041003ba674f918c33b1afaaad74e652f34bcea',
    'condition': 'dawn_standalone',
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
        'python3',
        'third_party/depot_tools/update_depot_tools_toggle.py',
        '--disable',
    ],
  },

  # Pull the compilers and system libraries for hermetic builds
  {
    'name': 'sysroot_x86',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_linux and (checkout_x86 or checkout_x64)',
    'action': ['python3', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x86'],
  },
  {
    'name': 'sysroot_x64',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_linux and checkout_x64',
    'action': ['python3', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x64'],
  },
  {
    # Update the Mac toolchain if possible, this makes builders use "hermetic XCode" which is
    # is more consistent (only changes when rolling build/) and is cached.
    'name': 'mac_toolchain',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_mac',
    'action': ['python3', 'build/mac_toolchain.py'],
  },
  {
    # Case-insensitivity for the Win SDK. Must run before win_toolchain below.
    'name': 'ciopfs_linux',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_win and host_os == "linux"',
    'action': [ 'python3',
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
    'action': ['python3', 'build/vs_toolchain.py', 'update', '--force'],
  },
  {
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    'action': ['python3', 'tools/clang/scripts/update.py',
               '--package=clang'],
    'condition': 'dawn_standalone',
  },
  {
    # This is also supposed to support the same set of platforms as 'clang'
    # above. LLVM ToT support isn't provided at the moment.
    'name': 'clang_tidy',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_clang_tidy',
    'action': ['python3', 'tools/clang/scripts/update.py',
               '--package=clang-tidy'],
  },
  {
    'name': 'objdump',
    'pattern': '.',
    'action': ['python3', 'tools/clang/scripts/update.py',
               '--package=objdump'],
    'condition': 'dawn_standalone',
  },
  # Pull dsymutil binaries using checked-in hashes.
  {
    'name': 'dsymutil_mac_arm64',
    'pattern': '.',
    'condition': 'dawn_standalone and host_os == "mac" and host_cpu == "arm64"',
    'action': [ 'python3',
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
    'action': [ 'python3',
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
    'action': [ 'python3',
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
    'action': [ 'python3',
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
    'action': [ 'python3',
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
    'action': ['python3', 'build/util/lastchange.py',
               '-o', 'build/util/LASTCHANGE'],
  },

  # Node binaries, when dawn_node or dawn_wasm is enabled
  {
    'name': 'node_linux',
    'pattern': '.',
    'condition': '(dawn_node or dawn_wasm) and host_os == "linux"',
    'action': [ 'python3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--extract',
                '--bucket', 'chromium-nodejs/20.11.0',
                Var('node_linux_x64_sha'),
                '-o', 'third_party/node/node-linux-x64.tar.gz',
    ],
  },
  {
    'name': 'node_mac_x64',
    'pattern': '.',
    'condition': '(dawn_node or dawn_wasm) and host_os == "mac" and host_cpu == "x64"',
    'action': [ 'python3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--extract',
                '--bucket', 'chromium-nodejs/20.11.0',
                Var('node_darwin_x64_sha'),
                '-o', 'third_party/node/node-darwin-x64.tar.gz',
    ],
  },
  {
    'name': 'node_mac_arm64',
    'pattern': '.',
    'condition': '(dawn_node or dawn_wasm) and host_os == "mac" and host_cpu == "arm64"',
    'action': [ 'python3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--extract',
                '--bucket', 'chromium-nodejs/20.11.0',
                Var('node_darwin_arm64_sha'),
                '-o', 'third_party/node/node-darwin-arm64.tar.gz',
    ],
  },
  {
    'name': 'node_win',
    'pattern': '.',
    'condition': '(dawn_node or dawn_wasm) and host_os == "win"',
    'action': [ 'python3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--bucket', 'chromium-nodejs/20.11.0',
                Var('node_win_x64_sha'),
                '-o', 'third_party/node/node.exe',
    ],
  },

  # Activate emsdk for WebAssembly builds
  {
    'name': 'activate_emsdk',
    'pattern': '.',
    'condition': 'dawn_wasm',
    'action': [ 'python3', 'tools/activate-emsdk' ],
  },

  # Configure remote exec cfg files
  {
    # Use luci_auth if on windows and using chrome-untrusted project
    'name': 'download_and_configure_reclient_cfgs',
    'pattern': '.',
    'condition': 'dawn_standalone and download_remoteexec_cfg and host_os == "win"',
    'action': ['python3',
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
    'action': ['python3',
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
    'action': ['python3',
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
    'action': ['python3',
               'build/config/siso/configure_siso.py',
               '--rbe_instance',
               Var('rbe_instance'),
               ],
  },
]

recursedeps = [
  'buildtools',
]
