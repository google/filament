To update glslang use the following steps:

- Download a ZIP of the [glslang repository](https://github.com/KhronosGroup/glslang) into `third_party/glslang`.
- Run the following commands (on Mac) to unzip and copy the new source files:

```
cd third_party
curl -L https://github.com/KhronosGroup/glslang/archive/8a6b381c9afbfaf9fff851582539fab15ea480c0.zip > main.zip
unzip main.zip
rsync -r glslang-8a6b381c9afbfaf9fff851582539fab15ea480c0/ glslang/ --delete
rm -r main.zip glslang-8a6b381c9afbfaf9fff851582539fab15ea480c0
rm -rf glslang/Test/
git checkout main glslang/tnt glslang/glslang/tnt glslang/SPIRV/tnt
git restore glslang/LICENSE
git add glslang
```

- If necessary, add or remove source files from `glslang/glslang/tnt/CMakeLists.txt` et al.
- If necessary, update the `DefaultTBuiltInResource` definition inside `libs/filamat/src/sca/builtinResource.h` to glslang's located at
`third_party/glslang/StandAlone/ResourceLimits.cpp`
- Compile and test `matc`
