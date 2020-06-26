# Revision history for `glslang`

All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](https://semver.org/).

## 10.15.3847-dev 2020-06-16

### Breaking changes

* The following files have been removed:
  * `glslang/include/revision.h`
  * `glslang/include/revision.template`

The `GLSLANG_MINOR_VERSION` and `GLSLANG_PATCH_LEVEL` defines have been removed
from the public headers. \
Instead each build script now uses the new `build_info.py`
script along with the `build_info.h.tmpl` and this `CHANGES.md` file to generate
the glslang build-time generated header `glslang/build_info.h`.

The new public API to obtain the `glslang` version is `glslang::GetVersion()`.

### Other changes
* `glslang` shared objects produced by CMake are now `SONAME` versioned using
   [Semantic Versioning 2.0.0](https://semver.org/).
