To update the version of glslang used in this project, run the `update_glslang.sh` script.

The script is located in `third_party/glslang/tnt`.

From the root of the repository, you can run it like this:

**Usage:**
```shell
./third_party/glslang/tnt/update_glslang.sh <version>
```

For example, to update to version 15.4.0:
```shell
./third_party/glslang/tnt/update_glslang.sh 15.4.0
```

- If necessary, add or remove source files from `glslang/glslang/tnt/CMakeLists.txt` et al.
- If necessary, update the `DefaultTBuiltInResource` definition inside `libs/filamat/src/sca/builtinResource.h` to glslang's located at
`third_party/glslang/StandAlone/ResourceLimits.cpp`
- Compile and test `matc`

- You can find the latest version number on the glslang releases page:
[https://github.com/KhronosGroup/glslang/releases](https://github.com/KhronosGroup/glslang/releases)
