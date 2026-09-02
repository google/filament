// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: %type_MyCBuffer = OpTypeStruct %v4float
cbuffer MyCBuffer {
    float4 CB_val;
}

// CHECK: %type_ConstantBuffer_S = OpTypeStruct %v4float
// CHECK: %S = OpTypeStruct %v4float
struct S {
    float4 val;
};
ConstantBuffer<S>          MyConstantBuffer;

// CHECK: %type_AppendStructuredBuffer_S = OpTypeStruct %_runtimearr_S
AppendStructuredBuffer<S>  MyASBuffer;
// CHECK: %type_ConsumeStructuredBuffer_S = OpTypeStruct %_runtimearr_S
ConsumeStructuredBuffer<S> MyCSBuffer;

// CHECK: %type__Globals = OpTypeStruct %v4float

// CHECK: %Empty1 = OpTypeStruct
// CHECK: %Empty2 = OpTypeStruct
struct Empty1 {};
struct Empty2 {};

// CHECK: %OneField1 = OpTypeStruct %v4float
// CHECK: %OneField2 = OpTypeStruct %v4float
struct OneField1 {
    float4 val;
};
struct OneField2 {
    float4 val;
};

float4 val; // Compose $Globals



void main() {
    Empty1 e1;
    Empty2 e2;
    OneField1 of1;
    OneField2 of2;
}
