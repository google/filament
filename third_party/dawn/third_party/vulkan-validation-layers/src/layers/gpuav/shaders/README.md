# GPU Assisted Validation Shaders

This directory is for holding shaders that are used for [shader instrumentation](../../docs/gpu_av_shader_instrumentation.md) inside [GPU Assisted Validation](../../docs/gpuav.md). These are turned in SPIR-V when generating code with `generate_spirv.py`　and turned into a header file in [layers/vulkan/generated](../generated/).

To regenerate the validation shader, run the following:

```bash
# generate all the shaders with glslangValidator at external/glslang/build/install/bin/glslangValidator
python3 ./scripts/generate_spirv.py

# Using own glslangValidator executable
python3 ./scripts/generate_spirv.py --glslang path/to/glslangValidator

# generate a single shader
python3 ./scripts/generate_spirv.py --shader layers/gpuav/shaders/gpu_pre_draw.vert
```

## Adding a new shader

1. Add the GLSL shader to this folder with the appropriate naming (consistent with the other files)
2. Include the generated header file
    - example: `#include "gpu_pre_draw_vert.h"` for `gpu_pre_draw.vert`
3. Add the new header file to `CMake` and `BUILD.gn` (or will fail CI build)

Special note, currently any shader file starting with "`inst_`" means the shader is "instrumented".
The script will call `glslang` with the `--no-link` flag and using `spirv-link` to append the function.