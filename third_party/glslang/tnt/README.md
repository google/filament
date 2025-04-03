To update glslang use the following steps:

- Download a ZIP of the [glslang repository](https://github.com/KhronosGroup/glslang) into `third_party/glslang`.
- Run the following commands (on Mac) to unzip and copy the new source files:

```
cd third_party
mkdir glslang_copy && cd glslang_copy
git init
git fetch --depth=1 https://github.com/KhronosGroup/glslang.git e57f993cff981c8c3ffd38967e030f04d13781a9
git reset --hard FETCH_HEAD
find . -name .git -type d -print0 | xargs -0 rm -r
rm -rf Test
cp -r ../glslang/tnt .
cd ..
rm -rf glslang
mv glslang_copy glslang
git checkout main glslang/tnt glslang/glslang/tnt glslang/SPIRV/tnt
git restore glslang/LICENSE
git add glslang
```

- If necessary, add or remove source files from `glslang/glslang/tnt/CMakeLists.txt` et al.
- If necessary, update the `DefaultTBuiltInResource` definition inside `libs/filamat/src/sca/builtinResource.h` to glslang's located at
`third_party/glslang/StandAlone/ResourceLimits.cpp`
- Compile and test `matc`
