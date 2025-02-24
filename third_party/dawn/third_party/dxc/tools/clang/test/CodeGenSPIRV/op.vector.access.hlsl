// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel
    float4 a;
    float1 b;
    float s;
    uint index;

    // Vector with more than one elements
// CHECK:      [[access0:%[0-9]+]] = OpAccessChain %_ptr_Function_float %a %uint_0
// CHECK-NEXT: [[a0:%[0-9]+]] = OpLoad %float [[access0]]
// CHECK-NEXT: OpStore %s [[a0]]
    s = a[0];
// CHECK-NEXT: [[s0:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[access1:%[0-9]+]] = OpAccessChain %_ptr_Function_float %a %uint_2
// CHECK-NEXT:       OpStore [[access1]] [[s0]]
    a[2] = s;

// CHECK-NEXT: [[index0:%[0-9]+]] = OpLoad %uint %index
// CHECK-NEXT: [[access2:%[0-9]+]] = OpAccessChain %_ptr_Function_float %a [[index0]]
// CHECK-NEXT: [[a1:%[0-9]+]] = OpLoad %float [[access2]]
// CHECK-NEXT: OpStore %s [[a1]]
    s = a[index];
// CHECK-NEXT: [[s1:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[index1:%[0-9]+]] = OpLoad %uint %index
// CHECK-NEXT: [[access3:%[0-9]+]] = OpAccessChain %_ptr_Function_float %a [[index1]]
// CHECK-NEXT: OpStore [[access3]] [[s1]]
    a[index] = s;

    // Vector with one elements
// CHECK-NEXT: [[b0:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: OpStore %s [[b0]]
    s = b[0];
// CHECK-NEXT: [[s2:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: OpStore %b [[s2]]
    b[0] = s;

// CHECK-NEXT: [[b1:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: OpStore %s [[b1]]
    s = b[index];
// CHECK-NEXT: [[s3:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: OpStore %b [[s3]]
    b[index] = s;

    // From rvalue
// CHECK-NEXT: [[a2:%[0-9]+]] = OpLoad %v4float %a
// CHECK-NEXT: [[a3:%[0-9]+]] = OpLoad %v4float %a
// CHECK-NEXT: [[add:%[0-9]+]] = OpFAdd %v4float [[a2]] [[a3]]
// CHECK-NEXT: OpStore %temp_var_vector [[add]]
// CHECK-NEXT: [[access4:%[0-9]+]] = OpAccessChain %_ptr_Function_float %temp_var_vector %uint_0
// CHECK-NEXT: [[s4:%[0-9]+]] = OpLoad %float [[access4]]
// CHECK-NEXT: OpStore %s [[s4]]
    s = (a + a)[0];
// CHECK-NEXT: [[index2:%[0-9]+]] = OpLoad %uint %index
// CHECK-NEXT: [[a4:%[0-9]+]] = OpLoad %v4float %a
// CHECK-NEXT: [[a5:%[0-9]+]] = OpLoad %v4float %a
// CHECK-NEXT: [[mul:%[0-9]+]] = OpFMul %v4float [[a4]] [[a5]]
// CHECK-NEXT: OpStore %temp_var_vector_0 [[mul]]
// CHECK-NEXT: [[access5:%[0-9]+]] = OpAccessChain %_ptr_Function_float %temp_var_vector_0 [[index2]]
// CHECK-NEXT: [[s5:%[0-9]+]] = OpLoad %float [[access5]]
// CHECK-NEXT: OpStore %s [[s5]]
    s = (a * a)[index];

    // The following will trigger frontend errors:
    //   subscripted value is not an array, matrix, or vector
    //s = (b + b)[0];
    //s = (b * b)[index];
}
