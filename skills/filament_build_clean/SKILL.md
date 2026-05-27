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
