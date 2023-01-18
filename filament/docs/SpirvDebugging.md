# Investigating SPIRV-Cross / SPIRV-Tools issues

There are 4 repositories at play here:
- [KhronosGroup/glslang](https://github.com/KhronosGroup/glslang)
- [KhronosGroup/spirv-tools](https://github.com/KhronosGroup/SPIRV-Tools)
- [KhronosGroup/spirv-cross](https://github.com/KhronosGroup/SPIRV-Cross)
- [KhronosGroup/SPIRV-Headers](https://github.com/KhronosGroup/SPIRV-Headers)

Typically, the bug is present either in spirv-tools or spirv-cross.

## Build and install command-line tools on PATH

The goal is to replicate the bug outside of Filament, so we're going to use command-line versions of
the SPIRV tools.

### Clone and build each repo

Note: Filament checks-out versions of these repositories inside `third_party/`; however, I've found
it easiser to check out fresh copies separately so I can simply `git pull` to get the latest
versions. Furthermore, Filament has modified some of these repositories locally for its own use
case. Checking them out separately "proves" that the issue isn't Filament-specific.

```
git clone git@github.com:KhronosGroup/SPIRV-Tools.git
git clone git@github.com:KhronosGroup/SPIRV-Cross.git
git clone git@github.com:KhronosGroup/glslang.git
git clone git@github.com:KhronosGroup/SPIRV-Headers.git SPIRV-Tools/external/SPIRV-Headers

cd SPIRV-Tools/
mkdir build && cmake . -G Ninja -B build
ninja -C build
cd ..

cd SPIRV-Cross/
mkdir build && cmake . -G Ninja -B build
ninja -C build
cd ..

cd glslang/
mkdir build && cmake . -G Ninja -B build
ninja -C build
cd ..
```

### Add directories to PATH

```
export PATH=`pwd`/SPIRV-Tools/build/tools:$PATH
export PATH=`pwd`/glslang/build/StandAlone:$PATH
export PATH=`pwd`/spirv-cross/build:$PATH
```

Ensure the following tools now exist on your PATH:

1. `glslangValidator`
2. `spiv-opt`
3. `spirv-val`
4. `spirv-cross`

## Isolate the problematic GLSL shader

First determine the Filament material and variant that causes the problem.

What we want is the "raw" GLSL version of the shader, before any optimizations / cross-compilation
happens.

The easiest way I've found to do this is by adding a `std::cout` statement inside `MaterialBuilder`,
right after `sg.createFragmentProgram` or `sg.createVertexProgram` is called. For example,

```
if (v.variant.key == 8u) { std::cout << shader << std::endl; }
```

Then, I invoke `matc` and redirect stdout to a .frag or .vert file.

Note that gltfio material "templates" first go through a build step. After building gltfio, the
gltfio Filament materials are output to:

```
out/cmake-release/libs/gltfio/*.mat
```

One of these materials can be compiled with the following command:

```
matc \
    -TCUSTOM_PARAMS="// no custom params" \
    -TCUSTOM_VERTEX="// no custom vertex" \
    -TCUSTOM_FRAGMENT="// no custom fragment" \
    -TDOUBLESIDED=false \
    -TTRANSPARENCY=default \
    -TSHADINGMODEL=unlit \
    -TBLENDING=opaque \
    --platform mobile --api metal -o temp.filamat \
    unlit_opaque.mat
```

## Reproduce the compilation error

The goal is to generate a .spv file that doesn't pass validation (through the spirv-val tool).

Reproducing the error usually involves a few steps:

1. Compile the raw GLSL shader into SPIR-V.

```
glslangValidator -V -o unoptimized.spv in.frag
```

2. Optimize for performance.

```
spirv-opt -Oconfig=optimizations.cfg unoptimized.spv -o optimized.spv
```

See [optimizations.cfg](optimizations.cfg) for a template. This file should contain the same list of optimizations that
Filament employs. This should match the same optimizations specified in `GLSLPostProcessor`, for
example, `GLSLPostProcessor::registerPerformancePasses` or `GLSLPostProcessor::registerSizePasses`.

3. For shaders targeting Metal, convert relaxed ops to half.

```
spirv-opt \
    --convert-relaxed-to-half \
    --simplify-instructions \
    --redundancy-elimination \
    --eliminate-dead-code-aggressive \
    optimized.spv \
    -o half.spv
```

4. Finally, validate the final SPIR-V.

```
spirv-val half.spv
```

5. Sometimes validation will still pass, but still generate invalid shaders after cross-compiling.
   In these cases, you'll need to cross compile to the target language and manually pick out errors
   in the generated shader.

```
# for OpenGL
spirv-cross optimized.spv > optimized.frag

# for OpenGL ES
spirv-cross --es optimized.spv > optimized.frag

# for MSL
spirv-cross --msl optimized.spv > optimized.metal
```

To invoke Apple's compiler to compile MSL, you can run:

```
xcrun -sdk macosx metal -c optimized.metal -o /dev/null
```

## Clean up the shader for a bug report

These commands will run the preprocessor only on `in.frag`, and remove any empty lines.

```
glslangValidator -E in.frag > preprocessed.frag
sed '/^$/d' preprocessed.frag > preprocessed_small.frag
```

You can also run `clang-format` on the preprocessed shader to make it easier to read:

```
clang-format -i preprocessed_small.frag
```

I always try to "whittle down" the shader to a smaller version that still reproduces the error. This
might make it a bit easier on the Khronos team to diagnose the issue. I typically follow these steps
in a loop until I'm satisfied:

1. Delete an unnecessary part of the shader
2. Run the steps to reproduce the error
3. If the error still reproduces, repeat
4. Otherwise, undo the change and make a smaller change

There's also a [Reducer](https://github.com/KhronosGroup/SPIRV-Tools#reducer) tool that's part of
SPIRV-Tools which can be used to automate these steps. I haven't experimented much with this, but it
seems promising.

## Submit an Issue with the relevant Khronos repository

See some example issues that have been filed in the past:

- https://github.com/KhronosGroup/SPIRV-Cross/issues/1935
- https://github.com/KhronosGroup/SPIRV-Cross/issues/1088
- https://github.com/KhronosGroup/SPIRV-Cross/issues/1026
- https://github.com/KhronosGroup/SPIRV-Tools/issues/4452
- https://github.com/KhronosGroup/SPIRV-Tools/issues/3406
- https://github.com/KhronosGroup/SPIRV-Tools/issues/3099
- https://github.com/KhronosGroup/SPIRV-Tools/issues/5044

