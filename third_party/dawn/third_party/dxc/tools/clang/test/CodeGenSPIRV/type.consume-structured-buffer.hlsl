// RUN: %dxc -T vs_6_0 -E main -fspv-reflect -fcgl  %s -spirv | FileCheck %s

// CHECK: OpExtension "SPV_GOOGLE_hlsl_functionality1"

// CHECK-DAG: OpName %type_ConsumeStructuredBuffer_v4float "type.ConsumeStructuredBuffer.v4float"
// CHECK-DAG: OpName %buffer1 "buffer1"

// CHECK-DAG: OpName %type_ACSBuffer_counter "type.ACSBuffer.counter"
// CHECK-DAG: OpName %counter_var_buffer1 "counter.var.buffer1"

// CHECK-DAG: OpName %type_ConsumeStructuredBuffer_S "type.ConsumeStructuredBuffer.S"
// CHECK-DAG: OpName %buffer2 "buffer2"

// CHECK-DAG: OpName %counter_var_buffer2 "counter.var.buffer2"

// CHECK: OpDecorateId %buffer1 CounterBuffer %counter_var_buffer1
// CHECK: OpDecorateId %buffer2 CounterBuffer %counter_var_buffer2

// CHECK: %type_ACSBuffer_counter = OpTypeStruct %int
// CHECK: %_ptr_Uniform_type_ACSBuffer_counter = OpTypePointer Uniform %type_ACSBuffer_counter

// CHECK: %type_ConsumeStructuredBuffer_v4float = OpTypeStruct %_runtimearr_v4float
// CHECK: %_ptr_Uniform_type_ConsumeStructuredBuffer_v4float = OpTypePointer Uniform %type_ConsumeStructuredBuffer_v4float

// CHECK: %type_ConsumeStructuredBuffer_S = OpTypeStruct %_runtimearr_S
// CHECK: %_ptr_Uniform_type_ConsumeStructuredBuffer_S = OpTypePointer Uniform %type_ConsumeStructuredBuffer_S

// CHECK: %counter_var_buffer1 = OpVariable %_ptr_Uniform_type_ACSBuffer_counter Uniform
// CHECK: %buffer1 = OpVariable %_ptr_Uniform_type_ConsumeStructuredBuffer_v4float Uniform
ConsumeStructuredBuffer<float4> buffer1;

struct S {
    float a;
    float3 b;
    float2x3 c;
};

// CHECK: %counter_var_buffer2 = OpVariable %_ptr_Uniform_type_ACSBuffer_counter Uniform
// CHECK: %buffer2 = OpVariable %_ptr_Uniform_type_ConsumeStructuredBuffer_S Uniform
ConsumeStructuredBuffer<S> buffer2;

float main() : A {
    return 1.0;
}
