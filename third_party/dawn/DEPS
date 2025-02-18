use_relative_paths = True

gclient_gn_args_file = 'build/config/gclient_args.gni'

gclient_gn_args = [
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
  'dawn_cmake_version': 'version:2@3.23.3',
  'dawn_cmake_win32_sha1': 'b106d66bcdc8a71ea2cdf5446091327bfdb1bcd7',
  'dawn_gn_version': 'git_revision:182a6eb05d15cc76d2302f7928fdb4f645d52c53',
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

  # Fetch the rust toolchain.
  #
  # Use a custom_vars section to enable it:
  # "custom_vars": {
  #   "checkout_rust": True,
  # }
  'checkout_rust': False,

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
  'reclient_version': 're_client_version:0.143.0.518e369-gomaip',
  # siso CIPD package version.
  'siso_version': 'git_revision:0b6159c39573013a79a47f8544200df07a6a74fc',

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
    'url': '{chromium_git}/chromium/src/buildtools@a660247d3c14a172b74b8e832ba1066b30183c97',
    'condition': 'dawn_standalone',
  },
  'third_party/clang-format/script': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/clang/tools/clang-format.git@95c834f3753e65ce6daa74e345c879566c1491d0',
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
    'url': '{chromium_git}/chromium/tools/depot_tools.git@77a800d7294d29ded899af9685c65babb3a62821',
    'condition': 'dawn_standalone',
  },

  'third_party/libc++/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libcxx.git@2e25154d49c29fa9aa42c30ad4a027bd30123434',
    'condition': 'dawn_standalone',
  },

  'third_party/libc++abi/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libcxxabi.git@8205ccf0f23545ebcd8846363ea1d29e77917a22',
    'condition': 'dawn_standalone',
  },

  # Required by libc++
  'third_party/llvm-libc/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libc.git@a485ddbbb2ffe528c3ebf82b9d72a7297916531f',
    'condition': 'dawn_standalone',
  },

  # Required by //build on Linux
  'third_party/libdrm/src': {
    'url': '{chromium_git}/chromiumos/third_party/libdrm.git@ad78bb591d02162d3b90890aa4d0a238b2a37cde',
    'condition': 'dawn_standalone and host_os == "linux"',
  },

  # Dependencies required to use GN, Clang, and Rust in standalone.
  # The //build, //tools/clang, and //tools/rust deps should all be updated
  # in unison, as there are dependencies between them.
  'build': {
    'url': '{chromium_git}/chromium/src/build@a252ef1991b42918f6e74bc8c26b6543afe7bb2e',
    'condition': 'dawn_standalone',
  },
  'tools/clang': {
    'url': '{chromium_git}/chromium/src/tools/clang@e262f0f8896e459fe7fd2a076af48d5746b1d332',
    'condition': 'dawn_standalone',
  },
  'tools/rust': {
    'url': '{chromium_git}/chromium/src/tools/rust@05dbbfdcec4a7cc6c7cbf735d5a67d331a2f40da',
    'condition': 'dawn_standalone and checkout_rust',
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


  # Testing, GTest and GMock
  'testing': {
    'url': '{chromium_git}/chromium/src/testing@1bd0da6657e330cf26ed0702b3f456393587ad7c',
    'condition': 'dawn_standalone',
  },
  'third_party/libFuzzer/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/compiler-rt/lib/fuzzer.git' + '@' + '26cc39e59b2bf5cbc20486296248a842c536878d',
    'condition': 'dawn_standalone',
  },
  'third_party/googletest': {
    'url': '{chromium_git}/external/github.com/google/googletest@7a7231c442484be389fdf01594310349ca0e42a8',
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
    'url': '{chromium_git}/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator@52dc220fb326e6ae132b7f262133b37b0dc334a3',
    'condition': 'dawn_standalone',
  },

  'third_party/angle': {
    'url': '{chromium_git}/angle/angle@38aa0780bb362a5f84518aa951d36dc0e05e101c',
    'condition': 'dawn_standalone',
  },

  'third_party/swiftshader': {
    'url': '{swiftshader_git}/SwiftShader@5017ce3c800e1410da6a3257098a8370591de5fa',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-deps': {
    'url': '{chromium_git}/vulkan-deps@c32804ca5a39f6e75c3a6289ae1bcca35c559a4c',
    'condition': 'dawn_standalone',
  },

  'third_party/glslang/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/glslang@8a6b381c9afbfaf9fff851582539fab15ea480c0',
    'condition': 'dawn_standalone',
  },

  'third_party/spirv-cross/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Cross@b8fcf307f1f347089e3c46eb4451d27f32ebc8d3',
    'condition': 'dawn_standalone',
  },

  'third_party/spirv-headers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Headers@09913f088a1197aba4aefd300a876b2ebbaa3391',
    'condition': 'dawn_standalone',
  },

  'third_party/spirv-tools/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Tools@682bcd51548e670811f1d03511968bb59a1157ce',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-headers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Headers@234c4b7370a8ea3239a214c9e871e4b17c89f4ab',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-loader/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Loader@fde0f9718bd60b49cf8efc80d3fb7a093c309ac0',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-tools/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Tools@a9f88a232a4cd6634e47f0f93c8fac5789387a7a',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-utility-libraries/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Utility-Libraries@fe7a09b13899c5c77d956fa310286f7a7eb2c4ed',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-validation-layers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-ValidationLayers@e00009bc3f550898c92e96a77ae4dca963778301',
    'condition': 'dawn_standalone',
  },

  'third_party/zlib': {
    'url': '{chromium_git}/chromium/src/third_party/zlib@209717dd69cd62f24cbacc4758261ae2dd78cfac',
    'condition': 'dawn_standalone',
  },

  'third_party/abseil-cpp': {
    'url': '{chromium_git}/chromium/src/third_party/abseil-cpp@f81f6c011baf9b0132a5594c034fe0060820711d',
    'condition': 'dawn_standalone',
  },

  'third_party/dxc': {
    'url': '{chromium_git}/external/github.com/microsoft/DirectXShaderCompiler@f8ef48362027241b3400b116bb8f2b55d3efabfd',
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
    'url': '{chromium_git}/external/github.com/gpuweb/cts@24d5dfa7725d6ece31941c3f3343ba6362986d6b',
    'condition': 'build_with_chromium',
  },

  # Dependencies required to build / run Dawn NodeJS bindings
  'third_party/node-api-headers': {
    'url': '{github_git}/nodejs/node-api-headers.git@d5cfe19da8b974ca35764dd1c73b91d57cd3c4ce',
    'condition': 'dawn_node',
  },
  'third_party/node-addon-api': {
    'url': '{github_git}/nodejs/node-addon-api.git@1e26dcb52829a74260ec262edb41fc22998669b6',
    'condition': 'dawn_node',
  },
  'third_party/gpuweb': {
    'url': '{github_git}/gpuweb/gpuweb.git@a118ba5f2afaad9f763fe09c3a53f96cd9106cb1',
    'condition': 'dawn_node',
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
    'dep_type': 'cipd',
    'condition': 'dawn_standalone',
  },

  # Misc dependencies inherited from Tint
  'third_party/protobuf': {
    'url': '{chromium_git}/chromium/src/third_party/protobuf@da2fe725b80ac0ba646fbf77d0ce5b4ac236f823',
    'condition': 'dawn_standalone',
  },

  'tools/protoc_wrapper': {
    'url': '{chromium_git}/chromium/src/tools/protoc_wrapper@b5ea227bd88235ab3ccda964d5f3819c4e2d8032',
    'condition': 'dawn_standalone',
  },

  'third_party/libprotobuf-mutator/src': {
    'url': '{chromium_git}/external/github.com/google/libprotobuf-mutator.git@a304ec48dcf15d942607032151f7e9ee504b5dcf',
    'condition': 'dawn_standalone',
  },

  # Dependencies for tintd.
  'third_party/jsoncpp': {
    'url': '{github_git}/open-source-parsers/jsoncpp.git@69098a18b9af0c47549d9a271c054d13ca92b006',
    'condition': 'dawn_standalone',
  },

  'third_party/langsvr': {
    'url': '{github_git}/google/langsvr.git@303c526231a90049a3e384549720f3fbd453cf66',
    'condition': 'dawn_standalone',
  },

  # Dependencies for PartitionAlloc.
  # Doc: https://docs.google.com/document/d/1wz45t0alQthsIU9P7_rQcfQyqnrBMXzrOjSzdQo-V-A
  'third_party/partition_alloc': {
    'url': '{chromium_git}/chromium/src/base/allocator/partition_allocator.git@2e6b2efb6f435aa3dd400cb3bdcead2a601f8f9a',
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
                '--no_auth',
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
    'action': ['python3', 'tools/clang/scripts/update.py'],
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
    'name': 'rust',
    'pattern': '.',
    'action': ['python3', 'tools/rust/update_rust.py'],
    'condition': 'dawn_standalone and checkout_rust',
  },
  # Pull dsymutil binaries using checked-in hashes.
  {
    'name': 'dsymutil_mac_arm64',
    'pattern': '.',
    'condition': 'dawn_standalone and host_os == "mac" and host_cpu == "arm64"',
    'action': [ 'python3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
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
                '--no_auth',
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
                '--no_auth',
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
                '--no_auth',
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
                '--no_auth',
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

  # Node binaries, when dawn_node is enabled
  {
    'name': 'node_linux64',
    'pattern': '.',
    'condition': 'dawn_node and host_os == "linux"',
    'action': [ 'python3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--extract',
                '--no_auth',
                '--bucket', 'chromium-nodejs/20.11.0',
                Var('node_linux_x64_sha'),
                '-o', 'third_party/node/node-linux-x64.tar.gz',
    ],
  },
  {
    'name': 'node_mac',
    'pattern': '.',
    'condition': 'dawn_node and host_os == "mac"',
    'action': [ 'python3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--extract',
                '--no_auth',
                '--bucket', 'chromium-nodejs/20.11.0',
                Var('node_darwin_x64_sha'),
                '-o', 'third_party/node/node-darwin-x64.tar.gz',
    ],
  },
  {
    'name': 'node_mac_arm64',
    'pattern': '.',
    'condition': 'dawn_node and host_os == "mac"',
    'action': [ 'python3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--extract',
                '--no_auth',
                '--bucket', 'chromium-nodejs/20.11.0',
                Var('node_darwin_arm64_sha'),
                '-o', 'third_party/node/node-darwin-arm64.tar.gz',
    ],
  },
  {
    'name': 'node_win',
    'pattern': '.',
    'condition': 'dawn_node and host_os == "win"',
    'action': [ 'python3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-nodejs/20.11.0',
                Var('node_win_x64_sha'),
                '-o', 'third_party/node/node.exe',
    ],
  },

  # Download remote exec cfg files
  {
    'name': 'fetch_reclient_cfgs',
    'pattern': '.',
    'condition': 'download_remoteexec_cfg and dawn_standalone',
    'action': ['python3',
               'buildtools/reclient_cfgs/fetch_reclient_cfgs.py',
               '--rbe_instance',
               Var('rbe_instance'),
               '--reproxy_cfg_template',
               'reproxy.cfg.template',
               '--rewrapper_cfg_project',
               Var('rewrapper_cfg_project'),
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
