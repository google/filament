# Copy this file to <dawn clone dir>/.gclient to bootstrap gclient in a
# standalone checkout of Dawn for maximal target support including
# building cross compilation, emscripten builds, and node builds. After
# copying this file, users should uncomment the respective lines below
# based on the host OS.
#
# Use this instead of `standalone.gclient` if you are a Dawn developer and
# would like to compile as much as possible on your local machine.

solutions = [
    {
        "name": ".",
        "url": "https://dawn.googlesource.com/dawn",
        "deps_file": "DEPS",
        "managed": False,
        "custom_vars": {
            #"download_remoteexec_cfg": True,  # for Siso
            "checkout_clang_tidy": True,
            "checkout_clangd": True,
            "dawn_node": True,
            "dawn_wasm": True,
        }
    },
]

# Enable all additional targets that are usable on this host OS.
target_os = []
# If cross compiling on Mac, uncomment this:
#target_os += ['win']
# If cross compiling on Linux, uncomment this:
#target_os += ['win', 'mac', 'android']
