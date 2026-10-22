#!/bin/sh

# Copyright 2023 The Khronos Group Inc.
# Copyright 2023 Valve Corporation
# Copyright 2023 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0

# Execute at repo root
cd "$(dirname $0)/../../"

# Use update_deps.py to update source dependencies from /scripts/known_good.json
scripts/update_deps.py --dir="external" --no-build

cat << EOF > .gn
buildconfig = "//build/config/BUILDCONFIG.gn"
secondary_source = "//scripts/gn/secondary/"

default_args = {
    clang_use_chrome_plugins = false
    use_custom_libcxx = false
}
EOF

# Use gclient to update toolchain dependencies from /scripts/gn/DEPS (from chromium)
cat << EOF > .gclient
solutions = [
  { "name"        : ".",
    "url"         : "https://github.com/KhronosGroup/Vulkan-Utility-Libraries",
    "deps_file"   : "scripts/gn/DEPS",
    "managed"     : False,
    "custom_deps" : {
    },
    "custom_vars": {},
  },
]
EOF
gclient sync

