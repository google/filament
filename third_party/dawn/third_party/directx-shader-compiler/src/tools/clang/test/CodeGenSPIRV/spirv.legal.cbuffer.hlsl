// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// %type_ConstantBuffer_S is the type for myCBuffer. With layout decoration.
// %S is the type for myASBuffer elements. With layout decoration.
// %S_0 is the type for function local variables. Without layout decoration.

// CHECK:     OpMemberDecorate %type_ConstantBuffer_S 0 Offset 0
// CHECK:     OpMemberDecorate %S 0 Offset 0
// CHECK-NOT: OpMemberDecorate %S_0 0 Offset 0

// CHECK:     %type_ConstantBuffer_S = OpTypeStruct %v4float
// CHECK:     %S = OpTypeStruct %v4float
// CHECK:     %S_0 = OpTypeStruct %v4float
struct S {
    float4 f;
};

// CHECK: %myCBuffer = OpVariable %_ptr_Uniform_type_ConstantBuffer_S Uniform
ConstantBuffer<S>         myCBuffer;
AppendStructuredBuffer<S> myASBuffer;

S retStuff();

float4 doStuff(S buffer) {
    return buffer.f;
}

float4 main(in float4 pos : SV_Position) : SV_Target
{
// Initializing a T with a ConstantBuffer<T> is a copy
// CHECK:      [[val:%[0-9]+]] = OpLoad %type_ConstantBuffer_S %myCBuffer
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeExtract %v4float [[val]] 0
// CHECK-NEXT: [[e0:%[0-9]+]] = OpCompositeExtract %float [[vec]] 0
// CHECK-NEXT: [[e1:%[0-9]+]] = OpCompositeExtract %float [[vec]] 1
// CHECK-NEXT: [[e2:%[0-9]+]] = OpCompositeExtract %float [[vec]] 2
// CHECK-NEXT: [[e3:%[0-9]+]] = OpCompositeExtract %float [[vec]] 3
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeConstruct %v4float  [[e0]] [[e1]] [[e2]] [[e3]]
// CHECK-NEXT: [[tmp:%[0-9]+]] = OpCompositeConstruct %S_0 [[vec]]
// CHECK-NEXT:                OpStore %buffer1 [[tmp]]
    S buffer1 = myCBuffer;

// Assigning a ConstantBuffer<T> to a T is a copy
// CHECK:      [[val_0:%[0-9]+]] = OpLoad %type_ConstantBuffer_S %myCBuffer
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeExtract %v4float [[val_0]] 0
// CHECK-NEXT: [[e0:%[0-9]+]] = OpCompositeExtract %float [[vec]] 0
// CHECK-NEXT: [[e1:%[0-9]+]] = OpCompositeExtract %float [[vec]] 1
// CHECK-NEXT: [[e2:%[0-9]+]] = OpCompositeExtract %float [[vec]] 2
// CHECK-NEXT: [[e3:%[0-9]+]] = OpCompositeExtract %float [[vec]] 3
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeConstruct %v4float  [[e0]] [[e1]] [[e2]] [[e3]]
// CHECK-NEXT: [[tmp_0:%[0-9]+]] = OpCompositeConstruct %S_0 [[vec]]
// CHECK-NEXT:                OpStore %buffer2 [[tmp_0]]
    S buffer2;
    buffer2 = myCBuffer;

// We have the same struct type here
// CHECK:      [[val_1:%[0-9]+]] = OpFunctionCall %S_0 %retStuff
// CHECK-NEXT:                OpStore %buffer3 [[val_1]]
    S buffer3;
    buffer3 = retStuff();

// Write out each component recursively
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S %myASBuffer %uint_0 {{%[0-9]+}}
// CHECK-NEXT: [[val_2:%[0-9]+]] = OpLoad %type_ConstantBuffer_S %myCBuffer
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeExtract %v4float [[val_2]] 0
// CHECK-NEXT: [[e0:%[0-9]+]] = OpCompositeExtract %float [[vec]] 0
// CHECK-NEXT: [[e1:%[0-9]+]] = OpCompositeExtract %float [[vec]] 1
// CHECK-NEXT: [[e2:%[0-9]+]] = OpCompositeExtract %float [[vec]] 2
// CHECK-NEXT: [[e3:%[0-9]+]] = OpCompositeExtract %float [[vec]] 3
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeConstruct %v4float  [[e0]] [[e1]] [[e2]] [[e3]]
// CHECK-NEXT: [[tmp_1:%[0-9]+]] = OpCompositeConstruct %S_0 [[vec]]
// CHECK-NEXT: [[vec_2:%[0-9]+]] = OpCompositeExtract %v4float [[tmp_1]] 0
// CHECK-NEXT: [[tmp_2:%[0-9]+]] = OpCompositeConstruct %S [[vec_2]]
// CHECK-NEXT:                OpStore [[ptr]] [[tmp_2]]
    myASBuffer.Append(myCBuffer);

// Passing a ConstantBuffer<T> to a T parameter is a copy
// CHECK:      [[val_3:%[0-9]+]] = OpLoad %type_ConstantBuffer_S %myCBuffer
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeExtract %v4float [[val_3]] 0
// CHECK-NEXT: [[e0:%[0-9]+]] = OpCompositeExtract %float [[vec]] 0
// CHECK-NEXT: [[e1:%[0-9]+]] = OpCompositeExtract %float [[vec]] 1
// CHECK-NEXT: [[e2:%[0-9]+]] = OpCompositeExtract %float [[vec]] 2
// CHECK-NEXT: [[e3:%[0-9]+]] = OpCompositeExtract %float [[vec]] 3
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeConstruct %v4float  [[e0]] [[e1]] [[e2]] [[e3]]
// CHECK-NEXT: [[tmp_3:%[0-9]+]] = OpCompositeConstruct %S_0 [[vec]]
// CHECK-NEXT:                OpStore %param_var_buffer [[tmp_3]]
    return doStuff(myCBuffer);
}

S retStuff() {
// Returning a ConstantBuffer<T> as a T is a copy
// CHECK:      [[val_4:%[0-9]+]] = OpLoad %type_ConstantBuffer_S %myCBuffer
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeExtract %v4float [[val_4]] 0
// CHECK-NEXT: [[e0:%[0-9]+]] = OpCompositeExtract %float [[vec]] 0
// CHECK-NEXT: [[e1:%[0-9]+]] = OpCompositeExtract %float [[vec]] 1
// CHECK-NEXT: [[e2:%[0-9]+]] = OpCompositeExtract %float [[vec]] 2
// CHECK-NEXT: [[e3:%[0-9]+]] = OpCompositeExtract %float [[vec]] 3
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeConstruct %v4float  [[e0]] [[e1]] [[e2]] [[e3]]
// CHECK-NEXT: [[ret:%[0-9]+]] = OpCompositeConstruct %S_0 [[vec]]
// CHECK-NEXT:                OpReturnValue [[ret]]
    return myCBuffer;
}
