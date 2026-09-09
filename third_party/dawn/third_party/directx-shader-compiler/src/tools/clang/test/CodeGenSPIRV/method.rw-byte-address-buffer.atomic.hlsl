// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// Note: According to HLSL reference (https://msdn.microsoft.com/en-us/library/windows/desktop/ff471475(v=vs.85).aspx),
// all RWByteAddressBuffer atomic methods must take unsigned integers as parameters.

RWByteAddressBuffer myBuffer;

float4 main() : SV_Target
{
    uint originalVal;
    int  originalValAsInt;

// CHECK:      [[offset:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset]]
// CHECK-NEXT:        {{%[0-9]+}} = OpAtomicIAdd %uint [[ptr]] %uint_1 %uint_0 %uint_42
    myBuffer.InterlockedAdd(16, 42);
// CHECK:      [[offset_0:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_0]]
// CHECK-NEXT:    [[val:%[0-9]+]] = OpAtomicIAdd %uint [[ptr_0]] %uint_1 %uint_0 %uint_42
// CHECK-NEXT:                   OpStore %originalVal [[val]]
    myBuffer.InterlockedAdd(16, 42, originalVal);

// CHECK:      [[offset_1:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_1]]
// CHECK-NEXT:        {{%[0-9]+}} = OpAtomicAnd %uint [[ptr_1]] %uint_1 %uint_0 %uint_42
    myBuffer.InterlockedAnd(16, 42);
// CHECK:      [[offset_2:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr_2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_2]]
// CHECK-NEXT:    [[val_0:%[0-9]+]] = OpAtomicAnd %uint [[ptr_2]] %uint_1 %uint_0 %uint_42
// CHECK-NEXT:                   OpStore %originalVal [[val_0]]
    myBuffer.InterlockedAnd(16, 42, originalVal);

// CHECK:      [[offset_3:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr_3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_3]]
// CHECK-NEXT:        {{%[0-9]+}} = OpAtomicOr %uint [[ptr_3]] %uint_1 %uint_0 %uint_42
    myBuffer.InterlockedOr(16, 42);
// CHECK:      [[offset_4:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr_4:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_4]]
// CHECK-NEXT:    [[val_1:%[0-9]+]] = OpAtomicOr %uint [[ptr_4]] %uint_1 %uint_0 %uint_42
// CHECK-NEXT:                   OpStore %originalVal [[val_1]]
    myBuffer.InterlockedOr(16, 42, originalVal);

// CHECK:      [[offset_5:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr_5:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_5]]
// CHECK-NEXT:        {{%[0-9]+}} = OpAtomicXor %uint [[ptr_5]] %uint_1 %uint_0 %uint_42
    myBuffer.InterlockedXor(16, 42);
// CHECK:      [[offset_6:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr_6:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_6]]
// CHECK-NEXT:    [[val_2:%[0-9]+]] = OpAtomicXor %uint [[ptr_6]] %uint_1 %uint_0 %uint_42
// CHECK-NEXT:                   OpStore %originalVal [[val_2]]
    myBuffer.InterlockedXor(16, 42, originalVal);

// CHECK:      [[offset_7:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr_7:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_7]]
// CHECK-NEXT:        {{%[0-9]+}} = OpAtomicUMax %uint [[ptr_7]] %uint_1 %uint_0 %uint_42
    myBuffer.InterlockedMax(16, 42);
// CHECK:      [[offset_8:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr_8:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_8]]
// CHECK-NEXT:    [[val_3:%[0-9]+]] = OpAtomicUMax %uint [[ptr_8]] %uint_1 %uint_0 %uint_42
// CHECK-NEXT:                   OpStore %originalVal [[val_3]]
    myBuffer.InterlockedMax(16, 42, originalVal);

// CHECK:      [[offset_9:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr_9:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_9]]
// CHECK-NEXT:        {{%[0-9]+}} = OpAtomicUMax %uint [[ptr_9]] %uint_1 %uint_0 %uint_42
    myBuffer.InterlockedMax(16, 42);
// CHECK:      [[offset_10:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr_10:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_10]]
// CHECK-NEXT:    [[val_4:%[0-9]+]] = OpAtomicUMax %uint [[ptr_10]] %uint_1 %uint_0 %uint_42
// CHECK-NEXT:                   OpStore %originalVal [[val_4]]
    myBuffer.InterlockedMax(16, 42, originalVal);

// CHECK:      [[offset_11:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr_11:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_11]]
// CHECK-NEXT:        {{%[0-9]+}} = OpAtomicUMin %uint [[ptr_11]] %uint_1 %uint_0 %uint_42
    myBuffer.InterlockedMin(16, 42);
// CHECK:      [[offset_12:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr_12:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_12]]
// CHECK-NEXT:    [[val_5:%[0-9]+]] = OpAtomicUMin %uint [[ptr_12]] %uint_1 %uint_0 %uint_42
// CHECK-NEXT:                   OpStore %originalVal [[val_5]]
    myBuffer.InterlockedMin(16, 42, originalVal);

// CHECK:      [[offset_13:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr_13:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_13]]
// CHECK-NEXT:        {{%[0-9]+}} = OpAtomicUMin %uint [[ptr_13]] %uint_1 %uint_0 %uint_42
    myBuffer.InterlockedMin(16, 42);
// CHECK:      [[offset_14:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr_14:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_14]]
// CHECK-NEXT:    [[val_6:%[0-9]+]] = OpAtomicUMin %uint [[ptr_14]] %uint_1 %uint_0 %uint_42
// CHECK-NEXT:                   OpStore %originalVal [[val_6]]
    myBuffer.InterlockedMin(16, 42, originalVal);

    // .InterlockedExchnage() has no two-parameter overload.
// CHECK:      [[offset_15:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr_15:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_15]]
// CHECK-NEXT:    [[val_7:%[0-9]+]] = OpAtomicExchange %uint [[ptr_15]] %uint_1 %uint_0 %uint_42
// CHECK-NEXT:                   OpStore %originalVal [[val_7]]
    myBuffer.InterlockedExchange(16, 42, originalVal);

// CHECK:      [[offset_16:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr_16:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_16]]
// CHECK-NEXT:    [[val_8:%[0-9]+]] = OpAtomicCompareExchange %uint [[ptr_16]] %uint_1 %uint_0 %uint_0 %uint_42 %uint_30
// CHECK-NEXT:                   OpStore %originalVal [[val_8]]
    myBuffer.InterlockedCompareExchange(/*offset=*/16, /*compare_value=*/30, /*value=*/42, originalVal);

// CHECK:      [[offset_17:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr_17:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_17]]
// CHECK-NEXT:    [[val_9:%[0-9]+]] = OpAtomicCompareExchange %uint [[ptr_17]] %uint_1 %uint_0 %uint_0 %uint_42 %uint_30
// CHECK-NEXT:   [[cast:%[0-9]+]] = OpBitcast %int [[val_9]]
// CHECK-NEXT:                   OpStore %originalValAsInt [[cast]]
    myBuffer.InterlockedCompareExchange(/*offset=*/16, /*compare_value=*/30, /*value=*/42, originalValAsInt);

// CHECK:      [[offset_18:%[0-9]+]] = OpShiftRightLogical %uint %uint_16 %uint_2
// CHECK-NEXT:    [[ptr_18:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_18]]
// CHECK-NEXT:    [[val_10:%[0-9]+]] = OpAtomicCompareExchange %uint [[ptr_18]] %uint_1 %uint_0 %uint_0 %uint_42 %uint_30
// CHECK-NOT:                    [[val]]
    myBuffer.InterlockedCompareStore(/*offset=*/16, /*compare_value=*/30, /*value=*/42);

// CHECK:      [[offset_19:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK-NEXT:    [[ptr_19:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_19]]
// CHECK-NEXT:    [[val_19:%[0-9]+]] = OpBitcast %uint %int_n1
// CHECK-NEXT:           {{%[0-9]+}} = OpAtomicSMin %uint [[ptr_19]] %uint_1 %uint_0 [[val_19]]
    myBuffer.InterlockedMin(0u, -1);

// CHECK:      [[offset_20:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK-NEXT:    [[ptr_20:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_20]]
// CHECK-NEXT:    [[val_20:%[0-9]+]] = OpBitcast %uint %int_n1
// CHECK-NEXT:    [[res_20:%[0-9]+]] = OpAtomicSMax %uint [[ptr_20]] %uint_1 %uint_0 [[val_20]]
// CHECK-NEXT:                         OpStore %originalVal [[res_20]]
    myBuffer.InterlockedMax(0u, -1, originalVal);

// CHECK:      [[offset_21:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK-NEXT:    [[ptr_21:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[offset_21]]
// CHECK-NEXT:    [[val_21:%[0-9]+]] = OpBitcast %uint %int_n1
// CHECK-NEXT:    [[res_21:%[0-9]+]] = OpAtomicSMin %uint [[ptr_21]] %uint_1 %uint_0 [[val_21]]
// CHECK-NEXT:   [[res_21b:%[0-9]+]] = OpBitcast %int [[res_21]]
// CHECK-NEXT:                         OpStore %originalValAsInt [[res_21b]]
    myBuffer.InterlockedMin(0u, -1, originalValAsInt);

    return 1.0;
}
