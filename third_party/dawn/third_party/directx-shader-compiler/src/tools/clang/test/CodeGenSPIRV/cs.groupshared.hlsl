// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    float  f1;
    float3 f2;
};

// CHECK-NOT: OpDecorate %a DescriptorSet
// CHECK-NOT: OpDecorate %b DescriptorSet
// CHECK-NOT: OpDecorate %c DescriptorSet
// CHECK-NOT: OpDecorate %d DescriptorSet
// CHECK-NOT: OpDecorate %s DescriptorSet

// CHECK: %a = OpVariable %_ptr_Workgroup_float Workgroup
groupshared              float    a;
// CHECK: %b = OpVariable %_ptr_Workgroup_v3float Workgroup
static groupshared       float3   b;  // Ignore static modifier
// CHECK: %c = OpVariable %_ptr_Workgroup_mat2v3float Workgroup
groupshared column_major float2x3 c;
// CHECK: %d = OpVariable %_ptr_Workgroup__arr_v2float_uint_5 Workgroup
groupshared              float2   d[5];
// CHECK: %s = OpVariable %_ptr_Workgroup_S Workgroup
groupshared              S        s;

[numthreads(8, 8, 8)]
void main(uint3 tid : SV_DispatchThreadID, uint2 gid : SV_GroupID) {
// Make sure pointers have the correct storage class
// CHECK:    {{%[0-9]+}} = OpAccessChain %_ptr_Workgroup_float %s %int_0
// CHECK: [[d0:%[0-9]+]] = OpAccessChain %_ptr_Workgroup_v2float %d %int_0
// CHECK:    {{%[0-9]+}} = OpAccessChain %_ptr_Workgroup_float [[d0]] %int_1
    d[0].y = s.f1;
}
