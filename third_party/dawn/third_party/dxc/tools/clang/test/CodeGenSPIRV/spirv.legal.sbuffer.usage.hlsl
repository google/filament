// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    float4 f;
    float  v[128];
};

// Signature for returnRWSBuffer(). Both the function parameter and return gain
// an extra level of pointer.
// CHECK: [[retRWSBSig:%[0-9]+]] = OpTypeFunction %_ptr_Uniform_type_RWStructuredBuffer_S %_ptr_Function__ptr_Uniform_type_RWStructuredBuffer_S
RWStructuredBuffer<S> returnRWSBuffer(RWStructuredBuffer<S> paramRWSBuffer);

float4 useAsStaticRWSBuffer();

// CHECK:  %globalRWSBuffer = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_S Uniform
       RWStructuredBuffer<S> globalRWSBuffer;
// Static global variables gain an extra level of pointer.
// CHECK: %staticgRWSBuffer = OpVariable %_ptr_Private__ptr_Uniform_type_RWStructuredBuffer_S Private
static RWStructuredBuffer<S> staticgRWSBuffer = globalRWSBuffer;

// CHECK: %globalv4f32RWSBuffer = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_v4float Uniform
  RWStructuredBuffer<float4> globalv4f32RWSBuffer;

// Static local variables gain an extra level of pointer.
// CHECK: %staticRWSBuffer = OpVariable %_ptr_Private__ptr_Uniform_type_RWStructuredBuffer_S Private

// CHECK: %src_main = OpFunction
float4 main(in float4 pos : SV_Position) : SV_Target
{
// Local variables gain an extra level of pointer.
// CHECK:           %localRWSBuffer = OpVariable %_ptr_Function__ptr_Uniform_type_RWStructuredBuffer_S Function

// Temporary variables for function calls gain an extra level of pointer.
// CHECK: %param_var_paramRWSBuffer = OpVariable %_ptr_Function__ptr_Uniform_type_RWStructuredBuffer_S Function

// CHECK: %localv4f32RWSBuffer = OpVariable %_ptr_Function__ptr_Uniform_type_RWStructuredBuffer_v4float Function

// CHECK:      [[ptr1:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWStructuredBuffer_S %staticgRWSBuffer
// CHECK-NEXT: [[ptr2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float [[ptr1]] %int_0 %uint_0 %int_0
// CHECK-NEXT:      {{%[0-9]+}} = OpLoad %v4float [[ptr2]]
    float4 val = staticgRWSBuffer[0].f + useAsStaticRWSBuffer();

// Directly storing the pointer to %param_var_paramRWSBuffer
// CHECK:      OpStore %param_var_paramRWSBuffer %globalRWSBuffer
// Function calls have matching signatures.
// CHECK-NEXT: [[ptr:%[0-9]+]] = OpFunctionCall %_ptr_Uniform_type_RWStructuredBuffer_S %returnRWSBuffer %param_var_paramRWSBuffer
// CHECK-NEXT: OpStore %localRWSBuffer [[ptr]]
    RWStructuredBuffer<S> localRWSBuffer = returnRWSBuffer(globalRWSBuffer);

// CHECK:      [[ptr_0:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWStructuredBuffer_S %staticgRWSBuffer
// CHECK-NEXT: OpStore %localRWSBuffer [[ptr_0]]
    localRWSBuffer = staticgRWSBuffer;

// CHECK:      {{%[0-9]+}} = OpAccessChain %_ptr_Uniform_v4float %globalRWSBuffer %int_0 %uint_1 %int_0
    globalRWSBuffer[1].f = 4.2;

// CHECK:      OpStore %localv4f32RWSBuffer %globalv4f32RWSBuffer
    RWStructuredBuffer<float4> localv4f32RWSBuffer = globalv4f32RWSBuffer;

    return val +
// CHECK:      [[ptr1_0:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWStructuredBuffer_S %localRWSBuffer
// CHECK-NEXT: [[ptr2_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float [[ptr1_0]] %int_0 %uint_2 %int_0
// CHECK-NEXT:      {{%[0-9]+}} = OpLoad %v4float [[ptr2_0]]
        localRWSBuffer[2].f +
// CHECK:      [[ptr1_1:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWStructuredBuffer_S %localRWSBuffer
// CHECK-NEXT: [[base:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr_float_uint_128 [[ptr1_1]] %int_0 %uint_33 %int_1
// CHECK-NEXT: [[ptr2_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float [[base]] %int_44
// CHECK-NEXT:      {{%[0-9]+}} = OpLoad %float [[ptr2_1]]
        localRWSBuffer[33].v[44] +
// CHECK:      [[ptr1_2:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWStructuredBuffer_v4float %localv4f32RWSBuffer
// CHECK-NEXT: [[ptr2_2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float [[ptr1_2]] %int_0 %uint_3
// CHECK-NEXT:      {{%[0-9]+}} = OpLoad %v4float [[ptr2_2]]
        localv4f32RWSBuffer[3];
}

// CHECK: %useAsStaticRWSBuffer = OpFunction
float4 useAsStaticRWSBuffer() {
// Directly storing the pointer to %staticRWSBuffer
// CHECK: OpStore %staticRWSBuffer %globalRWSBuffer
    static RWStructuredBuffer<S> staticRWSBuffer = globalRWSBuffer;
    staticRWSBuffer[0].f = 30;
    return staticRWSBuffer[0].f;
}

// CHECK: %returnRWSBuffer = OpFunction %_ptr_Uniform_type_RWStructuredBuffer_S None [[retRWSBSig]]
// Function parameters gain an extra level of pointer.
// CHECK:  %paramRWSBuffer = OpFunctionParameter %_ptr_Function__ptr_Uniform_type_RWStructuredBuffer_S
RWStructuredBuffer<S> returnRWSBuffer(RWStructuredBuffer<S> paramRWSBuffer) {
// CHECK:     [[ptr_1:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWStructuredBuffer_S %paramRWSBuffer
// CHECK-NEXT:               OpReturnValue [[ptr_1]]
    return paramRWSBuffer;
}
