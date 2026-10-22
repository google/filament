// RUN: %dxc -T ps_6_6 -E main -fspv-reflect -fcgl %s -spirv | FileCheck %s

// CHECK: OpExtension "SPV_GOOGLE_hlsl_functionality1"

// CHECK: OpName %type_RasterizerOrderedStructuredBuffer_S "type.RasterizerOrderedStructuredBuffer.S"

// CHECK: %S = OpTypeStruct %float %v3float %mat2v3float
// CHECK: %_runtimearr_S = OpTypeRuntimeArray %S
// CHECK: %type_RasterizerOrderedStructuredBuffer_S = OpTypeStruct %_runtimearr_S
// CHECK: %_ptr_Uniform_type_RasterizerOrderedStructuredBuffer_S = OpTypePointer Uniform %type_RasterizerOrderedStructuredBuffer_S
struct S {
    float    a;
    float3   b;
    float2x3 c;
};

// CHECK: %buf = OpVariable %_ptr_Uniform_type_RasterizerOrderedStructuredBuffer_S Uniform
RasterizerOrderedStructuredBuffer<S> buf : register(u1);

void main() { }
