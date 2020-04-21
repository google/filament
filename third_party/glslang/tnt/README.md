To update glslang use the following steps:

- Download a ZIP of the [glslang repository](https://github.com/KhronosGroup/glslang) into `third_party/glslang`.
- Run the following commands (on Mac) to unzip and copy the new source files:

```
cd third_party/glslang
unzip glslang-master.zip
rsync --remove-source-files -ra glslang-master/ ./
rm -r glslang-master.zip glslang-master
```

The `Test` folder can be removed, as it is not needed:

```
rm -rf glslang/Test/
```

- If necessary, update the `DefaultTBuiltInResource` definition inside `libs/filamat/src/sca/builtinResource.h` to glslang's located at
`third_party/glslang/StandAlone/ResourceLimits.cpp`
- Compile and test `matc`
