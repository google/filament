// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

struct S {
    [[vk::binding(5)]] // error
    float4 f;
    [[vk::counter_binding(11)]] // error
    float4 g;
};

[[vk::counter_binding(3)]] // error
SamplerState mySampler;
[[vk::counter_binding(4)]] // error
Texture2D    myTexture;
[[vk::counter_binding(5)]] // error
StructuredBuffer<S> mySBuffer;

[[vk::location(5)]] // error
ConsumeStructuredBuffer<S> myCSBuffer;

[[vk::location(12)]] // error
cbuffer myCBuffer {
    float field;
}

[[vk::binding(10)]] // error
float4 main([[vk::binding(15)]] float4 a: A // error
           ) : SV_Target {
 return 1.0;
}

struct T {
    [[vk::push_constant]] // error
    float4 f;
};

[[vk::push_constant]] // error
float foo([[vk::push_constant]] int param) // error
{
    return param;
}

[[vk::push_constant(5)]]
T pcs;

struct SRB_NV {
    [[vk::shader_record_nv]] // error
    float4 f;
};

[[vk::shader_record_nv]] // error
float foosrbnv([[vk::shader_record_nv]] int param) // error
{
    return param;
}

[[vk::shader_record_nv(5)]]
SRB_NV recordBufNV;

struct SRB_EXT {
    [[vk::shader_record_ext]] // error
    float4 f;
};

[[vk::shader_record_ext]] // error
float foosrbext([[vk::shader_record_ext]] int param)
{
    return param;
}

[[vk::shader_record_ext(5)]]
SRB_EXT recordBufEXT;

// CHECK:   :4:7: error: 'binding' attribute only applies to global variables, cbuffers, and tbuffers
// CHECK:   :6:7: error: 'counter_binding' attribute only applies to RWStructuredBuffers, AppendStructuredBuffers, and ConsumeStructuredBuffers
// CHECK:  :10:3: error: 'counter_binding' attribute only applies to RWStructuredBuffers, AppendStructuredBuffers, and ConsumeStructuredBuffers
// CHECK:  :12:3: error: 'counter_binding' attribute only applies to RWStructuredBuffers, AppendStructuredBuffers, and ConsumeStructuredBuffers
// CHECK:  :14:3: error: 'counter_binding' attribute only applies to RWStructuredBuffers, AppendStructuredBuffers, and ConsumeStructuredBuffers
// CHECK:  :17:3: error: 'location' attribute only applies to functions, parameters, and fields
// CHECK:  :20:3: error: 'location' attribute only applies to functions, parameters, and fields
// CHECK: :26:15: error: 'binding' attribute only applies to global variables, cbuffers, and tbuffers
// CHECK:  :25:3: error: 'binding' attribute only applies to global variables, cbuffers, and tbuffers
// CHECK:  :32:7: error: 'push_constant' attribute only applies to global variables of struct type
// CHECK: :37:13: error: 'push_constant' attribute only applies to global variables of struct type
// CHECK:  :36:3: error: 'push_constant' attribute only applies to global variables of struct type
// CHECK:  :42:3: error: 'push_constant' attribute takes no arguments
// CHECK:  :46:7: error: 'shader_record_nv' attribute only applies to cbuffer or ConstantBuffer
// CHECK: :51:18: error: 'shader_record_nv' attribute only applies to cbuffer or ConstantBuffer
// CHECK:  :50:3: error: 'shader_record_nv' attribute only applies to cbuffer or ConstantBuffer
// CHECK:  :56:3: error: 'shader_record_nv' attribute takes no arguments
// CHECK:  :60:7: error: 'shader_record_ext' attribute only applies to cbuffer or ConstantBuffer
// CHECK: :65:19: error: 'shader_record_ext' attribute only applies to cbuffer or ConstantBuffer
// CHECK:  :64:3: error: 'shader_record_ext' attribute only applies to cbuffer or ConstantBuffer
// CHECK:  :70:3: error: 'shader_record_ext' attribute takes no arguments
