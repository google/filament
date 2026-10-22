// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// Note: The following is invalid SPIR-V code.
//
// * The assignment ignores storage class (and thus layout) difference.

// %type_TextureBuffer_S is the type for myTBuffer. With layout decoration.
// %S is the type for myASBuffer elements. With layout decoration.
// %S_0 is the type for function local variables. Without layout decoration.

// CHECK:     OpMemberDecorate %type_TextureBuffer_S 0 Offset 0
// CHECK:     OpMemberDecorate %S 0 Offset 0
// CHECK-NOT: OpMemberDecorate %S_0 0 Offset 0

// CHECK:     %type_TextureBuffer_S = OpTypeStruct %v4float
// CHECK:     %S = OpTypeStruct %v4float
// CHECK:     %S_0 = OpTypeStruct %v4float
struct S {
    float4 f;
};

// CHECK: %myTBuffer = OpVariable %_ptr_Uniform_type_TextureBuffer_S Uniform
TextureBuffer<S>          myTBuffer;
AppendStructuredBuffer<S> myASBuffer;

S retStuff();

float4 doStuff(S buffer) {
    return buffer.f;
}

float4 main(in float4 pos : SV_Position) : SV_Target
{
// Initializing a T with a TextureBuffer<T> is a copy
// CHECK:      [[val:%[0-9]+]] = OpLoad %type_TextureBuffer_S %myTBuffer
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeExtract %v4float [[val]] 0
// CHECK-NEXT: [[e0:%[0-9]+]] = OpCompositeExtract %float [[vec]] 0
// CHECK-NEXT: [[e1:%[0-9]+]] = OpCompositeExtract %float [[vec]] 1
// CHECK-NEXT: [[e2:%[0-9]+]] = OpCompositeExtract %float [[vec]] 2
// CHECK-NEXT: [[e3:%[0-9]+]] = OpCompositeExtract %float [[vec]] 3
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeConstruct %v4float  [[e0]] [[e1]] [[e2]] [[e3]]
// CHECK: [[tmp:%[0-9]+]] = OpCompositeConstruct %S_0 [[vec]]
// CHECK-NEXT:                OpStore %buffer1 [[tmp]]
    S buffer1 = myTBuffer;

// Assigning a TextureBuffer<T> to a T is a copy
// CHECK:      [[val_0:%[0-9]+]] = OpLoad %type_TextureBuffer_S %myTBuffer
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeExtract %v4float [[val_0]] 0
// CHECK-NEXT: [[e0:%[0-9]+]] = OpCompositeExtract %float [[vec]] 0
// CHECK-NEXT: [[e1:%[0-9]+]] = OpCompositeExtract %float [[vec]] 1
// CHECK-NEXT: [[e2:%[0-9]+]] = OpCompositeExtract %float [[vec]] 2
// CHECK-NEXT: [[e3:%[0-9]+]] = OpCompositeExtract %float [[vec]] 3
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeConstruct %v4float  [[e0]] [[e1]] [[e2]] [[e3]]
// CHECK-NEXT: [[tmp_0:%[0-9]+]] = OpCompositeConstruct %S_0 [[vec]]
// CHECK-NEXT:                OpStore %buffer2 [[tmp_0]]
    S buffer2;
    buffer2 = myTBuffer;

// We have the same struct type here
// CHECK:      [[val_1:%[0-9]+]] = OpFunctionCall %S_0 %retStuff
// CHECK-NEXT:                OpStore %buffer3 [[val_1]]
    S buffer3;
    buffer3 = retStuff();

// TODO: The underlying struct type has the same layout but %type_TextureBuffer_S
// has an additional BufferBlock decoration. So this causes an validation error.
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S %myASBuffer %uint_0 {{%[0-9]+}}
// CHECK-NEXT:  [[tb:%[0-9]+]] = OpLoad %type_TextureBuffer_S %myTBuffer
// CHECK-NEXT: [[vec_1:%[0-9]+]] = OpCompositeExtract %v4float [[tb]] 0
// CHECK-NEXT: [[e0:%[0-9]+]] = OpCompositeExtract %float [[vec_1]] 0
// CHECK-NEXT: [[e1:%[0-9]+]] = OpCompositeExtract %float [[vec_1]] 1
// CHECK-NEXT: [[e2:%[0-9]+]] = OpCompositeExtract %float [[vec_1]] 2
// CHECK-NEXT: [[e3:%[0-9]+]] = OpCompositeExtract %float [[vec_1]] 3
// CHECK-NEXT: [[vec_1:%[0-9]+]] = OpCompositeConstruct %v4float  [[e0]] [[e1]] [[e2]] [[e3]]
// CHECK-NEXT: [[loc:%[0-9]+]] = OpCompositeConstruct %S_0 [[vec_1]]
// CHECK-NEXT: [[vec_2:%[0-9]+]] = OpCompositeExtract %v4float [[loc]] 0
// CHECK-NEXT: [[val_2:%[0-9]+]] = OpCompositeConstruct %S [[vec_2]]
// CHECK-NEXT:                OpStore [[ptr]] [[val_2]]
    myASBuffer.Append(myTBuffer);

// Passing a TextureBuffer<T> to a T parameter is a copy
// CHECK:      [[val_3:%[0-9]+]] = OpLoad %type_TextureBuffer_S %myTBuffer
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeExtract %v4float [[val_3]] 0
// CHECK-NEXT: [[e0:%[0-9]+]] = OpCompositeExtract %float [[vec]] 0
// CHECK-NEXT: [[e1:%[0-9]+]] = OpCompositeExtract %float [[vec]] 1
// CHECK-NEXT: [[e2:%[0-9]+]] = OpCompositeExtract %float [[vec]] 2
// CHECK-NEXT: [[e3:%[0-9]+]] = OpCompositeExtract %float [[vec]] 3
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeConstruct %v4float  [[e0]] [[e1]] [[e2]] [[e3]]
// CHECK-NEXT: [[tmp_1:%[0-9]+]] = OpCompositeConstruct %S_0 [[vec]]
// CHECK-NEXT:                OpStore %param_var_buffer [[tmp_1]]
    return doStuff(myTBuffer);
}

S retStuff() {
// Returning a TextureBuffer<T> as a T is a copy
// CHECK:      [[val_4:%[0-9]+]] = OpLoad %type_TextureBuffer_S %myTBuffer
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeExtract %v4float [[val_4]] 0
// CHECK-NEXT: [[e0:%[0-9]+]] = OpCompositeExtract %float [[vec]] 0
// CHECK-NEXT: [[e1:%[0-9]+]] = OpCompositeExtract %float [[vec]] 1
// CHECK-NEXT: [[e2:%[0-9]+]] = OpCompositeExtract %float [[vec]] 2
// CHECK-NEXT: [[e3:%[0-9]+]] = OpCompositeExtract %float [[vec]] 3
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeConstruct %v4float  [[e0]] [[e1]] [[e2]] [[e3]]
// CHECK-NEXT: [[ret:%[0-9]+]] = OpCompositeConstruct %S_0 [[vec]]
// CHECK-NEXT:                OpReturnValue [[ret]]
    return myTBuffer;
}
