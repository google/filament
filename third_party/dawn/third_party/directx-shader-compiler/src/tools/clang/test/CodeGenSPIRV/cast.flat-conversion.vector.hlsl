// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S1 {
    float2 data[2];
};

StructuredBuffer<S1> MySB;

struct S2 {
  float b0;
  float3 b1;
};

struct S3 {
  float3 vec;
};

StructuredBuffer<float4> input2;




float4 main() : SV_TARGET
{
// CHECK:        [[ac:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr_v2float_uint_2 %MySB %int_0 %uint_0 %int_0
// CHECK-NEXT: [[mysb:%[0-9]+]] = OpLoad %_arr_v2float_uint_2 [[ac]]
// CHECK-NEXT: [[vec0:%[0-9]+]] = OpCompositeExtract %v2float [[mysb]] 0
// CHECK-NEXT: [[vec1:%[0-9]+]] = OpCompositeExtract %v2float [[mysb]] 1
// CHECK-NEXT:   [[v1:%[0-9]+]] = OpCompositeExtract %float [[vec0]] 0
// CHECK-NEXT:   [[v2:%[0-9]+]] = OpCompositeExtract %float [[vec0]] 1
// CHECK-NEXT:   [[v3:%[0-9]+]] = OpCompositeExtract %float [[vec1]] 0
// CHECK-NEXT:   [[v4:%[0-9]+]] = OpCompositeExtract %float [[vec1]] 1
// CHECK-NEXT:  [[val:%[0-9]+]] = OpCompositeConstruct %_arr_float_uint_4 [[v1]] [[v2]] [[v3]] [[v4]]
// CHECK-NEXT:                 OpStore %data [[val]]
    float data[4] = (float[4])MySB[0].data;


// CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float %input2 %int_0 %uint_0
// CHECK-NEXT: [[ld:%[0-9]+]] = OpLoad %v4float [[ac]]
// CHECK-NEXT: [[elem0:%[0-9]+]] = OpCompositeExtract %float [[ld]] 0
// CHECK-NEXT: [[elem1:%[0-9]+]] = OpCompositeExtract %float [[ld]] 1
// CHECK-NEXT: [[elem2:%[0-9]+]] = OpCompositeExtract %float [[ld]] 2
// CHECK-NEXT: [[elem3:%[0-9]+]] = OpCompositeExtract %float [[ld]] 3
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeConstruct %v3float [[elem1]] [[elem2]] [[elem3]]
// CHECK-NEXT: OpCompositeConstruct %S2 [[elem0]] [[vec]]
    S2 d2 = (S2)input2[0];


// CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float %input2 %int_0 %uint_0
// CHECK-NEXT: [[ld:%[0-9]+]] = OpLoad %v4float [[ac]]
// CHECK-NEXT: [[elem0:%[0-9]+]] = OpCompositeExtract %float [[ld]] 0
// CHECK-NEXT: [[elem1:%[0-9]+]] = OpCompositeExtract %float [[ld]] 1
// CHECK-NEXT: [[elem2:%[0-9]+]] = OpCompositeExtract %float [[ld]] 2
// CHECK-NEXT: [[elem3:%[0-9]+]] = OpCompositeExtract %float [[ld]] 3
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeConstruct %v3float [[elem0]] [[elem1]] [[elem2]]
// CHECK-NEXT: OpCompositeConstruct %S3 [[vec]]
    S3 d3 = (S3)input2[0];

    return 0;
}
