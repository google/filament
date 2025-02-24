// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    float2x3 mat;
    float3 vec3;
    float2 vec2;
    float scalar;
    uint index;

    // 1 element (from lvalue)
// CHECK:      [[access0:%[0-9]+]] = OpAccessChain %_ptr_Function_float %mat %int_1 %int_2
// CHECK-NEXT: [[load0:%[0-9]+]] = OpLoad %float [[access0]]
// CHECK-NEXT: OpStore %scalar [[load0]]
    scalar = mat._m12; // Used as rvalue
// CHECK-NEXT: [[load1:%[0-9]+]] = OpLoad %float %scalar
// CHECK-NEXT: [[access1:%[0-9]+]] = OpAccessChain %_ptr_Function_float %mat %int_0 %int_1
// CHECK-NEXT: OpStore [[access1]] [[load1]]
    mat._12 = scalar; // Used as lvalue

    // >1 elements (from lvalue)
// CHECK-NEXT: [[access2:%[0-9]+]] = OpAccessChain %_ptr_Function_float %mat %int_0 %int_1
// CHECK-NEXT: [[load2:%[0-9]+]] = OpLoad %float [[access2]]
// CHECK-NEXT: [[access3:%[0-9]+]] = OpAccessChain %_ptr_Function_float %mat %int_0 %int_2
// CHECK-NEXT: [[load3:%[0-9]+]] = OpLoad %float [[access3]]
// CHECK-NEXT: [[cc0:%[0-9]+]] = OpCompositeConstruct %v2float [[load2]] [[load3]]
// CHECK-NEXT: OpStore %vec2 [[cc0]]
    vec2 = mat._m01_m02; // Used as rvalue
// CHECK-NEXT: [[rhs0:%[0-9]+]] = OpLoad %v3float %vec3
// CHECK-NEXT: [[ce0:%[0-9]+]] = OpCompositeExtract %float [[rhs0]] 0
// CHECK-NEXT: [[access4:%[0-9]+]] = OpAccessChain %_ptr_Function_float %mat %int_1 %int_0
// CHECK-NEXT: OpStore [[access4]] [[ce0]]
// CHECK-NEXT: [[ce1:%[0-9]+]] = OpCompositeExtract %float [[rhs0]] 1
// CHECK-NEXT: [[access5:%[0-9]+]] = OpAccessChain %_ptr_Function_float %mat %int_0 %int_1
// CHECK-NEXT: OpStore [[access5]] [[ce1]]
// CHECK-NEXT: [[ce2:%[0-9]+]] = OpCompositeExtract %float [[rhs0]] 2
// CHECK-NEXT: [[access6:%[0-9]+]] = OpAccessChain %_ptr_Function_float %mat %int_0 %int_0
// CHECK-NEXT: OpStore [[access6]] [[ce2]]
    mat._21_12_11 = vec3; // Used as lvalue

    // 1 element (from rvalue)
// CHECK:      [[cc1:%[0-9]+]] = OpCompositeConstruct %mat2v3float {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT: [[ce3:%[0-9]+]] = OpCompositeExtract %float [[cc1]] 1 2
// CHECK-NEXT: OpStore %scalar [[ce3]]
    // Codegen: construct a temporary matrix first out of (mat + mat) and
    // then extract the value
    scalar = (mat + mat)._m12;

    // > 1 element (from rvalue)
// CHECK:      [[cc2:%[0-9]+]] = OpCompositeConstruct %mat2v3float {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT: [[ce4:%[0-9]+]] = OpCompositeExtract %float [[cc2]] 0 1
// CHECK-NEXT: [[ce5:%[0-9]+]] = OpCompositeExtract %float [[cc2]] 0 2
// CHECK-NEXT: [[cc3:%[0-9]+]] = OpCompositeConstruct %v2float [[ce4]] [[ce5]]
// CHECK-NEXT: OpStore %vec2 [[cc3]]
    // Codegen: construct a temporary matrix first out of (mat * mat) and
    // then extract the value
    vec2 = (mat * mat)._m01_m02;

    // One level indexing (from lvalue)
// CHECK-NEXT: [[access7:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float %mat %uint_1
// CHECK-NEXT: [[load4:%[0-9]+]] = OpLoad %v3float [[access7]]
// CHECK-NEXT: OpStore %vec3 [[load4]]
    vec3 = mat[1]; // Used as rvalue

    // One level indexing (from lvalue)
// CHECK-NEXT: [[load5:%[0-9]+]] = OpLoad %v3float %vec3
// CHECK-NEXT: [[index0:%[0-9]+]] = OpLoad %uint %index
// CHECK-NEXT: [[access8:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float %mat [[index0]]
// CHECK-NEXT: OpStore [[access8]] [[load5]]
    mat[index] = vec3; // Used as lvalue

    // Two level indexing (from lvalue)
// CHECK-NEXT: [[index1:%[0-9]+]] = OpLoad %uint %index
// CHECK-NEXT: [[access9:%[0-9]+]] = OpAccessChain %_ptr_Function_float %mat [[index1]] %uint_2
// CHECK-NEXT: [[load6:%[0-9]+]] = OpLoad %float [[access9]]
// CHECK-NEXT: OpStore %scalar [[load6]]
    scalar = mat[index][2]; // Used as rvalue

    // Two level indexing (from lvalue)
// CHECK-NEXT: [[load7:%[0-9]+]] = OpLoad %float %scalar
// CHECK-NEXT: [[index2:%[0-9]+]] = OpLoad %uint %index
// CHECK-NEXT: [[access10:%[0-9]+]] = OpAccessChain %_ptr_Function_float %mat %uint_1 [[index2]]
// CHECK-NEXT: OpStore [[access10]] [[load7]]
    mat[1][index] = scalar; // Used as lvalue

    // One level indexing (from rvalue)
// CHECK:      [[cc4:%[0-9]+]] = OpCompositeConstruct %mat2v3float {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT: OpStore %temp_var_vector [[cc4]]
// CHECK-NEXT: [[access11:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float %temp_var_vector %uint_0
// CHECK-NEXT: [[load8:%[0-9]+]] = OpLoad %v3float [[access11]]
// CHECK-NEXT: OpStore %vec3 [[load8]]
    vec3 = (mat + mat)[0];

    // Two level indexing (from rvalue)
// CHECK-NEXT: [[index3:%[0-9]+]] = OpLoad %uint %index
// CHECK:      [[cc5:%[0-9]+]] = OpCompositeConstruct %mat2v3float {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT: OpStore %temp_var_vector_0 [[cc5]]
// CHECK-NEXT: [[access12:%[0-9]+]] = OpAccessChain %_ptr_Function_float %temp_var_vector_0 %uint_0 [[index3]]
// CHECK-NEXT: [[load9:%[0-9]+]] = OpLoad %float [[access12]]
// CHECK-NEXT: OpStore %scalar [[load9]]
    scalar = (mat + mat)[0][index];

// Try non-floating point matrix as they are represented differently (Array of vectors).
    int2x3 intMat;
    int3 intVec3;
    int2 intVec2;
    int intScalar;

    // 1 element (from lvalue)
// CHECK:      [[access0_0:%[0-9]+]] = OpAccessChain %_ptr_Function_int %intMat %int_1 %int_2
// CHECK-NEXT: [[load0_0:%[0-9]+]] = OpLoad %int [[access0_0]]
// CHECK-NEXT: OpStore %intScalar [[load0_0]]
    intScalar = intMat._m12; // Used as rvalue
// CHECK-NEXT: [[load1_0:%[0-9]+]] = OpLoad %int %intScalar
// CHECK-NEXT: [[access1_0:%[0-9]+]] = OpAccessChain %_ptr_Function_int %intMat %int_0 %int_1
// CHECK-NEXT: OpStore [[access1_0]] [[load1_0]]
    intMat._12 = intScalar; // Used as lvalue

    // >1 elements (from lvalue)
// CHECK-NEXT: [[access2_0:%[0-9]+]] = OpAccessChain %_ptr_Function_int %intMat %int_0 %int_1
// CHECK-NEXT: [[load2_0:%[0-9]+]] = OpLoad %int [[access2_0]]
// CHECK-NEXT: [[access3_0:%[0-9]+]] = OpAccessChain %_ptr_Function_int %intMat %int_0 %int_2
// CHECK-NEXT: [[load3_0:%[0-9]+]] = OpLoad %int [[access3_0]]
// CHECK-NEXT: [[cc0_0:%[0-9]+]] = OpCompositeConstruct %v2int [[load2_0]] [[load3_0]]
// CHECK-NEXT: OpStore %intVec2 [[cc0_0]]
    intVec2 = intMat._m01_m02; // Used as rvalue
// CHECK-NEXT: [[rhs0_0:%[0-9]+]] = OpLoad %v3int %intVec3
// CHECK-NEXT: [[ce0_0:%[0-9]+]] = OpCompositeExtract %int [[rhs0_0]] 0
// CHECK-NEXT: [[access4_0:%[0-9]+]] = OpAccessChain %_ptr_Function_int %intMat %int_1 %int_0
// CHECK-NEXT: OpStore [[access4_0]] [[ce0_0]]
// CHECK-NEXT: [[ce1_0:%[0-9]+]] = OpCompositeExtract %int [[rhs0_0]] 1
// CHECK-NEXT: [[access5_0:%[0-9]+]] = OpAccessChain %_ptr_Function_int %intMat %int_0 %int_1
// CHECK-NEXT: OpStore [[access5_0]] [[ce1_0]]
// CHECK-NEXT: [[ce2_0:%[0-9]+]] = OpCompositeExtract %int [[rhs0_0]] 2
// CHECK-NEXT: [[access6_0:%[0-9]+]] = OpAccessChain %_ptr_Function_int %intMat %int_0 %int_0
// CHECK-NEXT: OpStore [[access6_0]] [[ce2_0]]
    intMat._21_12_11 = intVec3; // Used as lvalue

    // 1 element (from rvalue)
// CHECK:      [[cc1_0:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT: [[ce3_0:%[0-9]+]] = OpCompositeExtract %int [[cc1_0]] 1 2
// CHECK-NEXT: OpStore %intScalar [[ce3_0]]
    // Codegen: construct a temporary matrix first out of (intMat + intMat) and
    // then extract the value
    intScalar = (intMat + intMat)._m12;

    // > 1 element (from rvalue)
// CHECK:      [[cc2_0:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT: [[ce4_0:%[0-9]+]] = OpCompositeExtract %int [[cc2_0]] 0 1
// CHECK-NEXT: [[ce5_0:%[0-9]+]] = OpCompositeExtract %int [[cc2_0]] 0 2
// CHECK-NEXT: [[cc3_0:%[0-9]+]] = OpCompositeConstruct %v2int [[ce4_0]] [[ce5_0]]
// CHECK-NEXT: OpStore %intVec2 [[cc3_0]]
    // Codegen: construct a temporary matrix first out of (intMat * intMat) and
    // then extract the value
    intVec2 = (intMat * intMat)._m01_m02;

    // One level indexing (from lvalue)
// CHECK-NEXT: [[access7_0:%[0-9]+]] = OpAccessChain %_ptr_Function_v3int %intMat %uint_1
// CHECK-NEXT: [[load4_0:%[0-9]+]] = OpLoad %v3int [[access7_0]]
// CHECK-NEXT: OpStore %intVec3 [[load4_0]]
    intVec3 = intMat[1]; // Used as rvalue

    // One level indexing (from lvalue)
// CHECK-NEXT: [[load5_0:%[0-9]+]] = OpLoad %v3int %intVec3
// CHECK-NEXT: [[index0_0:%[0-9]+]] = OpLoad %uint %index
// CHECK-NEXT: [[access8_0:%[0-9]+]] = OpAccessChain %_ptr_Function_v3int %intMat [[index0_0]]
// CHECK-NEXT: OpStore [[access8_0]] [[load5_0]]
    intMat[index] = intVec3; // Used as lvalue

    // Two level indexing (from lvalue)
// CHECK-NEXT: [[index1_0:%[0-9]+]] = OpLoad %uint %index
// CHECK-NEXT: [[access9_0:%[0-9]+]] = OpAccessChain %_ptr_Function_int %intMat [[index1_0]] %uint_2
// CHECK-NEXT: [[load6_0:%[0-9]+]] = OpLoad %int [[access9_0]]
// CHECK-NEXT: OpStore %intScalar [[load6_0]]
    intScalar = intMat[index][2]; // Used as rvalue

    // Two level indexing (from lvalue)
// CHECK-NEXT: [[load7_0:%[0-9]+]] = OpLoad %int %intScalar
// CHECK-NEXT: [[index2_0:%[0-9]+]] = OpLoad %uint %index
// CHECK-NEXT: [[access10_0:%[0-9]+]] = OpAccessChain %_ptr_Function_int %intMat %uint_1 [[index2_0]]
// CHECK-NEXT: OpStore [[access10_0]] [[load7_0]]
    intMat[1][index] = intScalar; // Used as lvalue

    // One level indexing (from rvalue)
// CHECK:      [[cc4_0:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT: OpStore %temp_var_vector_1 [[cc4_0]]
// CHECK-NEXT: [[access11_0:%[0-9]+]] = OpAccessChain %_ptr_Function_v3int %temp_var_vector_1 %uint_0
// CHECK-NEXT: [[load8_0:%[0-9]+]] = OpLoad %v3int [[access11_0]]
// CHECK-NEXT: OpStore %intVec3 [[load8_0]]
    intVec3 = (intMat + intMat)[0];

    // Two level indexing (from rvalue)
// CHECK-NEXT: [[index3_0:%[0-9]+]] = OpLoad %uint %index
// CHECK:      [[cc5_0:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT: OpStore %temp_var_vector_2 [[cc5_0]]
// CHECK-NEXT: [[access12_0:%[0-9]+]] = OpAccessChain %_ptr_Function_int %temp_var_vector_2 %uint_0 [[index3_0]]
// CHECK-NEXT: [[load9_0:%[0-9]+]] = OpLoad %int [[access12_0]]
// CHECK-NEXT: OpStore %intScalar [[load9_0]]
    intScalar = (intMat + intMat)[0][index];
}
