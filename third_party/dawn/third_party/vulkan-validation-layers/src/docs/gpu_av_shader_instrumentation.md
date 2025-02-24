# GPU-AV Shader Instrumentation

Shader instrumentation is the process of taking an existing SPIR-V file from the application and injecting additional SPIR-V instructions.
When we can't statically determine the value that will be used in a shader, GPU-AV adds logic to detect the value and if it catches an invalid instruction, it will not actually execute it.

## Expected behavior during error

When we find an error in the SPIR-V at runtime there are 3 possible behaviour the layer can take

1. Still execute the instruction as normal (chance it will crash/hang everything)
2. Don't execute the instruction (works well if there is not return value to worry about)
3. Try and call a "safe default" version of the instruction

For things such as ray tracing, we decided to go with `2` as it would have the least side effects. The main goal is to make sure we get the error message to the user.

## How Descriptor Indexing is instrumented

As an exaxmple, if the incoming shader was

```glsl
#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 1) uniform sampler2D tex[];
layout(location = 0) out vec4 uFragColor;
layout(location = 0) in flat uint index;

void main(){
   uFragColor = texture(tex[index], vec2(0, 0));
}
```

it is first ran through a custom pass (`InstBindlessCheckPass`) to inject logic to call a known function, this will look like after

```glsl
vec4 value;
if (inst_bindless_descriptor(/*...*/)) {
    value = texture(tex[index], vec2(0.0));
} else {
    value = vec4(0.0);
}
uFragColor = value;
```

The next step is to add the `inst_bindless_descriptor` function into the SPIR-V.

Currently, all these functions are found in `gpuav/shaders/instrumentation`

```glsl
bool inst_bindless_descriptor(const uint inst_num, const uvec4 stage_info, const uint desc_set,
                              const uint binding, const uint desc_index, const uint byte_offset) {
    // logic
    return no_error_found;
}
```

which is compiled with `glslang`'s `--no-link` option. This is done offline and the module is found in the generated directory.

> Note: This uses `Linkage` which is not technically valid Vulkan Shader `SPIR-V`, while debugging the output of the SPIR-V passes, some tools might complain

Now with the two modules, at runtime `GPU-AV` will call into `gpuav::spirv::Module::LinkFunction` which will match up the function arguments and create the final shader which looks like

```glsl
#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 1) uniform sampler2D tex[];

layout(location = 0) out vec4 uFragColor;
layout(location = 0) flat in uint index;

bool inst_bindless_descriptor(uint inst_num, uvec4 stage_info, uint desc_set,
                              uint binding, uint desc_index, uint byte_offset) {
    // logic
    return no_error_found;
}

void main()
{
    vec4 value;
    if (inst_bindless_descriptor(2, 42, uvec4(4, gl_FragCoord.xy, 0), 0, 1, index, 0)) {
        value = texture(tex[index], vec2(0.0));
    } else {
        value = vec4(0.0);
    }
    uFragColor = value;
}
```

## How runtime spirv-val for single instructions is instrumented (Currently in proposal status)

There are a set of `VUID-RuntimeSpirv` VUs that could be validated in `spirv-val` statically **if** it was using `OpConstant`.

Since it is likely these are variables decided at runtime, we will need to check in GPU-AV.

Luckily because these are usually bound to a single instruction, it is a lighter check

### Example - OpRayQueryInitializeKHR

An example of a VU we need to validate at GPU-AV time

> For OpRayQueryInitializeKHR instructions, the RayTmin operand must be less than or equal to the RayTmax operand

the instruction operands look like

```swift
OpRayQueryInitializeKHR %ray_query %as %flags %cull_mask %ray_origin %ray_tmin %ray_dir %ray_tmax
```

The first step will be adding logic to wrap every call of this instruction to look like

```glsl
if (inst_ray_query_initialize(/* copy of arguments */)) {
    rayQueryInitializeEXT(/* original arguments */)
}
```

From here, we will use the same `gpuav::spirv::Module::LinkFunction` flow to add the logic and link in where needed

The SPIR-V before and after adding the conditional check looks like

```swift
// before
// traceRayEXT(a, b, c)
%L1 = OpLabel
%value = OpLoad %x
OpRayQueryInitializeKHR %value %param
OpReturn


// after
// if (IsValid(a, b, c)) {
//   traceRayEXT(a, b, c)
// }

%L1 = OpLabel
%value = OpLoad %x // for simplicity, can stay hoisted out
%compare = OpSomeCompareInstruction
OpSelectionMerge %L3 None
OpBranchConditional %compare %L2 %L3

    %L2 = OpLabel
    OpRayQueryInitializeKHR %value %param
    OpBranch %L3

%L3 = OpLabel
OpReturn
```