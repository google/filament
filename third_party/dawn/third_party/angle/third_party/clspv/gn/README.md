# CLSPV GN Build

The clspv official build system is CMake and as such have no GN build infra. The
files in here enable building `clspv` using the ANGLE GN build infra.

The top-level build file is `BUILD.gn` with all the auxiliary build files
located in the `gn` folder. The `clspv` build is heavily dependent on the LLVM
build. The LLVM settings and targets needed for `clspv` are captured in the
`gn/llvm` location. These utilize the LLVM experimental GN build infra [1].

## Build Instructions

The GN build in here is setup to function within the ANGLE GN build
infrastructure and as such follows the same setup as of ANGLE project. Please
refer top level ANGLE readme file.

Add the following to `args.gn` file

```
angle_enable_cl = true
```

Note: Only the `linux/x86{,_64}` and `android/arm{64}` combination of `os/cpu`
are setup for now.

## Updating the LLVM build targets

The LLVM targets required for clspv are housed in the `gn/llvm` location. In the
case of source files getting added/removed in the upstream LLVM, the relavant
target sources needs to be modified in `gn/llvm/sources/BUILD.gn`.

The LLVM targets are named in the format `clspv_<llvm/clang>_<folder path
slug with _>`.

For instance `clspv_llvm_lib_frontend_offloading` refers to the LLVM
target in `third_party/llvm/src/llvm/lib/Frontend/Offloading`. The `sources`
list in from the corresponding LLVM `BUILD.gn` file, here
`third_party/llvm/src/llvm/utils/gn/secondary/llvm/lib/Frontend/Offloading` can
be copied with prefix path "//$clspv_llvm_dir/llvm/lib/Frontend/Offloading/"
applied to it.

## References

[1]: https://github.com/llvm/llvm-project/blob/main/llvm/utils/gn/README.rst
