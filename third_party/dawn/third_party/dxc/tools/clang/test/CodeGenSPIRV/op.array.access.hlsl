// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// TODO: collect consecutive OpAccessChains into one

struct S {
    float f[4]; // nested array
    float g[4]; // nested array
};

// CHECK-LABEL: %src_main
float main(float val: A, uint index: B) : C {
    float r;

    S var[8][16];       // struct element
    float4 vecvar[4];   // vector element
    float2x3 matvar[4]; // matrix element

// CHECK:       [[val:%[0-9]+]] = OpLoad %float %val
// CHECK-NEXT:  [[idx:%[0-9]+]] = OpLoad %uint %index
// CHECK-NEXT: [[base:%[0-9]+]] = OpAccessChain %_ptr_Function__arr_float_uint_4 %var [[idx]] %int_1 %int_0
// CHECK-NEXT: [[ptr0:%[0-9]+]] = OpAccessChain %_ptr_Function_float [[base]] %int_2
// CHECK-NEXT:                 OpStore [[ptr0]] [[val]]

    var[index][1].f[2] = val;
// CHECK-NEXT: [[idx0:%[0-9]+]] = OpLoad %uint %index
// CHECK:      [[base:%[0-9]+]] = OpAccessChain %_ptr_Function__arr_float_uint_4 %var %int_0 [[idx0]] %int_1
// CHECK:      [[idx1:%[0-9]+]] = OpLoad %uint %index
// CHECK:      [[ptr0_0:%[0-9]+]] = OpAccessChain %_ptr_Function_float [[base]] [[idx1]]
// CHECK-NEXT: [[load:%[0-9]+]] = OpLoad %float [[ptr0_0]]
// CHECK-NEXT:                 OpStore %r [[load]]
    r = var[0][index].g[index];

// CHECK:       [[val_0:%[0-9]+]] = OpLoad %float %val
// CHECK-NEXT: [[vec2:%[0-9]+]] = OpCompositeConstruct %v2float [[val_0]] [[val_0]]
// CHECK-NEXT: [[ptr0_1:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %vecvar %int_3
// CHECK-NEXT: [[vec4:%[0-9]+]] = OpLoad %v4float [[ptr0_1]]
// CHECK-NEXT:  [[res:%[0-9]+]] = OpVectorShuffle %v4float [[vec4]] [[vec2]] 0 1 5 4
// CHECK-NEXT:                 OpStore [[ptr0_1]] [[res]]
    vecvar[3].ab = val;
// CHECK-NEXT: [[ptr2:%[0-9]+]] = OpAccessChain %_ptr_Function_float %vecvar %int_2 %uint_1
// CHECK-NEXT: [[load_0:%[0-9]+]] = OpLoad %float [[ptr2]]
// CHECK-NEXT:                 OpStore %r [[load_0]]
    r = vecvar[2][1];

// CHECK:       [[val_1:%[0-9]+]] = OpLoad %float %val
// CHECK-NEXT: [[vec2_0:%[0-9]+]] = OpCompositeConstruct %v2float [[val_1]] [[val_1]]
// CHECK-NEXT: [[ptr0_2:%[0-9]+]] = OpAccessChain %_ptr_Function_mat2v3float %matvar %int_2
// CHECK-NEXT: [[val0:%[0-9]+]] = OpCompositeExtract %float [[vec2_0]] 0
// CHECK-NEXT: [[ptr1:%[0-9]+]] = OpAccessChain %_ptr_Function_float [[ptr0_2]] %int_0 %int_1
// CHECK-NEXT:                 OpStore [[ptr1]] [[val0]]
// CHECK-NEXT: [[val1:%[0-9]+]] = OpCompositeExtract %float [[vec2_0]] 1
// CHECK-NEXT: [[ptr2_0:%[0-9]+]] = OpAccessChain %_ptr_Function_float [[ptr0_2]] %int_1 %int_2
// CHECK-NEXT:                 OpStore [[ptr2_0]] [[val1]]
    matvar[2]._12_23 = val;
// CHECK-NEXT: [[ptr4:%[0-9]+]] = OpAccessChain %_ptr_Function_float %matvar %int_0 %uint_1 %uint_2
// CHECK-NEXT: [[load_1:%[0-9]+]] = OpLoad %float [[ptr4]]
// CHECK-NEXT:                 OpStore %r [[load_1]]
    r = matvar[0][1][2];

//
// Test using a boolean as index
//
// CHECK:            [[index:%[0-9]+]] = OpLoad %uint %index
// CHECK-NEXT:   [[indexBool:%[0-9]+]] = OpINotEqual %bool [[index]] %uint_0
// CHECK-NEXT:    [[indexNot:%[0-9]+]] = OpLogicalNot %bool [[indexBool]]
// CHECK-NEXT: [[indexResult:%[0-9]+]] = OpSelect %uint [[indexNot]] %uint_1 %uint_0
// CHECK-NEXT:             {{%[0-9]+}} = OpAccessChain %_ptr_Function_v4float %vecvar [[indexResult]]
    float4 result = vecvar[!index];

    return r;
}
