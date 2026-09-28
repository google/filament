---
name: filament-build-clean
description: >
  Clean and build Filament targets for development, debugging, and production.
  Use this skill whenever you need to clean the workspace or compile desktop targets.
---

# Filament Build & Clean Protocols

This skill defines standard procedures and command configurations for cleaning build artifacts and compiling Filament targets on desktop platforms.

## 1. Full Clean of All Builds

To completely reset the build environment and remove all compiled artifacts, caches, and generated files across all platforms:

```bash
./build.sh -C
```

## 2. Compiling Desktop Debug Build

For active development, local debugging, and interactive testing, compile the desktop target with debug symbols and no optimizations:

```bash
./build.sh -ip desktop debug
```

## 3. Compiling Desktop Release Build

For performance analysis, optimizations, profiling, and running local benchmarks on desktop systems, compile the release build:

```bash
./build.sh -ip desktop release
```

## 4. Incremental Builds with Ninja

For fast iteration you may invoke `ninja` directly against an existing build directory:

```bash
ninja -C out/cmake-debug <target>
```

**Beware of the second build directory.** Materials are *not* compiled by
`out/cmake-debug/tools/matc/matc`. Every material rule in `build.ninja` invokes the split-build
copy of the host tools at `out/prebuilt-tools-release/tools/matc/matc`. See
`build_tools_for_split_build` in `build.sh`; note that `SPLIT_BUILD_TYPE` defaults to `release`,
so this path is `prebuilt-tools-release` even for a debug build.

As a result, after changing anything that feeds shader generation — `libs/filamat/`,
`libs/filabridge/`, `shaders/`, or `third_party/spirv-cross/` — a plain
`ninja -C out/cmake-debug` will rebuild and relink binaries while silently reusing **stale
`.filamat` files**. This produces confusing runtime shader compilation errors in which freshly
generated code is mixed with previously generated code.

Rebuild the host tools first:

```bash
ninja -C out/prebuilt-tools-release matc resgen
ninja -C out/cmake-debug <target>
```

`./build.sh -ip desktop debug` performs both steps automatically, so prefer it when in doubt.
