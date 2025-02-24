// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    float4 f;
};

struct T1 {
    float3 f;
};

struct T2 {
    float2 f;
};

StructuredBuffer<S>         globalSBuffer;
RWStructuredBuffer<S>       globalRWSBuffer;
AppendStructuredBuffer<T1>  globalASBuffer;
ConsumeStructuredBuffer<T2> globalCSBuffer;
ByteAddressBuffer           globalBABuffer;
RWByteAddressBuffer         globalRWBABuffer;

float4 main() : SV_Target {
// CHECK: %localSBuffer = OpVariable %_ptr_Function__ptr_Uniform_type_StructuredBuffer_S Function
// CHECK: %localRWSBuffer = OpVariable %_ptr_Function__ptr_Uniform_type_RWStructuredBuffer_S Function
// CHECK: %localASBuffer = OpVariable %_ptr_Function__ptr_Uniform_type_AppendStructuredBuffer_T1 Function
// CHECK: %localCSBuffer = OpVariable %_ptr_Function__ptr_Uniform_type_ConsumeStructuredBuffer_T2 Function
// CHECK: %localBABuffer = OpVariable %_ptr_Function__ptr_Uniform_type_ByteAddressBuffer Function
// CHECK: %localRWBABuffer = OpVariable %_ptr_Function__ptr_Uniform_type_RWByteAddressBuffer Function

// CHECK: OpStore %localSBuffer %globalSBuffer
// CHECK: OpStore %localRWSBuffer %globalRWSBuffer
// CHECK: OpStore %localASBuffer %globalASBuffer
// CHECK: OpStore %localCSBuffer %globalCSBuffer
// CHECK: OpStore %localBABuffer %globalBABuffer
// CHECK: OpStore %localRWBABuffer %globalRWBABuffer
    StructuredBuffer<S>         localSBuffer    = globalSBuffer;
    RWStructuredBuffer<S>       localRWSBuffer  = globalRWSBuffer;
    AppendStructuredBuffer<T1>  localASBuffer   = globalASBuffer;
    ConsumeStructuredBuffer<T2> localCSBuffer   = globalCSBuffer;
    ByteAddressBuffer           localBABuffer   = globalBABuffer;
    RWByteAddressBuffer         localRWBABuffer = globalRWBABuffer;

    T1 t1 = {float3(1., 2., 3.)};
    T2 t2;
    uint numStructs, stride, counter;
    float4 val;

// CHECK:      [[ptr:%[0-9]+]] = OpLoad %_ptr_Uniform_type_StructuredBuffer_S %localSBuffer
// CHECK-NEXT:     {{%[0-9]+}} = OpArrayLength %uint [[ptr]] 0
    localSBuffer.GetDimensions(numStructs, stride);
// CHECK:      [[ptr1:%[0-9]+]] = OpLoad %_ptr_Uniform_type_StructuredBuffer_S %localSBuffer
// CHECK-NEXT: [[ptr2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float [[ptr1]] %int_0 %int_1 %int_0
// CHECK-NEXT:      {{%[0-9]+}} = OpLoad %v4float [[ptr2]]
    val = localSBuffer.Load(1).f;
// CHECK:      [[ptr1_0:%[0-9]+]] = OpLoad %_ptr_Uniform_type_StructuredBuffer_S %localSBuffer
// CHECK-NEXT: [[ptr2_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float [[ptr1_0]] %int_0 %uint_2 %int_0
// CHECK-NEXT:      {{%[0-9]+}} = OpLoad %v4float [[ptr2_0]]
    val = localSBuffer[2].f;

// CHECK:      [[ptr_0:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWStructuredBuffer_S %localRWSBuffer
// CHECK-NEXT:     {{%[0-9]+}} = OpArrayLength %uint [[ptr_0]] 0
    localRWSBuffer.GetDimensions(numStructs, stride);
// CHECK:      [[ptr1_1:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWStructuredBuffer_S %localRWSBuffer
// CHECK-NEXT: [[ptr2_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float [[ptr1_1]] %int_0 %int_3 %int_0
// CHECK-NEXT:      {{%[0-9]+}} = OpLoad %v4float [[ptr2_1]]
    val = localRWSBuffer.Load(3).f;
// CHECK:      [[ptr1_2:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWStructuredBuffer_S %localRWSBuffer
// CHECK-NEXT: [[ptr2_2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float [[ptr1_2]] %int_0 %uint_4 %int_0
// CHECK-NEXT:                 OpStore [[ptr2_2]] {{%[0-9]+}}
    localRWSBuffer[4].f = 42.;

// CHECK:      [[ptr_1:%[0-9]+]] = OpLoad %_ptr_Uniform_type_AppendStructuredBuffer_T1 %localASBuffer
// CHECK-NEXT:     {{%[0-9]+}} = OpArrayLength %uint [[ptr_1]] 0
    localASBuffer.GetDimensions(numStructs, stride);

// CHECK:      [[ptr_2:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ConsumeStructuredBuffer_T2 %localCSBuffer
// CHECK-NEXT:     {{%[0-9]+}} = OpArrayLength %uint [[ptr_2]] 0
    localCSBuffer.GetDimensions(numStructs, stride);

    uint  byte;
    uint2 byte2;
    uint3 byte3;
    uint4 byte4;
    uint  dim;

    uint dest, value, compare, origin;

// CHECK:      [[ptr_3:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ByteAddressBuffer %localBABuffer
// CHECK-NEXT:     {{%[0-9]+}} = OpArrayLength %uint [[ptr_3]] 0
    localBABuffer.GetDimensions(dim);
// CHECK:      [[ptr1_3:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ByteAddressBuffer %localBABuffer
// CHECK:      [[ptr2_3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_3]] %uint_0 {{%[0-9]+}}
// CHECK-NEXT:      {{%[0-9]+}} = OpLoad %uint [[ptr2_3]]
    byte  = localBABuffer.Load(4);
// CHECK:      [[ptr1_4:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ByteAddressBuffer %localBABuffer
// CHECK:      [[ptr2_4:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_4]] %uint_0 {{%[0-9]+}}
// CHECK-NEXT:      {{%[0-9]+}} = OpLoad %uint [[ptr2_4]]
    byte2 = localBABuffer.Load2(5);
// CHECK:      [[ptr1_5:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ByteAddressBuffer %localBABuffer
// CHECK:      [[ptr2_5:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_5]] %uint_0 {{%[0-9]+}}
// CHECK-NEXT:      {{%[0-9]+}} = OpLoad %uint [[ptr2_5]]
    byte3 = localBABuffer.Load3(6);
// CHECK:      [[ptr1_6:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ByteAddressBuffer %localBABuffer
// CHECK:      [[ptr2_6:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_6]] %uint_0 {{%[0-9]+}}
// CHECK-NEXT:      {{%[0-9]+}} = OpLoad %uint [[ptr2_6]]
    byte4 = localBABuffer.Load4(7);

// CHECK:      [[ptr_4:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWByteAddressBuffer %localRWBABuffer
// CHECK-NEXT:     {{%[0-9]+}} = OpArrayLength %uint [[ptr_4]] 0
    localRWBABuffer.GetDimensions(dim);
// CHECK:      [[ptr1_7:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWByteAddressBuffer %localRWBABuffer
// CHECK:      [[ptr2_7:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_7]] %uint_0 {{%[0-9]+}}
// CHECK-NEXT:      {{%[0-9]+}} = OpLoad %uint [[ptr2_7]]
    byte  = localRWBABuffer.Load(8);
// CHECK:      [[ptr1_8:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWByteAddressBuffer %localRWBABuffer
// CHECK:      [[ptr2_8:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_8]] %uint_0 {{%[0-9]+}}
// CHECK-NEXT:      {{%[0-9]+}} = OpLoad %uint [[ptr2_8]]
    byte2 = localRWBABuffer.Load2(9);
// CHECK:      [[ptr1_9:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWByteAddressBuffer %localRWBABuffer
// CHECK:      [[ptr2_9:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_9]] %uint_0 {{%[0-9]+}}
// CHECK-NEXT:      {{%[0-9]+}} = OpLoad %uint [[ptr2_9]]
    byte3 = localRWBABuffer.Load3(10);
// CHECK:      [[ptr1_10:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWByteAddressBuffer %localRWBABuffer
// CHECK:      [[ptr2_10:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_10]] %uint_0 {{%[0-9]+}}
// CHECK-NEXT:      {{%[0-9]+}} = OpLoad %uint [[ptr2_10]]
    byte4 = localRWBABuffer.Load4(11);
// CHECK:      [[ptr1_11:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWByteAddressBuffer %localRWBABuffer
// CHECK:      [[ptr2_11:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_11]] %uint_0 {{%[0-9]+}}
// CHECK-NEXT:                 OpStore [[ptr2_11]] {{%[0-9]+}}
    localRWBABuffer.Store(12, byte);
// CHECK:      [[ptr1_12:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWByteAddressBuffer %localRWBABuffer
// CHECK:      [[ptr2_12:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_12]] %uint_0 {{%[0-9]+}}
// CHECK-NEXT:                 OpStore [[ptr2_12]] {{%[0-9]+}}
    localRWBABuffer.Store2(13, byte2);
// CHECK:      [[ptr1_13:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWByteAddressBuffer %localRWBABuffer
// CHECK:      [[ptr2_13:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_13]] %uint_0 {{%[0-9]+}}
// CHECK-NEXT:                 OpStore [[ptr2_13]] {{%[0-9]+}}
    localRWBABuffer.Store3(14, byte3);
// CHECK:      [[ptr1_14:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWByteAddressBuffer %localRWBABuffer
// CHECK:      [[ptr2_14:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_14]] %uint_0 {{%[0-9]+}}
// CHECK-NEXT:                 OpStore [[ptr2_14]] {{%[0-9]+}}
    localRWBABuffer.Store4(15, byte4);
// CHECK:      [[ptr1_15:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWByteAddressBuffer %localRWBABuffer
// CHECK:      [[ptr2_15:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_15]] %uint_0 {{%[0-9]+}}
    localRWBABuffer.InterlockedAdd(dest, value, origin);
// CHECK:      [[ptr1_16:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWByteAddressBuffer %localRWBABuffer
// CHECK:      [[ptr2_16:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_16]] %uint_0 {{%[0-9]+}}
    localRWBABuffer.InterlockedAnd(dest, value, origin);
// CHECK:      [[ptr1_17:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWByteAddressBuffer %localRWBABuffer
// CHECK:      [[ptr2_17:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_17]] %uint_0 {{%[0-9]+}}
    localRWBABuffer.InterlockedCompareExchange(dest, compare, value, origin);
// CHECK:      [[ptr1_18:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWByteAddressBuffer %localRWBABuffer
// CHECK:      [[ptr2_18:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_18]] %uint_0 {{%[0-9]+}}
    localRWBABuffer.InterlockedCompareStore(dest, compare, value);
// CHECK:      [[ptr1_19:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWByteAddressBuffer %localRWBABuffer
// CHECK:      [[ptr2_19:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_19]] %uint_0 {{%[0-9]+}}
    localRWBABuffer.InterlockedExchange(dest, value, origin);
// CHECK:      [[ptr1_20:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWByteAddressBuffer %localRWBABuffer
// CHECK:      [[ptr2_20:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_20]] %uint_0 {{%[0-9]+}}
    localRWBABuffer.InterlockedMax(dest, value, origin);
// CHECK:      [[ptr1_21:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWByteAddressBuffer %localRWBABuffer
// CHECK:      [[ptr2_21:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_21]] %uint_0 {{%[0-9]+}}
    localRWBABuffer.InterlockedMin(dest, value, origin);
// CHECK:      [[ptr1_22:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWByteAddressBuffer %localRWBABuffer
// CHECK:      [[ptr2_22:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_22]] %uint_0 {{%[0-9]+}}
    localRWBABuffer.InterlockedOr(dest, value, origin);
// CHECK:      [[ptr1_23:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWByteAddressBuffer %localRWBABuffer
// CHECK:      [[ptr2_23:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ptr1_23]] %uint_0 {{%[0-9]+}}
    localRWBABuffer.InterlockedXor(dest, value, origin);

    return val;
}
