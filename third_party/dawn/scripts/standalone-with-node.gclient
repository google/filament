# Copy this file to <dawn clone dir>/.gclient to bootstrap gclient in a
# standalone checkout of Dawn that also compiles dawn_node.

solutions = [
  { "name"        : ".",
    "url"         : "https://dawn.googlesource.com/dawn",
    "deps_file"   : "DEPS",
    "managed"     : False,
    "custom_vars" : {
      "dawn_node" : True,
    }
  },
]
