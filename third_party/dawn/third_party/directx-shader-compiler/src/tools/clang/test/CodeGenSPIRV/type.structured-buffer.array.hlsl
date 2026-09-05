// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: %_runtimearr_v4float = OpTypeRuntimeArray %v4float

// CHECK: %type_StructuredBuffer_v4float = OpTypeStruct %_runtimearr_v4float
// CHECK: %_arr_type_StructuredBuffer_v4float_uint_8 = OpTypeArray %type_StructuredBuffer_v4float %uint_8

// CHECK: %_runtimearr_uint = OpTypeRuntimeArray %uint

// CHECK: %type_ByteAddressBuffer = OpTypeStruct %_runtimearr_uint
// CHECK: %_runtimearr_type_ByteAddressBuffer = OpTypeRuntimeArray %type_ByteAddressBuffer

// CHECK: %type_RWByteAddressBuffer = OpTypeStruct %_runtimearr_uint
// CHECK: %_arr_type_RWByteAddressBuffer_uint_4 = OpTypeArray %type_RWByteAddressBuffer %uint_4

// CHECK:    %MySBuffer = OpVariable %_ptr_Uniform__arr_type_StructuredBuffer_v4float_uint_8 Uniform
StructuredBuffer<float4>   MySBuffer[8];
// CHECK:   %MyBABuffer = OpVariable %_ptr_Uniform__runtimearr_type_ByteAddressBuffer Uniform
ByteAddressBuffer          MyBABuffer[];
// CHECK: %MyRWBABuffer = OpVariable %_ptr_Uniform__arr_type_RWByteAddressBuffer_uint_4 Uniform
RWByteAddressBuffer        MyRWBABuffer[4];

float4 main(uint index : INDEX) : SV_Target {
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_type_ByteAddressBuffer %MyBABuffer {{%[0-9]+}}
// CHECK:                OpAccessChain %_ptr_Uniform_uint [[ptr]] %uint_0 {{%[0-9]+}}
    uint val1 = MyBABuffer[index].Load(index);

// CHECK: [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_type_RWByteAddressBuffer %MyRWBABuffer {{%[0-9]+}}
// CHECK:                OpAccessChain %_ptr_Uniform_uint [[ptr_0]] %uint_0 {{%[0-9]+}}
    MyRWBABuffer[index].Store(index, val1);

// CHECK: [[ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_type_StructuredBuffer_v4float %MySBuffer {{%[0-9]+}}
// CHECK:                OpStore %localSBuffer [[ptr_1]]
    StructuredBuffer<float4>   localSBuffer = MySBuffer[index];

// CHECK: [[ptr_2:%[0-9]+]] = OpLoad %_ptr_Uniform_type_StructuredBuffer_v4float %localSBuffer
// CHECK:                OpAccessChain %_ptr_Uniform_v4float [[ptr_2]] %int_0 {{%[0-9]+}}
    float4 val2 = localSBuffer[index];

    return val1 * val2;
}
